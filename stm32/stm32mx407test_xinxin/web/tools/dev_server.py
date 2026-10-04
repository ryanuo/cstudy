"""本地开发服务器：把 api/*.py 三个 Flask 应用 + web/ 静态文件挂在同一个端口上。

用法：  .venv/bin/python tools/dev_server.py         # http://127.0.0.1:3000
（线上是 Vercel；`vercel dev` 也能用，但需要登录。这个脚本纯本地、无需登录。）

环境变量从 .env.local 读（Vercel 上用 vercel env，不需要这个文件）。
"""
import mimetypes
import os
import pathlib
import sys
from wsgiref.simple_server import make_server

ROOT = pathlib.Path(__file__).resolve().parents[1]   # = web/
WEB = ROOT                          # 静态文件就在 web/ 下


def load_env_local(path):
    if not path.exists():
        return
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        k, v = line.split("=", 1)
        v = v.strip().strip('"').strip("'")
        if v and not os.environ.get(k.strip()):      # 真环境变量优先
            os.environ[k.strip()] = v


load_env_local(ROOT / ".env.local")
sys.path.insert(0, str(ROOT / "api"))   # api/_lib.py 等

from chat import app as chat_app          # noqa: E402
from health import app as health_app      # noqa: E402
from onenet import app as onenet_app      # noqa: E402


def static_app(environ, start_response):
    rel = environ.get("PATH_INFO", "/").lstrip("/")
    target = (WEB / rel) if rel else (WEB / "index.html")
    if target.is_dir():
        target = target / "index.html"
    if not target.exists() or WEB not in target.resolve().parents:
        start_response("404 Not Found", [("Content-Type", "text/plain; charset=utf-8")])
        return [b"404"]
    body = target.read_bytes()
    ctype = mimetypes.guess_type(str(target))[0] or "application/octet-stream"
    start_response("200 OK", [("Content-Type", ctype), ("Cache-Control", "no-store")])
    return [body]


def app(environ, start_response):
    path = environ.get("PATH_INFO", "/")
    if path.startswith("/api/health") or path == "/health":
        return health_app(environ, start_response)
    if path.startswith("/api/chat") or path == "/chat":
        return chat_app(environ, start_response)
    if path.startswith("/api/onenet") or path == "/onenet":
        return onenet_app(environ, start_response)
    return static_app(environ, start_response)


if __name__ == "__main__":
    port = int(os.environ.get("PORT", "3000"))
    print("panel+api  http://127.0.0.1:%d   (口令 %s)"
          % (port, "已启用" if os.environ.get("PANEL_PASSWORD") else "未设置=放行"))
    make_server("127.0.0.1", port, app).serve_forever()
