"""面板后端共用件。

设计约束：
- 本文件**不 import flask**：路由和守卫在 api/*.py 里，这里只放可单测的纯逻辑 + 出网调用。
- 所有密钥只从环境变量读（本地 `vercel dev` / 线上 Vercel 环境变量 / 本地 .env.local）。
- 兼容 Python 3.9（本地系统解释器），别用 3.10+ 语法。
"""
import json
import os
import re
import time

import requests

ONENET_BASE = "https://iot-api.heclouds.com"
DASHSCOPE_URL = "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions"

ALLOWED_ACTIONS = {"toggle", "toggle_many", "query", "refresh", "reboot", "chat", "unknown", "steps"}

# 一句话里做多件事（"太热了" = 开风扇 + 报温度）。只允许这些内层动作，且最多 3 步。
STEP_ACTIONS = {"toggle", "toggle_many", "query", "refresh"}
MAX_STEPS = 3

MAX_TARGETS = 8   # 一次最多动几个：防模型抽风列一大串

# op -> (HTTP 方法, OneNET 路径)：只放行这里列出的，前端不能自己拼路径
OP_MAP = {
    "deviceDetail": ("GET", "device/detail"),
    "getProperty": ("GET", "thingmodel/query-device-property"),
    "getHistory": ("GET", "thingmodel/query-device-property-history"),
    "setProperty": ("POST", "thingmodel/set-device-property"),
    "callService": ("POST", "thingmodel/call-service"),
}


# ----------------------------- 意图解析 -----------------------------
def build_prompt(controls, cards):
    ctrl = "\n".join("- %s：%s" % (c.get("key"), c.get("name", "")) for c in controls) or "（暂无）"
    card = "\n".join("- %s：%s（%s）" % (c.get("id"), c.get("name", ""), c.get("unit") or "")
                     for c in cards) or "（暂无）"
    return (
        "你是 IoT 设备控制助手，把用户的话转成 JSON。\n"
        "可用开关（target 只能取这些 key）：\n%s\n"
        "可查询属性（target 只能取这些 id）：\n%s\n"
        "只输出 JSON，不要 markdown，不要解释：\n"
        '{"action":"toggle|toggle_many|query|refresh|reboot|chat|unknown","target":"上面的 key/id 或 null",'
        '"targets":["一次要动多个时列在这里，否则 null"],"value":true|false|null,"reply":"15字内中文"}\n'
        "规则：打开/开启/启动→toggle value=true；关闭/关掉/停止→toggle value=false；"
        "问某个数值→query；刷新/更新数据→refresh；重启/复位设备→reboot；打招呼/闲聊/没听懂→chat 或 unknown。\n"
        '一次要动多个（"把灯都关了""全部关掉""蜂鸣器和风扇都关"）→ action=toggle_many，'
        'targets 填涉及的 key 列表、value 填 true/false、target 填 null。\n'
        '没指明是哪一路（例如只说"开灯"）→ {"action":"toggle","target":null,"value":null,"reply":"要开哪一路？"}\n'
        '只报了几个目标但没说开还是关（例如「灯1和灯3」「风扇和蜂鸣器」）→ action=toggle_many、'
        'targets 填这几个 key、value=null、reply 问「要开还是关」\n'
        '场景（用户描述感受/状态时按这里的动作做，用 steps 输出）：\n'
        '  "太热了/好热/热死了/有点热" → [开风扇, 查温度]（reply 写"已打开风扇"）\n'
        '  "太冷了/有点冷" → [关风扇]\n'
        '  "我要睡了/我出门了/全都关掉" → [把灯和风扇蜂鸣器全关]\n'
        '一句话里说了几件事（"打开风扇并告诉我温度"）→ 也用 steps，按顺序列出来。\n'
        'steps 写法：{"action":"steps","steps":[{"action":"toggle","target":"fan","value":true},'
        '{"action":"query","target":"temperature"}],"target":null,"targets":null,"value":null,"reply":"已打开风扇"}\n'
        "上下文：如果用户这句是在回答你上一轮的追问（上一轮你问了「要开哪一路？」、用户只说「灯 1」），"
        "就结合上文把动作补全成完整意图；用户说「算了」「不用了」→ action=chat。"
        % (ctrl, card)
    )


