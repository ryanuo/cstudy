"""火山引擎 TTS（可选）：把要念的话在后端合成成 mp3 返回，key 不出后端。"""
from flask import Flask, jsonify, request

from _lib import key_ok, rate_ok, tts_volc, volc_tts_ready, volc_voices

app = Flask(__name__)


@app.get("/api/tts")
@app.get("/tts")
def tts_info():
    """前端打开设置面板时问一次：能用吗、有哪些音色。"""
    return jsonify({"configured": volc_tts_ready(), "voices": volc_voices()})


@app.post("/api/tts")
@app.post("/tts")
def tts():
    if not key_ok(request):
        return jsonify({"code": 401, "msg": "口令不对"}), 401
    if not rate_ok(request):
        return jsonify({"code": 429, "msg": "慢点说"}), 429

    d = request.get_json(force=True) or {}
    text = (d.get("text") or "").strip()[:300]
    if not text:
        return jsonify({"code": 400, "msg": "text 为空"}), 400
    if not volc_tts_ready():
        return jsonify({"code": 500, "msg": "后端没配 VOLC_TTS_APPID / VOLC_TTS_TOKEN"}), 500

    audio, err = tts_volc(text, d.get("voice"), d.get("speed"), d.get("pitch"), d.get("volume"))
    if err:
        app.logger.warning("火山 TTS 失败: %s", err)
        return jsonify({"code": 502, "msg": err}), 502
    return audio, 200, {"Content-Type": "audio/mpeg", "Cache-Control": "no-store"}
