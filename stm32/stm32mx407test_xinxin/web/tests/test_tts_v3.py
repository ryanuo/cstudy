"""火山 V3（豆包语音合成 2.0）通道：帧编解码 + WS 会话流程 + 通道路由。

注意：这些测试**不连真实服务**，只按官方帧规范构造/解析二进制，
真实可用性还需要用户在控制台开通 seed-tts-2.0 后用真 key 验证一次。
"""
import json
import struct
import sys

import pytest

import _lib
from _lib import _is_v3_voice, _v3_headers, tts_volc, v3_finish_frame, v3_parse_frame, v3_send_text_frame


def u32(n):
    return struct.pack(">I", n)


def audio_frame(audio, sid=b"sid-1", event=352):
    return bytes([0x11, 0xB4, 0x10, 0x00]) + u32(event) + u32(len(sid)) + sid + u32(len(audio)) + audio


def meta_frame(event, js=b"{}"):
    return bytes([0x11, 0x94, 0x10, 0x00]) + u32(event) + u32(len(js)) + js


def err_frame(code):
    return bytes([0x11, 0xF0, 0x10, 0x00]) + u32(code)


# ---------------- 帧编解码 ----------------
def test_send_text_frame_layout():
    f = v3_send_text_frame({"a": 1, "中文": "好"})
    assert f[:4] == b"\x11\x10\x10\x00"
    n = struct.unpack(">I", f[4:8])[0]
    assert n == len(f) - 8
    assert json.loads(f[8:].decode("utf-8")) == {"a": 1, "中文": "好"}


def test_finish_frame_layout():
    f = v3_finish_frame()
    assert f[:4] == b"\x11\x14\x10\x00"
    assert struct.unpack(">I", f[4:8])[0] == 2          # event = FinishConnection
    assert struct.unpack(">I", f[8:12])[0] == 2         # payload "{}"
    assert f[12:] == b"{}"


def test_parse_audio_frame():
    f = v3_parse_frame(audio_frame(b"ID3abc"))
    assert f["type"] == "audio" and f["data"] == b"ID3abc" and f["event"] == 352


def test_parse_meta_and_error_frames():
    m = v3_parse_frame(meta_frame(152, b'{"ok":1}'))
    assert m["type"] == "meta" and m["event"] == 152 and json.loads(m["json"]) == {"ok": 1}
    e = v3_parse_frame(err_frame(40000001))
    assert e["type"] == "error" and e["code"] == 40000001
    assert v3_parse_frame(b"")["type"] == "unknown"


# ---------------- 鉴权头 / 通道选择 ----------------
def test_headers_new_console(monkeypatch):
    monkeypatch.setenv("VOLC_TTS_API_KEY", "KEY1")
    monkeypatch.setenv("VOLC_TTS_APPID", "A")
    monkeypatch.setenv("VOLC_TTS_TOKEN", "T")
    assert _v3_headers() == {"X-Api-Key": "KEY1"}        # 新版优先


def test_headers_old_console(monkeypatch):
    monkeypatch.delenv("VOLC_TTS_API_KEY", raising=False)
    monkeypatch.setenv("VOLC_TTS_APPID", "A")
    monkeypatch.setenv("VOLC_TTS_TOKEN", "T")
    assert _v3_headers() == {"X-Api-App-Id": "A", "X-Api-Access-Key": "T"}
    monkeypatch.delenv("VOLC_TTS_APPID")
    assert _v3_headers() is None


def test_voice_routing():
    assert _is_v3_voice("zh_female_vv_uranus_bigtts")
    assert _is_v3_voice("multi_xxx_bigtts")
    assert not _is_v3_voice("BV700_streaming")


def test_tts_volc_routes_by_voice(monkeypatch):
    seen = []
    monkeypatch.setattr(_lib, "tts_volc_v1_http", lambda *a, **k: (seen.append("v1") or b"V1", None))
    monkeypatch.setattr(_lib, "tts_volc_v3_ws", lambda *a, **k: (seen.append("v3") or b"V3", None))
    monkeypatch.delenv("VOLC_TTS_API", raising=False)
    assert tts_volc("hi", voice="BV700_streaming")[0] == b"V1"
    assert tts_volc("hi", voice="zh_female_vv_uranus_bigtts")[0] == b"V3"
    assert seen == ["v1", "v3"]
    # 强制指定也能覆盖
    monkeypatch.setenv("VOLC_TTS_API", "v3")
    assert tts_volc("hi", voice="BV700_streaming")[0] == b"V3"