def parse_json_loose(content):
    """模型偶尔会套 ```json 围栏或加解释，剥掉再解；解析不了返回 None。"""
    if not content:
        return None
    s = re.sub(r"^```(?:json)?|```$", "", content.strip(), flags=re.M).strip()
    try:
        return json.loads(s)
    except Exception:
        m = re.search(r"\{.*\}", s, flags=re.S)
        if not m:
            return None
        try:
            return json.loads(m.group(0))
        except Exception:
            return None


def validate(intent, controls, cards):
    """LLM 输出不可信：动作、标识符、取值一律按白名单收口。"""
    fallback = {"action": "unknown", "target": None, "targets": None,
                "value": None, "reply": "没听懂，再说一次"}
    if not isinstance(intent, dict):
        return dict(fallback)

    action = intent.get("action")
    target = intent.get("target")
    value = intent.get("value")
    reply = str(intent.get("reply") or "")[:40]

    if action not in ALLOWED_ACTIONS:
        return dict(fallback, reply=reply or fallback["reply"])

    keys = {c.get("key") for c in controls} | {c.get("id") for c in cards}
    if target not in keys:          # 幻觉出的标识符（led4/relay）一律丢掉
        target = None

    if action == "steps":
        raw = intent.get("steps")
        if not isinstance(raw, list):
            return dict(fallback, reply=reply or fallback["reply"])
        steps = []
        for st in raw[:MAX_STEPS]:
            if not isinstance(st, dict) or st.get("action") not in STEP_ACTIONS:
                continue
            one = validate(dict(st, steps=None), controls, cards)      # 单步走同一套白名单
            if one["action"] not in STEP_ACTIONS:
                continue
            steps.append({k: one[k] for k in ("action", "target", "targets", "value")})
        if not steps:
            return dict(fallback, reply=reply or fallback["reply"])
        if len(steps) == 1:                      # 只剩一步就退回普通动作，前端少一条分支
            return dict(steps[0], reply=reply)
        return {"action": "steps", "steps": steps, "target": None, "targets": None,
                "value": None, "reply": reply}

    if action == "toggle_many":
        raw = intent.get("targets")
        targets = []
        if isinstance(raw, list):
            for t in raw:                      # 逐个过白名单 + 去重，幻觉出来的直接丢
                if t in keys and t not in targets:
                    targets.append(t)
        targets = targets[:MAX_TARGETS]
        if not targets or value not in (True, False):
            out = {"action": "unknown", "target": None, "targets": None, "value": None,
                   "reply": reply or "要动哪几个？开还是关？"}
            if targets or value in (True, False):
                out["pending"] = {"action": "toggle_many", "targets": targets or None,
                                  "value": value if value in (True, False) else None}
            return out
        if len(targets) == 1:                  # 只剩一个就退回单目标，前端少一条分支
            return {"action": "toggle", "target": targets[0], "targets": None,
                    "value": value, "reply": reply}
        return {"action": "toggle_many", "target": None, "targets": targets,
                "value": value, "reply": reply}

    if action == "toggle":
        if target is None or value not in (True, False):
            out = {"action": "unknown", "target": None, "targets": None, "value": None,
                   "reply": reply or "要控制哪一路？开还是关？"}
            if target is not None or value in (True, False):
                # 只要有"目标"或"开关"其中一样，就还差另一样 → 可继续追问
                # （"开灯"缺目标；"灯1和灯3"缺开关；两样都缺就只能让用户重说）
                out["pending"] = {"action": "toggle", "target": target,
                                  "value": value if value in (True, False) else None}
            return out
    else:
        value = None

    if action in ("query",) and target is None:
        return {"action": "unknown", "target": None, "value": None,
                "reply": reply or "要查哪一项？"}

    return {"action": action, "target": target, "targets": None,
            "value": value, "reply": reply}


# ----------------------------- 口令 / 限流 -----------------------------
PANEL_PASSWORD = lambda: os.environ.get("PANEL_PASSWORD", "")


