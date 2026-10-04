"""火山 TTS（免费音色 / V1 HTTP 一次性）的请求形状、错误透传、端点行为。

不连真实服务：请求用假 requests，响应用构造的火山报文。真实可用性仍需填上 appid/token 跑一次。
"""
import base64
import json
import pathlib
import sys

import pytest

import _lib
from _lib import _ratio, _swap_voice_suffix, tts


class FakeResp:
    def __init__(self, payload):
        self._p = payload

    def json(self):
        return self._p


@pytest.fixture
def volc(monkeypatch):
    """把 requests.post 换掉，记录每次调用的 (url, body, headers)，并回一段可编排的响应。"""
    calls = []
    replies = []

    def fake_post(url, data=None, headers=None, timeout=None):
        calls.append({"url": url, "data": data, "headers": headers, "timeout": timeout})
        return FakeResp(replies.pop(0) if replies else {"code": 3000, "data": "SUQz"})

    monkeypatch.setattr(_lib.requests, "post", fake_post)
    monkeypatch.setenv("VOLC_TTS_APPID", "APP1")
    monkeypatch.setenv("VOLC_TTS_TOKEN", "TOK456")
    monkeypatch.delenv("VOLC_TTS_CLUSTER", raising=False)
    return calls, replies


def test_request_matches_official_sample(volc):
    calls, _ = volc
    audio, err = tts("太热了", speed=1.2, pitch=0.9, volume=0.7)
    assert err is None and audio is not None

    c = calls[0]
    assert c["url"] == "https://openspeech.bytedance.com/api/v1/tts"
    # 官方示例：Authorization 是 "Bearer;<token>"（分号），不是空格
    assert c["headers"]["Authorization"] == "Bearer;TOK456"
    assert "Content-Type" not in c["headers"], "照官方示例发：不额外加 Content-Type"
    assert isinstance(c["data"], str), "照官方示例发：body 是 json.dumps 的字符串"

    b = json.loads(c["data"])
    assert set(b) == {"app", "user", "audio", "request"}
    assert b["app"] == {"appid": "APP1", "token": "TOK456", "cluster": "volcano_tts"}
    assert b["app"]["token"] == "TOK456", "app.token 必须是真 token（示例里写的字面量是文档的坑）"
    assert b["audio"]["voice_type"] == _lib.VOLC_TTS_VOICE    # 音色是代码常量，不是配置项
    assert b["audio"]["encoding"] == "mp3"
    assert (b["audio"]["speed_ratio"], b["audio"]["pitch_ratio"], b["audio"]["volume_ratio"]) == (1.2, 0.9, 0.7)
    assert b["request"]["text"] == "太热了" and b["request"]["operation"] == "query"
    assert b["request"]["text_type"] == "plain"
    assert b["request"]["with_frontend"] == 1 and b["request"]["frontend_type"] == "unitTson"
    assert b["request"]["reqid"] and len(b["request"]["reqid"]) >= 16
    assert "\\u" in c["data"], "中文按示例走默认 ensure_ascii（body 保持纯 ASCII）"


def test_cluster_from_env_voice_from_code(volc, monkeypatch):
    """cluster 走配置；音色是代码常量（环境变量不该再影响它）"""
    calls, _ = volc
    monkeypatch.setenv("VOLC_TTS_VOICE", "BV021_streaming")   # 故意设了也不该生效
    monkeypatch.setenv("VOLC_TTS_CLUSTER", "volcano_mega")
    tts("你好")
    b = json.loads(calls[0]["data"])
    assert b["audio"]["voice_type"] == _lib.VOLC_TTS_VOICE, "音色不该再看环境变量"
    assert b["app"]["cluster"] == "volcano_mega"


def test_audio_bytes_and_ratios(volc):
    calls, replies = volc
    replies.append({"code": 3000, "data": base64.b64encode(b"MP3DATA").decode()})
    audio, err = tts("你好", speed=9, volume=0.01, pitch=None)
    assert audio == b"MP3DATA" and err is None
    b = json.loads(calls[0]["data"])
    assert b["audio"]["speed_ratio"] == 3.0        # 越界夹到上限
    assert b["audio"]["volume_ratio"] == 0.2       # 越界夹到下限
    assert b["audio"]["pitch_ratio"] == 1.0        # 没传就是 1.0


def test_ratio_edge_cases():
    assert _ratio(None) == 1.0 and _ratio("abc") == 1.0 and _ratio("") == 1.0
    assert _ratio(1) == 1.0 and _ratio("1.234") == 1.23 and _ratio(-5) == 0.2


def test_upstream_error_is_passed_through(volc):
    calls, replies = volc
    replies.append({"code": 3001, "message": "authentication failed"})
    audio, err = tts("你好")
    assert audio is None and "3001" in err and "authentication failed" in err
    assert len(calls) == 1, "不是音色问题就不该重试"


