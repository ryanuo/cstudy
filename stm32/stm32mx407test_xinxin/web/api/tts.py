"""POST /api/tts：面板朗读用的语音合成（火山免费音色）。

成功直接回 audio/mpeg（前端 <audio> 一放就响）；失败回 JSON，前端自动退回浏览器朗读。
音色在后端环境变量 VOLC_TTS_VOICE 里固定，前端不参与，面板里没有音色清单。
"""
import os

from flask import Flask, jsonify, request

from _lib import key_ok, rate_ok, tts

app = Flask(__name__)

MAX_CHARS = int(os.environ.get("TTS_MAX_CHARS", "300"))     # 播报就一句话，长了没必要合成


@app.post("/api/tts")
@app.post("/tts")          # 兜底：不同版本的 Vercel 运行时可能削掉 /api 前缀
def speak_route():
    if not key_ok(request):
        return jsonify({"code": 401, "msg": "口令不对"}), 401
    if not rate_ok(request):
        return jsonify({"code": 429, "msg": "慢点说"}), 429

    body = request.get_json(silent=True) or {}
    text = (body.get("text") or "").strip()[:MAX_CHARS]
    if not text:
        return jsonify({"code": 400, "msg": "文本为空"}), 400

    audio, err = tts(text, body.get("rate"), body.get("pitch"), body.get("volume"))
    if err:
        app.logger.warning("TTS 失败：%s", err)
        code = 500 if "没配" in err else 502      # 没配凭据是我们这边的问题，别混进上游错误
        return jsonify({"code": code, "msg": err}), code
    return app.response_class(audio, mimetype="audio/mpeg")