def key_ok(request):
    """面板口令门。

    放行的情况：没配 PANEL_PASSWORD（本地开发），或显式设了 PANEL_DISABLE_KEY=1
    （本地预览调试用；**线上绝对不要设**，否则面板谁都能开）。
    """
    if os.environ.get("PANEL_DISABLE_KEY") == "1":
        return True
    want = PANEL_PASSWORD()
    if not want:
        return True
    return request.headers.get("X-Panel-Key") == want


def _redis(cmds):
    url = os.environ.get("UPSTASH_REDIS_REST_URL")
    tok = os.environ.get("UPSTASH_REDIS_REST_TOKEN")
    if not url or not tok:
        return None
    try:
        r = requests.post(url.rstrip("/") + "/pipeline", json=cmds, timeout=5,
                          headers={"Authorization": "Bearer " + tok})
        r.raise_for_status()
        return r.json()
    except Exception:
        return None


def rate_ok(request, per_min=None, daily=None):
    """每 IP 每分钟 + 全局每日两道闸。计数在 Upstash（serverless 无进程内存）。

    没配 Upstash 就放行（本地开发），线上务必配上，否则 key 可能被刷。
    """
    ip = (request.headers.get("x-forwarded-for") or
          getattr(request, "remote_addr", None) or "?").split(",")[0].strip()
    minute, day = time.strftime("%Y%m%d%H%M"), time.strftime("%Y%m%d")
    per_min = per_min or int(os.environ.get("RATE_PER_MIN", "20"))
    daily = daily or int(os.environ.get("DAILY_BUDGET", "1000"))

    res = _redis([["INCR", "r:%s:%s" % (ip, minute)], ["EXPIRE", "r:%s:%s" % (ip, minute), 90],
                  ["INCR", "d:%s" % day], ["EXPIRE", "d:%s" % day, 90000]])
    if res is None:
        return True
    try:
        per_ip, _e1, today, _e2 = [int(x["result"]) for x in res]
    except Exception:
        return True
    return per_ip <= per_min and today <= daily


# ----------------------------- OneNET 转发 -----------------------------
def _post_payload(op, body, product_id, device_name):
    """POST 两个 op 的报文形状不一样，统一在这里补齐（前端不该知道这些）：

    - setProperty: {"product_id","device_name","params":{标识符:值}}
      调用方直接传属性表也行，会被塞进 params（漏了平台回 10001 Params required）
    - callService: {"product_id","device_name","identifier","params":{…}}
    """
    payload = dict(body or {})
    if op == "setProperty":
        inner = payload.pop("params", None)
        if inner is None:
            inner, payload = payload, {}
        payload["params"] = inner
    else:
        payload.setdefault("params", {})
    payload.setdefault("product_id", product_id)
    payload.setdefault("device_name", device_name)
    return payload


def onenet_call(op, params=None, body=None):
    """只放行 OP_MAP 里的 op；token 由后端加，前端永远看不到。"""
    if op not in OP_MAP:
        return {"code": 403, "msg": "op not allowed"}, 403
    method, path = OP_MAP[op]
    q = {"product_id": os.environ.get("ONENET_PRODUCT_ID", ""),
         "device_name": os.environ.get("ONENET_DEVICE_NAME", "")}
    for k, v in (params or {}).items():
        if v is not None:
            q[k] = v
    kwargs = {"headers": {"authorization": os.environ.get("ONENET_TOKEN", "")}, "timeout": 20}
    if method == "GET":
        kwargs["params"] = q
    else:
        kwargs["json"] = _post_payload(op, body, q["product_id"], q["device_name"])
    try:
        r = requests.request(method, "%s/%s" % (ONENET_BASE, path), **kwargs)
    except Exception as e:
        return {"code": 599, "msg": "onenet unreachable: %s" % e}, 502
    try:
        return r.json(), r.status_code
    except Exception:
        return {"code": 598, "msg": "bad json from onenet"}, 502


# ----------------------------- 火山引擎 TTS（可选，云端音色） -----------------------------
# 走官方 v1 HTTP 接口：POST /api/v1/tts，Authorization: Bearer;<token>，
# body 里 app(appid/token/cluster) + audio(voice_type/encoding/speed_ratio...) + request(text/reqid)。
# 返回 {"code":3000,"data":"<base64 mp3>"}。字段如有出入，只改这里的环境变量/这一段。
VOLC_TTS_URL_DEFAULT = "https://openspeech.bytedance.com/api/v1/tts"

