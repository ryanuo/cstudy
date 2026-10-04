"""火山 TTS（豆包大模型 2.0 / HTTP Chunked 单向流式）的报文形状、流式解析、端点行为。

不连真实服务：requests.post 用假的（回一个可编排的分块流），断言请求形状与各种失败路径。
真实可用性仍需配好 API Key 后跑一次。
"""
import base64
import json
import pathlib
import sys

import pytest

import _lib
from _lib import _offset, _stream_objs, _v3_headers, tts


class FakeStream:
    """假的 requests 响应：status_code + 分块字节流。"""
    def __init__(self, chunks, status=200, text=""):
        self._chunks = [c if isinstance(c, bytes) else c.encode() for c in chunks]
        self.status_code = status
        self.text = text

    def iter_content(self, chunk_size=None):
        yield from self._chunks


def obj(**kw):
    return json.dumps(kw, ensure_ascii=False)


@pytest.fixture
def volc(monkeypatch):
    """换掉 requests.post：记录调用，并按编排好的分块流返回。"""
    calls = []
    replies = []

    def fake_post(url, json=None, headers=None, stream=None, timeout=None):
        calls.append({"url": url, "json": json, "headers": headers, "stream": stream, "timeout": timeout})
        return replies.pop(0)

    monkeypatch.setattr(_lib.requests, "post", fake_post)
    monkeypatch.setenv("VOLC_TTS_API_KEY", "KEY-1")
    monkeypatch.delenv("VOLC_TTS_APPID", raising=False)
    monkeypatch.delenv("VOLC_TTS_ACCESS_KEY", raising=False)
    return calls, replies


# ---------------- 请求形状 ----------------
def test_request_matches_doc(volc):
    calls, replies = volc
    replies.append(FakeStream([obj(code=0, message="OK", data=base64.b64encode(b"MP3").decode())]))
    audio, err = tts("太热了", speed=1.2, pitch=0.8, volume=0.9)
    assert audio == b"MP3" and err is None

    c = calls[0]
    assert c["url"] == "https://openspeech.bytedance.com/api/v3/tts/unidirectional"
    assert c["headers"]["X-Api-Key"] == "KEY-1"
    assert c["headers"]["X-Api-Resource-Id"] == "seed-tts-2.0"
    assert len(c["headers"]["X-Api-Request-Id"]) >= 16      # 文档标了必选
    assert c["stream"] is True, "要流式收包"

    rp = c["json"]["req_params"]
    assert rp["text"] == "太热了"
    assert rp["speaker"] == _lib.VOLC_TTS_VOICE == "zh_female_vv_uranus_bigtts"
    assert rp["audio_params"]["format"] == "mp3"
    assert rp["audio_params"]["sample_rate"] == 24000
    assert rp["audio_params"]["speech_rate"] == 20          # 1.2 → +20
    assert rp["audio_params"]["loudness_rate"] == -10       # 0.9 → -10
    assert rp["post_process"]["pitch"] == -2                # 0.8 → -2.4 取整
    assert set(c["json"]) == {"req_params"}


def test_old_console_creds(monkeypatch):
    for k in ("VOLC_TTS_API_KEY", "VOLC_TTS_ACCESS_KEY", "VOLC_TTS_TOKEN"):
        monkeypatch.delenv(k, raising=False)
    assert _v3_headers() is None                            # 一个都没有 → 不可用
    monkeypatch.setenv("VOLC_TTS_APPID", "A1")
    monkeypatch.setenv("VOLC_TTS_ACCESS_KEY", "AK")
    h = _v3_headers()
    assert h["X-Api-App-Id"] == "A1" and h["X-Api-Access-Key"] == "AK" and "X-Api-Key" not in h
    monkeypatch.delenv("VOLC_TTS_ACCESS_KEY")
    monkeypatch.setenv("VOLC_TTS_TOKEN", "TOK")             # 旧名字（V1 的 access_token）也认
    assert _v3_headers()["X-Api-Access-Key"] == "TOK"


