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