# 火山引擎**免费**的 21 款音色（用户从控制台抄的清单，按场景分组）。
# id 用官方 TTS 接口常见的 "_streaming" 写法；不同接口/账号可能要短名（BV700），
# 所以 tts_volc() 里带了自动重试（见下面 _toggle_streaming_suffix）。
# 要改/加音色，直接给 VOLC_TTS_VOICES 环境变量（JSON 数组，可带 group）。
VOLC_VOICES_DEFAULT = [
    # 通用场景（3）
    {"id": "BV700_streaming", "name": "灿灿", "group": "通用场景"},
    {"id": "BV001_streaming", "name": "通用女声", "group": "通用场景"},
    {"id": "BV002_streaming", "name": "通用男声", "group": "通用场景"},
    # 有声阅读（5）
    {"id": "BV701_streaming", "name": "擎苍", "group": "有声阅读"},
    {"id": "BV119_streaming", "name": "通用赘婿", "group": "有声阅读"},
    {"id": "BV102_streaming", "name": "儒雅青年", "group": "有声阅读"},
    {"id": "BV113_streaming", "name": "甜宠少御", "group": "有声阅读"},
    {"id": "BV115_streaming", "name": "古风少御", "group": "有声阅读"},
    # 智能助手 / 视频配音 / 特色 / 教育（6）
    {"id": "BV007_streaming", "name": "亲切女声", "group": "助手·配音·教育"},
    {"id": "BV056_streaming", "name": "阳光男声", "group": "助手·配音·教育"},
    {"id": "BV005_streaming", "name": "活泼女声", "group": "助手·配音·教育"},
    {"id": "BV051_streaming", "name": "奶气萌娃", "group": "助手·配音·教育"},
    {"id": "BV034_streaming", "name": "知性姐姐（双语）", "group": "助手·配音·教育"},
    {"id": "BV033_streaming", "name": "温柔小哥", "group": "助手·配音·教育"},
    # 方言（3）
    {"id": "BV021_streaming", "name": "东北老铁", "group": "方言"},
    {"id": "BV019_streaming", "name": "重庆小伙", "group": "方言"},
    {"id": "BV213_streaming", "name": "广西表哥", "group": "方言"},
    # 英语（2）
    {"id": "BV503_streaming", "name": "活力女声 Ariana", "group": "英语"},
    {"id": "BV504_streaming", "name": "活力男声 Jackson", "group": "英语"},
    # 日语（2）
    {"id": "BV522_streaming", "name": "气质女生", "group": "日语"},
    {"id": "BV524_streaming", "name": "日语男声", "group": "日语"},
    # 豆包语音合成 2.0（大模型音色，走 V3 WebSocket，计费项 seed-tts-2.0，需在控制台开通）
    {"id": "zh_female_vv_uranus_bigtts", "name": "Vivi 2.0（多语种/方言）", "group": "豆包 2.0（大模型）"},
]


def volc_voices():
    raw = os.environ.get("VOLC_TTS_VOICES")
    if raw:
        try:
            v = json.loads(raw)
            if isinstance(v, list) and v:
                return [{"id": str(x.get("id")), "name": str(x.get("name") or x.get("id")),
                         "group": str(x.get("group") or "")}
                        for x in v if isinstance(x, dict) and x.get("id")]
        except Exception:
            pass
    return VOLC_VOICES_DEFAULT


def volc_tts_ready(voice=None):
    """两条通道凭据不同：V1 要 appid+token，V3（豆包 2.0）要 API Key。
    只配了一种时，另一种音色不该被"误判为可用"。"""
    mode = os.environ.get("VOLC_TTS_API", "auto").lower()
    has_v1 = bool(os.environ.get("VOLC_TTS_APPID") and os.environ.get("VOLC_TTS_TOKEN"))
    has_v3 = _v3_headers() is not None
    if mode == "v1":
        return has_v1
    if mode == "v3":
        return has_v3
    if voice is not None:
        return has_v3 if _is_v3_voice(voice) else has_v1
    return has_v1 or has_v3