def test_offset_mapping():
    assert _offset(None) == 0 and _offset("abc") == 0 and _offset(1) == 0
    assert _offset(1.2) == 20 and _offset(2.0) == 100 and _offset(0.5) == -50
    assert _offset(3.0) == 100 and _offset(0.1) == -50       # 夹到区间
    assert _offset(1.5, 12, -12, 12) == 6 and _offset(2.0, 12, -12, 12) == 12
    assert _offset(0.5, 12, -12, 12) == -6


# ---------------- 流式解析 ----------------
def test_stream_parses_frames_split_and_glued():
    """一块可能是半条 JSON、也可能粘了两条：按 buffer 逐个 raw_decode。"""
    a, b = obj(code=0, data="AAA"), obj(code=0, data="BBB")
    raw = (a + b).encode()
    chunks = [raw[:12], raw[12:40], raw[40:60], raw[60:]]      # 任意切
    got = list(_stream_objs(iter(chunks)))
    assert [g["data"] for g in got] == ["AAA", "BBB"]

    # 带 data: 前缀 / 空块 / 噪声也不能崩
    noisy = [b"", b"data: " + obj(code=0, data="C").encode(), b"\n\n", b"garbage", obj(code=0, data="D").encode()]
    assert [g["data"] for g in _stream_objs(iter(noisy))] == ["C", "D"]


def test_real_world_success_code_20000000(volc):
    """实测这条接口成功帧回的是 code=20000000 + message=OK（不是文档另一页写的 0）：
    必须当成功收下音频，而不是报"火山返回 20000000"。"""
    calls, replies = volc
    replies.append(FakeStream([
        obj(code=20000000, message="OK", data=base64.b64encode(b"ID3-first").decode()),
        obj(code=20000000, message="OK", data=base64.b64encode(b"-second").decode()),
        obj(code=20000000, message="OK"),                      # 收尾帧没有 data
    ]))
    audio, err = tts("你好")
    assert err is None and audio == b"ID3-first-second"


def test_multi_chunk_audio_is_concatenated(volc):
    calls, replies = volc
    replies.append(FakeStream([
        obj(code=0, message="OK", data=base64.b64encode(b"ID3").decode()),
        obj(code=0, message="OK", data=base64.b64encode(b"-rest").decode()),
        obj(code=0, message="OK", usage={"text_words": 4}),
    ]))
    audio, err = tts("你好")
    assert audio == b"ID3-rest" and err is None
    assert len(calls) == 1


def test_error_frame_mid_stream_stops(volc):
    calls, replies = volc
    replies.append(FakeStream([
        obj(code=0, message="OK", data=base64.b64encode(b"PART").decode()),
        obj(code=45000001, message="invalid speaker"),          # 真正的错误码照旧要拦
    ]))
    audio, err = tts("你好")
    assert audio is None and "45000001" in err and "invalid speaker" in err
    assert _lib.VOLC_TTS_VOICE in err, "报错里带上是哪个音色，方便定位"


def test_http_error_status_and_empty_audio(volc):
    calls, replies = volc
    replies.append(FakeStream([], status=401, text="invalid api key"))     # 非 JSON 的裸文本
    audio, err = tts("你好")
    assert audio is None and "401" in err and "invalid api key" in err

    # 实测的真实错误体：{"header":{"code":45000010,"message":"load grant: …"}} → 只提 code/message
    replies.append(FakeStream([], status=401, text=json.dumps(
        {"header": {"reqid": "x", "code": 45000010, "message": "load grant: requested grant not found in SaaS storage"}})))
    err = tts("你好")[1]
    assert "45000010" in err and "load grant" in err and "reqid" not in err

    replies.append(FakeStream([obj(code=0, message="OK", data="")]))
    assert "没返回音频" in tts("你好")[1]


