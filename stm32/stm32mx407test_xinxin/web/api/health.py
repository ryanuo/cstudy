import os
import sys

# Vercel 把 api/*.py 各当一个独立函数打包，运行时 sys.path 里**没有** api/ 这一层，
# 直接 `from _lib import …` 会 ModuleNotFoundError（本地 dev_server 手动加了才没暴露）。
# 一行把函数自己的目录加进去，本地/线上都成立。
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from flask import Flask, jsonify

from _lib import VOLC_TTS_VOICE, VOLC_TTS_VOICES, _v3_headers

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
        "tts": {"voice": VOLC_TTS_VOICE, "voices": VOLC_TTS_VOICES,
                "ready": _v3_headers() is not None},
    })