def tts_volc_v1_http(text, voice=None, speed=None, pitch=None, volume=None):
    """返回 (mp3 bytes, None) 或 (None, 错误说明)。错误原样带出去，方便在设置面板直接看到。"""
    import base64
    import uuid

    appid = os.environ.get("VOLC_TTS_APPID", "")
    token = os.environ.get("VOLC_TTS_TOKEN", "")
    if not appid or not token:
        return None, "后端没配 VOLC_TTS_APPID / VOLC_TTS_TOKEN"
    payload = {
        "app": {"appid": appid, "token": token,
                "cluster": os.environ.get("VOLC_TTS_CLUSTER", "volcano_tts")},
        "user": {"uid": os.environ.get("VOLC_TTS_UID", "onenet-panel")},
        "audio": {
            "voice_type": voice or os.environ.get("VOLC_TTS_VOICE", "BV001_streaming"),
            "encoding": os.environ.get("VOLC_TTS_ENCODING", "mp3"),
            "speed_ratio": float(speed or 1.0),
            "volume_ratio": float(volume or 1.0),
            "pitch_ratio": float(pitch or 1.0),
        },
        "request": {"reqid": uuid.uuid4().hex, "text": text,
                    "text_type": "plain", "operation": "query",
                    # 官方示例里的这两个字段：交给它的前端做文本规范化（数字/单位/符号读法），
                    # 对我们有用（要念"26.7 度""%RH"这种）。缺了也能跑，加上更稳。
                    "with_frontend": 1, "frontend_type": "unitTson"},
    }
    def _call(p):
        return requests.post(os.environ.get("VOLC_TTS_URL", VOLC_TTS_URL_DEFAULT),
                             json=p, timeout=20,
                             headers={"Authorization": "Bearer;" + token})

    try:
        r = _call(payload)
        j = r.json()
        # 音色不存在时换一种 id 写法重试一次：有的接口要 BV700，有的要 BV700_streaming
        msg = str(j.get("message", ""))
        if j.get("code") != 3000 and ("voice" in msg.lower() or "音色" in msg):
            vid = payload["audio"]["voice_type"]
            alt = vid[:-len("_streaming")] if vid.endswith("_streaming") else vid + "_streaming"
            payload["audio"]["voice_type"] = alt
            r = _call(payload)
            j = r.json()
    except Exception as e:
        return None, "连不上火山 TTS：%s" % e
    try:
        j = r.json()
    except Exception:
        return None, "火山返回的不是 JSON（HTTP %s）：%s" % (r.status_code, r.text[:200])
    if j.get("code") != 3000:
        return None, "火山报错 code=%s message=%s" % (j.get("code"), j.get("message"))
    try:
        return base64.b64decode(j["data"]), None
    except Exception as e:
        return None, "音频解码失败：%s" % e


# ---------- 火山 V3（豆包语音合成 2.0 / Vivi 2.0 这类 bigtts 音色必须走这条） ----------
# 协议：单向流式 WebSocket，二进制帧 = 4 字节 header (+4 字节 event) + 4 字节 payload 长度 + payload（大端）。
# 关键点：**浏览器原生 WebSocket 不能带自定义请求头**，带 key 连接只能在服务端做，
# 所以这里由后端当 WS 客户端把音频攒齐，再按普通 HTTP 返回 mp3（前端无感）。
VOLC_TTS_V3_URL_DEFAULT = "wss://openspeech.bytedance.com/api/v3/tts/unidirectional/stream"
VOLC_V3_RESOURCE_DEFAULT = "seed-tts-2.0"          # 豆包语音合成 2.0 的计费项


def _v3_headers():
    """新版控制台用 X-Api-Key；旧版用 X-Api-App-Id + X-Api-Access-Key。"""
    key = os.environ.get("VOLC_TTS_API_KEY")
    if key:
        return {"X-Api-Key": key}
    appid, token = os.environ.get("VOLC_TTS_APPID"), os.environ.get("VOLC_TTS_TOKEN")
    if appid and token:
        return {"X-Api-App-Id": appid, "X-Api-Access-Key": token}
    return None


