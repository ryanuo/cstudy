"""火山引擎 TTS（可选）：把要念的话在后端合成成 mp3 返回，key 不出后端。"""
from flask import Flask, jsonify, request

from _lib import key_ok, rate_ok, tts_volc, volc_tts_ready, volc_voices

app = Flask(__name__)


@app.get("/api/tts")
@app.get("/tts")
def tts_info():
    """前端打开设置面板时问一次：能用吗、有哪些音色。"""
    v = volc_voices()
    return jsonify({"configured": volc_tts_ready(v[0]["id"] if v else None), "voices": v})


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
    # 不再在这里预判"配没配"：两条通道凭据不同（V1 要 appid+token、V3 要 API Key），
    # 让 tts_volc 按音色选通道并给出精确原因，避免把 V3 音色误杀成"缺 appid/token"
    audio, err = tts_volc(text, d.get("voice"), d.get("speed"), d.get("pitch"), d.get("volume"))
    if err:
        app.logger.warning("火山 TTS 失败: %s", err)
        code = 500 if "没配" in err else 502
        return jsonify({"code": code, "msg": err}), code
    return audio, 200, {"Content-Type": "audio/mpeg", "Cache-Control": "no-store"}