def test_missing_creds_network_and_bad_base64(volc, monkeypatch):
    monkeypatch.delenv("VOLC_TTS_API_KEY", raising=False)
    assert "VOLC_TTS_API_KEY" in tts("你好")[1]

    monkeypatch.setenv("VOLC_TTS_API_KEY", "K")
    monkeypatch.setattr(_lib.requests, "post", lambda *a, **kw: (_ for _ in ()).throw(RuntimeError("dns fail")))
    assert "连不上火山" in tts("你好")[1]

    monkeypatch.setattr(_lib.requests, "post", lambda *a, **kw: FakeStream([obj(code=0, data="!!!notb64")]))
    assert "base64" in tts("你好")[1]


# ---------------- 端点 ----------------
@pytest.fixture
def client(monkeypatch):
    pytest.importorskip("flask")
    monkeypatch.delenv("PANEL_PASSWORD", raising=False)
    sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "api"))
    import tts as mod
    return mod


def test_route_returns_audio(client, monkeypatch):
    monkeypatch.setattr(client, "tts", lambda text, r=None, p=None, v=None: (b"MP3", None))
    res = client.app.test_client().post("/api/tts", json={"text": "你好", "rate": 1.1})
    assert res.status_code == 200 and res.data == b"MP3"
    assert res.headers["Content-Type"].startswith("audio/mpeg")


def test_route_errors(client, monkeypatch):
    c = client.app.test_client()
    assert c.post("/api/tts", json={}).status_code == 400

    monkeypatch.setattr(client, "tts", lambda text, r=None, p=None, v=None: (None, "后端没配 VOLC_TTS_API_KEY（新版控制台）"))
    res = c.post("/api/tts", json={"text": "你好"})
    assert res.status_code == 500 and "VOLC_TTS_API_KEY" in res.get_json()["msg"]

    monkeypatch.setattr(client, "tts", lambda text, r=None, p=None, v=None: (None, "火山返回 45000001：invalid speaker（音色 zh_female_vv_uranus_bigtts）"))
    res = c.post("/api/tts", json={"text": "你好"})
    assert res.status_code == 502 and "invalid speaker" in res.get_json()["msg"]


def test_route_key_rate_and_truncate(client, monkeypatch):
    monkeypatch.setenv("PANEL_PASSWORD", "PW")
    monkeypatch.setattr(client, "tts", lambda text, r=None, p=None, v=None: (b"X", None))
    c = client.app.test_client()
    assert c.post("/api/tts", json={"text": "你好"}).status_code == 401
    monkeypatch.setattr(client, "rate_ok", lambda req: False)
    assert c.post("/api/tts", json={"text": "你好"}, headers={"X-Panel-Key": "PW"}).status_code == 429

    seen = {}
    monkeypatch.setattr(client, "rate_ok", lambda req: True)
    monkeypatch.setattr(client, "tts", lambda text, r=None, p=None, v=None: (seen.setdefault("t", text), (b"X", None))[1])
    c.post("/api/tts", json={"text": "啊" * 500}, headers={"X-Panel-Key": "PW"})
    assert len(seen["t"]) == 300


def test_health_reports_tts_channel(monkeypatch):
    """/api/health 要报出"实际用哪条朗读通道"，面板靠它显示云端音色/凭据状态"""
    pytest.importorskip("flask")
    sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "api"))
    monkeypatch.setenv("VOLC_TTS_API_KEY", "K")
    import health
    d = health.app.test_client().get("/api/health").get_json()
    assert d["ok"] is True
    assert d["tts"]["voice"] == _lib.VOLC_TTS_VOICE
    assert d["tts"]["ready"] is True

    monkeypatch.delenv("VOLC_TTS_API_KEY")
    for k in ("VOLC_TTS_APPID", "VOLC_TTS_ACCESS_KEY", "VOLC_TTS_TOKEN"):
        monkeypatch.delenv(k, raising=False)
    assert health.app.test_client().get("/api/health").get_json()["tts"]["ready"] is False