def _u32(n):
    import struct
    return struct.pack(">I", n)


def v3_send_text_frame(payload_obj):
    """SendText：header 0x11 0x10 0x10 0x00 + 长度(4) + JSON"""
    import json as _json
    body = _json.dumps(payload_obj, ensure_ascii=False).encode("utf-8")
    return b"\x11\x10\x10\x00" + _u32(len(body)) + body


def v3_finish_frame():
    """FinishConnection：header 0x11 0x14 0x10 0x00 + event=2 + 长度(4) + {}"""
    body = b"{}"
    return b"\x11\x14\x10\x00" + _u32(2) + _u32(len(body)) + body


def v3_parse_frame(buf):
    """解析服务端帧：音频 / JSON / 错误。event: 352=TTSResponse 152=SessionFinished 52=ConnectionFinished"""
    import struct
    if len(buf) < 4:
        return {"type": "unknown"}
    b1 = buf[1]
    msg_type = (b1 >> 4) & 0x0F
    has_event = (b1 & 0x0F & 0x04) != 0
    off = 4
    event = None
    if has_event:
        if len(buf) < off + 4:
            return {"type": "unknown"}
        event = struct.unpack(">I", buf[off:off + 4])[0]
        off += 4
    if msg_type == 0b1111:                          # 错误帧
        return {"type": "error", "code": struct.unpack(">I", buf[4:8])[0] if len(buf) >= 8 else -1}
    if msg_type == 0b1011:                          # 音频帧：sid_len(4)+sid+audio_len(4)+audio
        sid_len = struct.unpack(">I", buf[off:off + 4])[0]
        off += 4 + sid_len
        audio_len = struct.unpack(">I", buf[off:off + 4])[0]
        off += 4
        return {"type": "audio", "event": event, "data": buf[off:off + audio_len]}
    if msg_type == 0b1001:                          # JSON 帧：len(4)+json
        payload_len = struct.unpack(">I", buf[off:off + 4])[0]
        off += 4
        return {"type": "meta", "event": event, "json": buf[off:off + payload_len].decode("utf-8", "replace")}
    return {"type": "unknown"}


def _is_v3_voice(voice):
    """豆包 2.0/大模型音色命名：含 bigtts 或 zh_/en_/ja_/multi_ 前缀；其余（BVxxx）走 V1 HTTP。"""
    v = (voice or "").lower()
    return ("bigtts" in v) or v.startswith(("zh_", "en_", "ja_", "multi_", "cn_"))


def tts_volc_v3_ws(text, voice=None, speed=None, pitch=None, volume=None, timeout=25):
    """返回 (mp3 bytes, None) 或 (None, 错误说明)。pitch 在 V3 没有对应参数，忽略。"""
    import uuid
    headers = _v3_headers()
    if not headers:
        return None, "后端没配 VOLC_TTS_API_KEY（新版控制台）或 VOLC_TTS_APPID+VOLC_TTS_TOKEN（旧版）"
    try:
        import websocket                                  # websocket-client
    except Exception:
        return None, "后端缺依赖：websocket-client（requirements.txt 里加）"

    voice = voice or os.environ.get("VOLC_TTS_VOICE", "zh_female_vv_uranus_bigtts")
    h = dict(headers)
    h["X-Api-Resource-Id"] = os.environ.get("VOLC_TTS_V3_RESOURCE", VOLC_V3_RESOURCE_DEFAULT)
    h["X-Api-Connect-Id"] = uuid.uuid4().hex
    params = {"text": text, "speaker": voice,
              "audio_params": {"format": "mp3", "sample_rate": 24000}}
    if speed is not None:
        params["audio_params"]["speech_rate"] = int(round((float(speed) - 1) * 100))   # -50~100
    if volume is not None:
        params["audio_params"]["loudness_rate"] = int(round((float(volume) - 1) * 100))
    payload = {"user": {"uid": os.environ.get("VOLC_TTS_UID", "onenet-panel")}, "req_params": params}

    try:
        ws = websocket.create_connection(os.environ.get("VOLC_TTS_V3_URL", VOLC_TTS_V3_URL_DEFAULT),
                                        header=h, timeout=timeout)
    except Exception as e:
        return None, "连不上火山 V3：%s" % e
    try:
        ws.send_binary(v3_send_text_frame(payload))
        chunks = []
        while True:
            raw = ws.recv()
            if not raw:
                break
            f = v3_parse_frame(raw)
            if f["type"] == "audio":
                chunks.append(f["data"])
            elif f["type"] == "meta":
                if f.get("event") == 152:                 # SessionFinished → 收尾
                    ws.send_binary(v3_finish_frame())
                elif f.get("event") == 52:                # ConnectionFinished → 结束
                    break
            elif f["type"] == "error":
                return None, "火山 V3 报错 code=%s" % f.get("code")
        if not chunks:
            return None, "火山 V3 没返回音频（音色 id、资源 id 或权限可能不对）"
        return b"".join(chunks), None
    except Exception as e:
        return None, "火山 V3 会话异常：%s" % e
    finally:
        try:
            ws.close()
        except Exception:
            pass