def test_v3_error_not_masked_for_bigtts(monkeypatch):
    """2.0 音色在 V1 必然失败：V3 出错就原样报错，别偷偷回退掩盖原因"""
    monkeypatch.setattr(_lib, "tts_volc_v3_ws", lambda *a, **k: (None, "火山 V3 报错 code=1"))
    monkeypatch.setattr(_lib, "tts_volc_v1_http", lambda *a, **k: (b"should-not-reach", None))
    monkeypatch.delenv("VOLC_TTS_API", raising=False)
    audio, err = tts_volc("hi", voice="zh_female_vv_uranus_bigtts")
    assert audio is None and "V3 报错" in err


# ---------------- WS 会话流程（假 websocket 模块） ----------------
class FakeWS:
    def __init__(self, frames):
        self.frames = list(frames)
        self.sent = []
        self.closed = False
    def send_binary(self, b):
        self.sent.append(b)
    def recv(self):
        return self.frames.pop(0) if self.frames else b""
    def close(self):
        self.closed = True


@pytest.fixture
def fake_ws(monkeypatch):
    holder = {}

    def install(frames, **conn_kw):
        ws = FakeWS(frames)

        def create_connection(url, header=None, timeout=None):
            holder.update(url=url, header=header, timeout=timeout)
            return ws
        mod = type(sys)("websocket")
        mod.create_connection = create_connection
        monkeypatch.setitem(sys.modules, "websocket", mod)
        return ws, holder
    return install


def test_v3_ws_collects_audio_and_finishes(monkeypatch, fake_ws):
    monkeypatch.setenv("VOLC_TTS_API_KEY", "KEY1")
    monkeypatch.delenv("VOLC_TTS_V3_RESOURCE", raising=False)
    ws, holder = fake_ws([audio_frame(b"AA"), audio_frame(b"BB"), meta_frame(152), meta_frame(52)])

    audio, err = _lib.tts_volc_v3_ws("太热了", voice="zh_female_vv_uranus_bigtts", speed=1.2, volume=0.8)
    assert err is None and audio == b"AABB"
    assert holder["url"].startswith("wss://")
    assert holder["header"]["X-Api-Key"] == "KEY1"
    assert holder["header"]["X-Api-Resource-Id"] == "seed-tts-2.0"
    assert holder["header"]["X-Api-Connect-Id"]
    # 第一帧是 SendText（带 text/speaker/speech_rate），SessionFinished 之后要发 FinishConnection
    first = json.loads(ws.sent[0][8:].decode())
    assert first["req_params"]["speaker"] == "zh_female_vv_uranus_bigtts"
    assert first["req_params"]["audio_params"]["speech_rate"] == 20
    assert first["req_params"]["audio_params"]["loudness_rate"] == -20
    assert ws.sent[1][:4] == b"\x11\x14\x10\x00"
    assert ws.closed is True


def test_v3_ws_error_frame(monkeypatch, fake_ws):
    monkeypatch.setenv("VOLC_TTS_API_KEY", "KEY1")
    fake_ws([err_frame(40000001)])
    audio, err = _lib.tts_volc_v3_ws("hi", voice="zh_female_vv_uranus_bigtts")
    assert audio is None and "40000001" in err


def test_v3_ws_no_audio(monkeypatch, fake_ws):
    monkeypatch.setenv("VOLC_TTS_API_KEY", "KEY1")
    fake_ws([meta_frame(152), meta_frame(52)])
    audio, err = _lib.tts_volc_v3_ws("hi", voice="zh_female_vv_uranus_bigtts")
    assert audio is None and "没返回音频" in err


def test_v3_ws_connect_failure(monkeypatch):
    monkeypatch.setenv("VOLC_TTS_API_KEY", "KEY1")
    mod = type(sys)("websocket")
    def boom(*a, **kw):
        raise RuntimeError("handshake refused")
    mod.create_connection = boom
    monkeypatch.setitem(sys.modules, "websocket", mod)
    audio, err = _lib.tts_volc_v3_ws("hi", voice="zh_female_vv_uranus_bigtts")
    assert audio is None and "连不上火山 V3" in err


def test_v3_ws_requires_key(monkeypatch):
    for k in ("VOLC_TTS_API_KEY", "VOLC_TTS_APPID", "VOLC_TTS_TOKEN"):
        monkeypatch.delenv(k, raising=False)
    audio, err = _lib.tts_volc_v3_ws("hi", voice="zh_female_vv_uranus_bigtts")
    assert audio is None and "VOLC_TTS_API_KEY" in err


