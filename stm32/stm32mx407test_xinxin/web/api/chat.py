import os

from flask import Flask, jsonify, request

from _lib import ask_qwen, key_ok, parse_json_loose, rate_ok, validate

app = Flask(__name__)


@app.post("/api/chat")
@app.post("/chat")          # 兜底：不同版本的 Vercel 运行时可能削掉 /api 前缀
def chat():
    if not key_ok(request):
        return jsonify({"code": 401, "msg": "口令不对"}), 401
    if not rate_ok(request):
        return jsonify({"code": 429, "msg": "慢点说"}), 429

    body = request.get_json(force=True) or {}
    text = (body.get("text") or "").strip()
    controls = body.get("controls") or []
    cards = body.get("cards") or []
    history = body.get("history") or []          # 多轮追问：最近几轮 user/assistant
    pending = body.get("pending") or None        # 上一轮还差什么（缺哪一路 / 缺开还是关）
    if not text:
        return jsonify({"intent": {"action": "unknown", "reply": "没听到内容"}})
    if not os.environ.get("DASHSCOPE_API_KEY"):
        return jsonify({"code": 500, "msg": "后端没配 DASHSCOPE_API_KEY"}), 500

    try:
        raw = ask_qwen(text, controls, cards, history, pending)
    except Exception as e:
        app.logger.warning("LLM 失败: %s", e)
        return jsonify({"intent": validate({"action": "unknown",
                                            "reply": "理解失败，再说一次"}, controls, cards)})

    return jsonify({"intent": validate(parse_json_loose(raw), controls, cards)})