def tts_volc(text, voice=None, speed=None, pitch=None, volume=None):
    """按音色自动选通道：bigtts/大模型音色 → V3 WS；BVxxx 普通音色 → V1 HTTP。
    也可用 VOLC_TTS_API=v1|v3 强制指定。"""
    voice = voice or os.environ.get("VOLC_TTS_VOICE", "BV001_streaming")
    mode = os.environ.get("VOLC_TTS_API", "auto").lower()
    use_v3 = (mode == "v3") or (mode == "auto" and _is_v3_voice(voice))
    if use_v3:
        audio, err = tts_volc_v3_ws(text, voice, speed, pitch, volume)
        if err is None:
            return audio, None
        if _is_v3_voice(voice):
            return None, err                       # 2.0 音色在 V1 必然失败，别掩盖真原因
        return tts_volc_v1_http(text, voice, speed, pitch, volume)
    return tts_volc_v1_http(text, voice, speed, pitch, volume)


# ----------------------------- LLM -----------------------------
def build_messages(text, controls, cards, history=None, pending=None):
    """系统提示 + 最近几轮对话 + 本轮用户话。

    pending 是上一轮"还差什么信息"的结构化提示（例如只说了"开灯"、差哪一路），
    显式喂回去比只靠 history 可靠得多：实测只给 history 时，用户答"关掉"
    会被理解成"把全部都关了"，而不是补全上一轮那两路。
    """
    system = build_prompt(controls, cards)
    if pending:
        system += ("\n\n注意：用户这句是在回答你的上一次追问。上一轮没定下来的部分是："
                   + json.dumps(pending, ensure_ascii=False)
                   + "。请只把缺的部分补上，不要扩大范围、也不要改动已经确定的目标，"
                     "然后输出完整意图 JSON。")
    msgs = [{"role": "system", "content": system}]
    for h in (history or [])[-6:]:
        role, content = h.get("role"), h.get("content")
        if role in ("user", "assistant") and isinstance(content, str) and content.strip():
            msgs.append({"role": role, "content": content[:400]})
    msgs.append({"role": "user", "content": text})
    return msgs


def ask_qwen(text, controls, cards, history=None, pending=None):
    key = os.environ.get("DASHSCOPE_API_KEY", "")
    payload = {
        "model": os.environ.get("QWEN_MODEL", "qwen-plus"),
        "messages": build_messages(text, controls, cards, history, pending),
        "temperature": 0.1,
        "max_tokens": 300,
    }
    # 思考型模型（qwen3.x 这类）会先吐一大段 reasoning_content：实测 8.7s / 1.3 万 token，
    # 而我们只要一句 JSON。QWEN_THINKING=0 关掉它；非思考模型会忽略这个字段（qwen-flash 实测无副作用）。
    thinking = os.environ.get("QWEN_THINKING")
    if thinking in ("0", "1"):
        payload["enable_thinking"] = (thinking == "1")
    r = requests.post(DASHSCOPE_URL, json=payload, timeout=15,
                      headers={"Authorization": "Bearer " + key,
                               "Content-Type": "application/json"})
    r.raise_for_status()
    return r.json()["choices"][0]["message"]["content"]
