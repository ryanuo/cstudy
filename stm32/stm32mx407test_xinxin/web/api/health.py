import os

from flask import Flask, jsonify

app = Flask(__name__)


@app.route("/api/health")
@app.route("/health")
def health():
    return jsonify({
        "ok": True,
        "model": os.environ.get("QWEN_MODEL", "qwen-plus"),
        "redis": bool(os.environ.get("UPSTASH_REDIS_REST_URL")),
        "password": bool(os.environ.get("PANEL_PASSWORD")),
    })
