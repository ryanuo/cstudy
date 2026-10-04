import os
import sys

# Vercel 把 api/*.py 各当一个独立函数打包，运行时 sys.path 里**没有** api/ 这一层，
# 直接 `from _lib import …` 会 ModuleNotFoundError（本地 dev_server 手动加了才没暴露）。
# 一行把函数自己的目录加进去，本地/线上都成立。
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from flask import Flask, jsonify, request

from _lib import key_ok, onenet_call, rate_ok

app = Flask(__name__)


@app.post("/api/onenet")
@app.post("/onenet")
def onenet():
    if not key_ok(request):
        return jsonify({"code": 401, "msg": "口令不对"}), 401
    if not rate_ok(request):
        return jsonify({"code": 429, "msg": "请求太快"}), 429

    d = request.get_json(force=True) or {}
    data, code = onenet_call(d.get("op"), d.get("params"), d.get("body"))
    return jsonify(data), code