def test_v3_ws_missing_dependency(monkeypatch):
    monkeypatch.setenv("VOLC_TTS_API_KEY", "KEY1")
    monkeypatch.setitem(sys.modules, "websocket", None)   # import websocket -> None 时不可用
    audio, err = _lib.tts_volc_v3_ws("hi", voice="zh_female_vv_uranus_bigtts")
    assert audio is None and ("websocket-client" in err or "连不上" in err or err)


# ---------------- 就绪判断（两条通道凭据不同，别互相误杀） ----------------
def test_ready_checks_the_right_channel(monkeypatch):
    monkeypatch.delenv("VOLC_TTS_API_KEY", raising=False)
    monkeypatch.delenv("VOLC_TTS_APPID", raising=False)
    monkeypatch.delenv("VOLC_TTS_TOKEN", raising=False)
    monkeypatch.delenv("VOLC_TTS_API", raising=False)
    assert _lib.volc_tts_ready() is False
    monkeypatch.setenv("VOLC_TTS_API_KEY", "K")            # 只配新版 Key
    assert _lib.volc_tts_ready() is True                   # 任意通道可用即算"可用"
    assert _lib.volc_tts_ready("zh_female_vv_uranus_bigtts") is True
    assert _lib.volc_tts_ready("BV700_streaming") is False  # 但 V1 音色还不满足
    monkeypatch.setenv("VOLC_TTS_APPID", "A")
    monkeypatch.setenv("VOLC_TTS_TOKEN", "T")
    assert _lib.volc_tts_ready("BV700_streaming") is True


def test_api_tts_reports_missing_v3_key(monkeypatch):
    """一个凭据都没配时，请求 2.0 音色要明确提示缺哪种（V3 用 API Key）"""
    flask = pytest.importorskip("flask")
    monkeypatch.delenv("PANEL_PASSWORD", raising=False)
    for k in ("VOLC_TTS_API_KEY", "VOLC_TTS_APPID", "VOLC_TTS_TOKEN", "VOLC_TTS_API"):
        monkeypatch.delenv(k, raising=False)
    sys.path.insert(0, str(__import__("pathlib").Path(__file__).resolve().parents[1] / "api"))
    import tts as mod
    c = mod.app.test_client()
    r = c.post("/api/tts", json={"text": "你好", "voice": "zh_female_vv_uranus_bigtts"})
    assert r.status_code == 500 and "VOLC_TTS_API_KEY" in r.get_json()["msg"]


def test_api_tts_v1_voice_with_only_v1_creds_talks_to_upstream(monkeypatch):
    """V1 音色 + V1 凭据：不该被就绪检查拦下（会真去请求上游，这里断言不是 500 配置错）"""
    flask = pytest.importorskip("flask")
    monkeypatch.delenv("PANEL_PASSWORD", raising=False)
    monkeypatch.delenv("VOLC_TTS_API_KEY", raising=False)
    monkeypatch.setenv("VOLC_TTS_APPID", "A")
    monkeypatch.setenv("VOLC_TTS_TOKEN", "T")

    import requests

    def boom(*a, **kw):
        raise RuntimeError("no network in test")
    monkeypatch.setattr(requests, "post", boom)
    sys.path.insert(0, str(__import__("pathlib").Path(__file__).resolve().parents[1] / "api"))
    import tts as mod
    c = mod.app.test_client()
    r = c.post("/api/tts", json={"text": "你好", "voice": "BV700_streaming"})
    assert r.status_code == 502 and "连不上火山 TTS" in r.get_json()["msg"]


def test_api_tts_v3_accepts_old_console_creds(monkeypatch, fake_ws):
    """旧版控制台的 appid/access_token 也能给 V3 用（走 X-Api-App-Id 头）"""
    flask = pytest.importorskip("flask")
    monkeypatch.delenv("PANEL_PASSWORD", raising=False)
    monkeypatch.delenv("VOLC_TTS_API_KEY", raising=False)
    monkeypatch.setenv("VOLC_TTS_APPID", "A")
    monkeypatch.setenv("VOLC_TTS_TOKEN", "T")
    ws, holder = fake_ws([audio_frame(b"MP3DATA"), meta_frame(152), meta_frame(52)])
    sys.path.insert(0, str(__import__("pathlib").Path(__file__).resolve().parents[1] / "api"))
    import tts as mod
    r = mod.app.test_client().post("/api/tts", json={"text": "你好", "voice": "zh_female_vv_uranus_bigtts"})
    assert r.status_code == 200 and r.data == b"MP3DATA"
    assert holder["header"]["X-Api-App-Id"] == "A" and "X-Api-Key" not in holder["header"]
