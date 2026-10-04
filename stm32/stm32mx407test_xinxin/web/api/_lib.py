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

ALLOWED_ACTIONS = {"toggle", "query", "refresh", "reboot", "chat", "unknown"}

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
        '{"action":"toggle|query|refresh|reboot|chat|unknown","target":"上面的 key/id 或 null",'
        '"value":true|false|null,"reply":"15字内中文"}\n'
        "规则：打开/开启/启动→toggle value=true；关闭/关掉/停止→toggle value=false；"
        "问某个数值→query；刷新/更新数据→refresh；重启/复位设备→reboot；打招呼/闲聊/没听懂→chat 或 unknown。\n"
        '没指明是哪一路（例如只说"开灯"）→ {"action":"toggle","target":null,"value":null,"reply":"要开哪一路？"}'
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
    fallback = {"action": "unknown", "target": None, "value": None, "reply": "没听懂，再说一次"}
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

    if action == "toggle":
        if target is None or value not in (True, False):
            return {"action": "unknown", "target": None, "value": None,
                    "reply": reply or "要控制哪一路？开还是关？"}
    else:
        value = None

    if action in ("query",) and target is None:
        return {"action": "unknown", "target": None, "value": None,
                "reply": reply or "要查哪一项？"}

    return {"action": action, "target": target, "value": value, "reply": reply}


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
        kwargs["json"] = body or {}
    try:
        r = requests.request(method, "%s/%s" % (ONENET_BASE, path), **kwargs)
    except Exception as e:
        return {"code": 599, "msg": "onenet unreachable: %s" % e}, 502
    try:
        return r.json(), r.status_code
    except Exception:
        return {"code": 598, "msg": "bad json from onenet"}, 502


# ----------------------------- LLM -----------------------------
def ask_qwen(text, controls, cards):
    key = os.environ.get("DASHSCOPE_API_KEY", "")
    payload = {
        "model": os.environ.get("QWEN_MODEL", "qwen-plus"),
        "messages": [{"role": "system", "content": build_prompt(controls, cards)},
                     {"role": "user", "content": text}],
        "temperature": 0.1,
        "max_tokens": 200,
    }
    r = requests.post(DASHSCOPE_URL, json=payload, timeout=15,
                      headers={"Authorization": "Bearer " + key,
                               "Content-Type": "application/json"})
    r.raise_for_status()
    return r.json()["choices"][0]["message"]["content"]
