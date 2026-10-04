"""POST /api/tts：面板朗读用的语音合成（火山免费音色）。

成功直接回 audio/mpeg（前端 <audio> 一放就响）；失败回 JSON，前端自动退回浏览器朗读。
音色在 _lib.VOLC_TTS_VOICES 清单里维护、面板可切（白名单校验），默认第一条。
"""
import os
import sys

# Vercel 把 api/*.py 各当一个独立函数打包，运行时 sys.path 里**没有** api/ 这一层，
# 直接 `from _lib import …` 会 ModuleNotFoundError（本地 dev_server 手动加了才没暴露）。
# 一行把函数自己的目录加进去，本地/线上都成立。
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

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

    audio, err = tts(text, body.get("rate"), body.get("pitch"), body.get("volume"), body.get("voice"))
    if err:
        app.logger.warning("TTS 失败：%s", err)
        code = 500 if "没配" in err else 502      # 没配凭据是我们这边的问题，别混进上游错误
        return jsonify({"code": code, "msg": err}), code
    return app.response_class(audio, mimetype="audio/mpeg")