def test_voice_id_suffix_retried_once(volc, monkeypatch):
    calls, replies = volc
    monkeypatch.setattr(_lib, "VOLC_TTS_VOICE", "BV700")     # 写法不带后缀（常量，直接改）
    replies.append({"code": 3006, "message": "voice_type invalid"})
    replies.append({"code": 3000, "data": base64.b64encode(b"OK").decode()})
    audio, err = tts("你好")
    assert audio == b"OK" and err is None
    assert len(calls) == 2
    assert json.loads(calls[0]["data"])["audio"]["voice_type"] == "BV700"
    assert json.loads(calls[1]["data"])["audio"]["voice_type"] == "BV700_streaming"


def test_swap_voice_suffix():
    assert _swap_voice_suffix("BV700") == "BV700_streaming"
    assert _swap_voice_suffix("BV700_streaming") == "BV700"


def test_missing_creds_and_network_and_empty(monkeypatch):
    for k in ("VOLC_TTS_APPID", "VOLC_TTS_TOKEN"):
        monkeypatch.delenv(k, raising=False)
    assert "VOLC_TTS_APPID" in tts("你好")[1]

    monkeypatch.setenv("VOLC_TTS_APPID", "A")
    monkeypatch.setenv("VOLC_TTS_TOKEN", "T")

    def boom(*a, **kw):
        raise RuntimeError("dns fail")
    monkeypatch.setattr(_lib.requests, "post", boom)
    assert "连不上火山" in tts("你好")[1]

    monkeypatch.setattr(_lib.requests, "post", lambda *a, **kw: FakeResp({"code": 3000, "data": ""}))
    assert "空音频" in tts("你好")[1]

    monkeypatch.setattr(_lib.requests, "post", lambda *a, **kw: FakeResp({"code": 3000, "data": "!!!not-b64"}))
    assert "base64" in tts("你好")[1]


# ---------------- 端点 ----------------
@pytest.fixture
def client(monkeypatch):
    flask = pytest.importorskip("flask")
    monkeypatch.delenv("PANEL_PASSWORD", raising=False)
    sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "api"))
    import tts as mod
    return mod.app.test_client()


def test_route_returns_audio(client, monkeypatch):
    monkeypatch.setattr("_lib.tts", lambda text, r=None, p=None, v=None: (b"MP3", None))
    import tts as mod
    monkeypatch.setattr(mod, "tts", lambda text, r=None, p=None, v=None: (b"MP3", None))
    res = client.post("/api/tts", json={"text": "你好", "rate": 1.1})
    assert res.status_code == 200 and res.data == b"MP3"
    assert res.headers["Content-Type"].startswith("audio/mpeg")


def test_route_errors(client, monkeypatch):
    import tts as mod
    res = client.post("/api/tts", json={})
    assert res.status_code == 400 and "文本为空" in res.get_json()["msg"]

    monkeypatch.setattr(mod, "tts", lambda text, r=None, p=None, v=None: (None, "后端没配 VOLC_TTS_APPID / VOLC_TTS_TOKEN"))
    res = client.post("/api/tts", json={"text": "你好"})
    assert res.status_code == 500 and "VOLC_TTS_APPID" in res.get_json()["msg"]

    monkeypatch.setattr(mod, "tts", lambda text, r=None, p=None, v=None: (None, "火山返回 3001：authentication failed（音色 BV700_streaming）"))
    res = client.post("/api/tts", json={"text": "你好"})
    assert res.status_code == 502 and "authentication" in res.get_json()["msg"]


def test_route_checks_key_and_rate(client, monkeypatch):
    import tts as mod
    monkeypatch.setenv("PANEL_PASSWORD", "PW")
    assert client.post("/api/tts", json={"text": "你好"}).status_code == 401
    # 口令对了就放行：这条走到"没配凭据" → 500（凭据问题算我们自己的，不混成上游错误）
    res = client.post("/api/tts", json={"text": "你好"}, headers={"X-Panel-Key": "PW"})
    assert res.status_code == 500 and "VOLC_TTS_APPID" in res.get_json()["msg"]
    monkeypatch.setattr(mod, "rate_ok", lambda req: False)
    assert client.post("/api/tts", json={"text": "你好"}, headers={"X-Panel-Key": "PW"}).status_code == 429


def test_route_truncates_text(client, monkeypatch):
    import tts as mod
    seen = {}
    monkeypatch.setattr(mod, "tts", lambda text, r=None, p=None, v=None: (seen.setdefault("t", text), (b"X", None))[1])
    client.post("/api/tts", json={"text": "啊" * 500})
    assert len(seen["t"]) == 300, "播报文本截到 TTS_MAX_CHARS，别把整段书丢给合成"
