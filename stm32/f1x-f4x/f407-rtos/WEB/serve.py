#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""起一个本地前端服务，把本目录（WEB/）里的页面发给浏览器。

用法：
    python3 serve.py                             # 端口 8000，服务本目录
    python3 serve.py -p 5173                     # 换端口（被占用会自动往后找）
    python3 serve.py --board 192.168.1.101       # 顺便测一下板子通不通
    python3 serve.py --board 192.168.1.101 -o    # 顺便自动打开浏览器（带 ?ip= 预填）

注意：
  * 页面要跑在 http:// 上；别用 https 页面去请求 http 板子（混合内容会被浏览器拦掉）
  * 服务绑在 0.0.0.0，所以手机也能用"局域网"那行的地址打开（手机需和板子同一 WiFi）
  * 响应带 no-store，改完 index.html 刷新就能看到，不用清缓存
"""

import argparse
import functools
import http.server
import os
import socket
import socketserver
import sys
import urllib.request
import webbrowser

class NoCacheHandler(http.server.SimpleHTTPRequestHandler):
    """静态文件服务 + 禁用缓存"""

    def end_headers(self):
        self.send_header("Cache-Control", "no-store, no-cache, must-revalidate")
        self.send_header("Pragma", "no-cache")
        self.send_header("Expires", "0")
        super().end_headers()

    def log_message(self, fmt, *args):
        sys.stderr.write("  %s  %s\n" % (self.address_string(), fmt % args))


def lan_ips():
    """尽量列出本机的局域网 IPv4（给手机用）"""
    ips = set()
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))          # 不会真的发包，只为让内核选出出口网卡
        ips.add(s.getsockname()[0])
        s.close()
    except OSError:
        pass
    try:
        for info in socket.getaddrinfo(socket.gethostname(), None, socket.AF_INET):
            ips.add(info[4][0])
    except OSError:
        pass
    return sorted(i for i in ips if not i.startswith("127."))


def pick_port(preferred):
    """端口被占用就往后找几个；都不行就交给系统分配"""
    for p in range(preferred, preferred + 20):
        with socket.socket() as s:
            try:
                s.bind(("", p))
                return p
            except OSError:
                continue
    return 0


def check_board(ip, token=""):
    """用普通 HTTP 测一下板子（这里没有浏览器同源策略，通不通只看网络）"""
    url = "http://%s/data%s" % (ip, ("?k=" + token) if token else "")
    print("  测试板子 %s ..." % url)
    try:
        with urllib.request.urlopen(url, timeout=3) as r:
            body = r.read().decode("utf-8", "replace").strip()
            if '"err"' in body:
                print("  [OK] 板子有响应，但令牌不对: %s" % body)
                print("       板子通了，只是 --token 没带或不对（web.h 里的 WEB_TOKEN）")
                return True
            print("  [OK] 板子有响应: %s" % body)
            return True
    except Exception as e:                                   # noqa: BLE001
        print("  [FAIL] 连不上: %s" % e)
        print("         依次检查：")
        print("         1) 电脑和板子在同一台路由器、同一网段"
              "（对比 ipconfig / ifconfig 里 IP 的前三段和 OLED 上的 IP）")
        print("         2) 路由器/热点没开 AP 隔离（访客网络常见）")
        print("         3) OLED 上 Server:80 是 OK，不是 FAIL")
        return False


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    ap = argparse.ArgumentParser(description="起一个本地前端服务（默认服务本目录）")
    ap.add_argument("-d", "--dir", default=here, help="要服务的目录，默认就是本文件所在目录")
    ap.add_argument("-p", "--port", type=int, default=8000, help="端口，默认 8000")
    ap.add_argument("--board", help="顺便测一下板子，例如 192.168.1.101")
    ap.add_argument("--token", help="板子令牌（web.h 里的 WEB_TOKEN），会带进页面 ?k= 并用于测板子")
    ap.add_argument("-o", "--open", action="store_true", help="起完自动打开浏览器")
    a = ap.parse_args()

    if not os.path.isdir(a.dir):
        print("目录不存在: %s" % a.dir)
        return 1

    port = pick_port(a.port)
    handler = functools.partial(NoCacheHandler, directory=a.dir)
    httpd = socketserver.ThreadingTCPServer(("", port), handler)
    httpd.daemon_threads = True

    print("前端服务已启动")
    print("  目录:     %s" % a.dir)
    print("  本机:     http://localhost:%d/" % port)
    for ip in lan_ips():
        print("  局域网:   http://%s:%d/" % (ip, port))
    print("  Ctrl+C 停止\n")
    sys.stdout.flush()          # 重定向到文件时也能立刻看到（默认会缓冲）

    if a.board:
        check_board(a.board, a.token or "")

    q = []
    if a.board:
        q.append("ip=" + a.board)
    if a.token:
        q.append("k=" + a.token)
    url = "http://localhost:%d/" % port
    if q:
        url += "?" + "&".join(q)
    if a.open:
        webbrowser.open(url)

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n已停止")
    finally:
        httpd.server_close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
