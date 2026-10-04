"""面板后端共用件。

设计约束：
- 本文件**不 import flask**：路由和守卫在 api/*.py 里，这里只放可单测的纯逻辑 + 出网调用。
- 所有密钥只从环境变量读（本地 `vercel dev` / 线上 Vercel 环境变量 / 本地 .env.local）。
- 兼容 Python 3.9（本地系统解释器），别用 3.10+ 语法。
"""
import base64
import json
import os
import re
import time
import uuid

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


# ----------------------------- 火山 TTS（豆包语音合成大模型 2.0，HTTP Chunked 单向流式）-------------
# 这条是**实时**用的：文本一次给，音频分块流回来，响应体是一串 JSON 对象
# （每块形如 {"code":0,"message":"OK","data":"<base64 音频>",…}），纯 HTTP + JSON，不用二进制帧。
# 同族还有两条，别选错：
#   · /api/v3/tts/submit + /query（异步长文本）：文档自述"专为长文本、非实时场景"，走集群排队，
#     同类接口通常数十分钟才返回 —— 给面板念一句话不能用它。
#   · wss://…/api/v3/tts/unidirectional/stream（WebSocket）：能力相同，但要手搓二进制帧，没必要。
# 计费：大模型 2.0 按**合成字符数**计费（资源 seed-tts-2.0），需在控制台开通；
# 鉴权：新版控制台用 X-Api-Key；旧版控制台用 X-Api-App-Id + X-Api-Access-Key。
VOLC_TTS_URL = "https://openspeech.bytedance.com/api/v3/tts/unidirectional"
VOLC_TTS_RESOURCE = os.environ.get("VOLC_TTS_RESOURCE", "seed-tts-2.0")

# 音色写死在代码里（配置里不用管）。换音色＝改这一行；可选音色在控制台 > 音色库（*_bigtts 那批）。
# 这里用的是文档示例同款：豆包语音合成模型 2.0 的 Vivi 2.0（中/日/印尼/西语 + 30 多语种 + 方言）。
VOLC_TTS_VOICE = "zh_female_vv_uranus_bigtts"


def _v3_headers():
    """X-Api-Resource-Id 必选；X-Api-Request-Id 也必选（uuid）。凭据优先用新版 API Key。"""
    h = {"X-Api-Resource-Id": VOLC_TTS_RESOURCE, "X-Api-Request-Id": uuid.uuid4().hex,
         "Content-Type": "application/json"}
    key = os.environ.get("VOLC_TTS_API_KEY", "")
    if key:
        h["X-Api-Key"] = key
        return h
    appid = os.environ.get("VOLC_TTS_APPID", "")
    akey = os.environ.get("VOLC_TTS_ACCESS_KEY") or os.environ.get("VOLC_TTS_TOKEN") or ""
    if appid and akey:                       # 旧版控制台鉴权（Access Key 老名字叫 access_token）
        h["X-Api-App-Id"] = appid
        h["X-Api-Access-Key"] = akey
        return h
    return None


def _offset(v, scale=100, lo=-50, hi=100):
    """面板滑条倍率（1.0 = 正常）→ 接口整数：语速/音量 0 正常、100 是 2 倍；音调 0 正常、±12 顶格。"""
    try:
        n = int(round((float(v) - 1.0) * scale))
    except (TypeError, ValueError):
        return 0
    return min(max(n, lo), hi)


def _stream_objs(chunks):
    """Chunked 流里是一串 JSON 对象：可能被拆包/粘连，也可能带 `data:` 前缀。

    所以累积成 buffer 后逐个 raw_decode —— 别假设"一块就是一条完整 JSON"。
    """
    dec = json.JSONDecoder()
    buf = ""
    for raw in chunks:
        if not raw:
            continue
        buf += raw.decode("utf-8", "ignore")
        while True:
            k = buf.find("{")
            if k < 0:
                buf = ""
                break
            buf = buf[k:]
            try:
                obj, end = dec.raw_decode(buf)
            except ValueError:
                break                        # 还没收全，等下一块
            buf = buf[end:]
            yield obj


def tts(text, speed=None, pitch=None, volume=None):
    """一句话 → mp3 字节。失败返回 (None, 原因)。"""
    headers = _v3_headers()
    if not headers:
        return None, "后端没配 VOLC_TTS_API_KEY（新版控制台）或 VOLC_TTS_APPID+VOLC_TTS_ACCESS_KEY（旧版）"
    payload = {"req_params": {
        "text": text,
        "speaker": VOLC_TTS_VOICE,
        "audio_params": {"format": "mp3", "sample_rate": 24000,
                         "speech_rate": _offset(speed),             # 语速：0=正常，100=2 倍
                         "loudness_rate": _offset(volume)},         # 音量：同上
        "post_process": {"pitch": _offset(pitch, 12, -12, 12)},     # 音调：-12~12
    }}
    try:
        r = requests.post(VOLC_TTS_URL, json=payload, headers=headers, stream=True, timeout=(10, 90))
        if r.status_code != 200:
            # 错误体也是 JSON（形如 {"header":{"code":45000010,"message":"load grant: …"}}）：
            # 只把 code/message 提出来，别把整坨 JSON 糊给用户（实测 401 就是"凭据没映射到已开通的服务"）
            body = (r.text or "").strip()
            try:
                h = (json.loads(body) or {}).get("header") or {}
                if h.get("code") is not None:
                    return None, "火山 HTTP %s：%s %s（音色 %s）" % (
                        r.status_code, h.get("code"), h.get("message") or "", VOLC_TTS_VOICE)
            except Exception:
                pass
            return None, "火山 HTTP %s：%s" % (r.status_code, body[:200])
        audio = bytearray()
        for obj in _stream_objs(r.iter_content(chunk_size=8192)):
            code = obj.get("code")
            if code not in (0, None):            # 流里带错就立刻停：别把半截音频当成功
                return None, "火山返回 %s：%s（音色 %s）" % (code, obj.get("message") or "无说明", VOLC_TTS_VOICE)
            d = obj.get("data")
            if d:
                try:
                    audio += base64.b64decode(d)
                except Exception:
                    return None, "火山返回的音频不是合法 base64"
    except Exception as e:
        return None, "连不上火山 TTS：%s" % e
    return (bytes(audio), None) if audio else (None, "火山没返回音频（音色 %s）" % VOLC_TTS_VOICE)


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
