import os

from flask import Flask, jsonify

from _lib import VOLC_TTS_VOICE, _v3_headers

app = Flask(__name__)


@app.route("/api/health")
@app.route("/health")
def health():
    return jsonify({
        "ok": True,
        "model": os.environ.get("QWEN_MODEL", "qwen-plus"),
        "redis": bool(os.environ.get("UPSTASH_REDIS_REST_URL")),
        "password": bool(os.environ.get("PANEL_PASSWORD")),
        # 面板要显示"实际用哪条朗读通道"：音色名来自代码常量，ready=凭据是否配齐
        "tts": {"voice": VOLC_TTS_VOICE, "ready": _v3_headers() is not None},
    })
