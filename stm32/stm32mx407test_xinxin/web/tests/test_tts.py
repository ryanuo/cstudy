import base64
import json

import pytest

import _lib
from _lib import tts_volc, volc_tts_ready, volc_voices


@pytest.fixture(autouse=True)
def _env(monkeypatch):
    monkeypatch.setenv("VOLC_TTS_APPID", "APP123")
    monkeypatch.setenv("VOLC_TTS_TOKEN", "TOK456")
    monkeypatch.delenv("VOLC_TTS_CLUSTER", raising=False)
    monkeypatch.delenv("VOLC_TTS_VOICES", raising=False)
    monkeypatch.delenv("VOLC_TTS_VOICE", raising=False)


def _fake_post(captured, code=3000, message="Success", data=None):
    class R:
        status_code = 200
        def json(self):
            return {"code": code, "message": message, "data": data}
    def post(url, **kw):
        captured.update(url=url, kw=kw)
        return R()
    return post


def test_payload_and_auth_header(monkeypatch):
    """官方 v1 形态：Bearer;<token>，body 里 app(appid/token/cluster) + audio + request"""
    cap = {}
    mp3 = b"ID3fake-mp3"
    monkeypatch.setattr(_lib.requests, "post", _fake_post(cap, data=base64.b64encode(mp3).decode()))
    audio, err = tts_volc("太热了", voice="BV001_streaming", speed=1.2, pitch=0.9, volume=0.8)
    assert err is None and audio == mp3
    assert cap["url"] == "https://openspeech.bytedance.com/api/v1/tts"
    assert cap["kw"]["headers"]["Authorization"] == "Bearer;TOK456"
    b = cap["kw"]["json"]
    assert b["app"] == {"appid": "APP123", "token": "TOK456", "cluster": "volcano_tts"}
    assert b["audio"]["voice_type"] == "BV001_streaming"
    assert b["audio"]["encoding"] == "mp3"
    assert (b["audio"]["speed_ratio"], b["audio"]["volume_ratio"], b["audio"]["pitch_ratio"]) == (1.2, 0.8, 0.9)
    assert b["request"]["text"] == "太热了" and b["request"]["operation"] == "query"
    assert b["request"]["reqid"]


def test_default_voice_and_cluster_from_env(monkeypatch):
    monkeypatch.setenv("VOLC_TTS_VOICE", "BV700_streaming")
    monkeypatch.setenv("VOLC_TTS_CLUSTER", "volcano_mega")
    cap = {}
    monkeypatch.setattr(_lib.requests, "post", _fake_post(cap, data=base64.b64encode(b"x").decode()))
    tts_volc("你好")
    assert cap["kw"]["json"]["audio"]["voice_type"] == "BV700_streaming"
    assert cap["kw"]["json"]["app"]["cluster"] == "volcano_mega"


def test_upstream_error_is_returned_verbatim(monkeypatch):
    monkeypatch.setattr(_lib.requests, "post", _fake_post({}, code=3001, message="invalid appid"))
    audio, err = tts_volc("你好")
    assert audio is None and "3001" in err and "invalid appid" in err


def test_not_configured(monkeypatch):
    monkeypatch.delenv("VOLC_TTS_APPID")
    assert volc_tts_ready() is False
    audio, err = tts_volc("你好")
    assert audio is None and "VOLC_TTS_APPID" in err


def test_network_error_is_caught(monkeypatch):
    def boom(*a, **kw):
        raise RuntimeError("connection reset")
    monkeypatch.setattr(_lib.requests, "post", boom)
    audio, err = tts_volc("你好")
    assert audio is None and "连不上火山" in err


def test_default_voice_list_is_the_21_free_ones():
    v = volc_voices()
    assert len(v) == 21, "免费音色应有 21 款"
    ids = {x["id"] for x in v}
    assert {"BV700_streaming", "BV001_streaming", "BV021_streaming", "BV503_streaming"} <= ids
    assert {x["group"] for x in v} == {"通用场景", "有声阅读", "助手·配音·教育", "方言", "英语", "日语"}


def test_voice_list_can_be_overridden_by_env(monkeypatch):
    monkeypatch.setenv("VOLC_TTS_VOICES", json.dumps([{"id": "BV005_streaming", "name": "测试音色", "group": "自定义"}]))
    assert volc_voices() == [{"id": "BV005_streaming", "name": "测试音色", "group": "自定义"}]
    monkeypatch.setenv("VOLC_TTS_VOICES", "{坏 JSON")
    assert len(volc_voices()) == 21        # 解析失败退回默认，不要炸


def test_voice_id_suffix_fallback(monkeypatch):
    """不同接口要 BV700 或 BV700_streaming：第一次报音色错，换写法重试一次"""
    calls = []

    class R:
        def __init__(self, code, msg, data=None):
            self.status_code = 200
            self._j = {"code": code, "message": msg, "data": data}
        def json(self):
            return self._j

    def post(url, **kw):
        vid = kw["json"]["audio"]["voice_type"]
        calls.append(vid)
        if vid.endswith("_streaming"):
            return R(3001, "voice type not found")
        return R(3000, "Success", data=base64.b64encode(b"MP3").decode())

    monkeypatch.setattr(_lib.requests, "post", post)
    audio, err = tts_volc("你好", voice="BV700_streaming")
    assert err is None and audio == b"MP3"
    assert calls == ["BV700_streaming", "BV700"], "应该退掉 _streaming 再试一次"

    # 反向：短名失败就补后缀
    calls.clear()
    def post2(url, **kw):
        vid = kw["json"]["audio"]["voice_type"]
        calls.append(vid)
        return R(3001, "voice type not found") if not vid.endswith("_streaming") else R(3000, "ok", base64.b64encode(b"M").decode())
    monkeypatch.setattr(_lib.requests, "post", post2)
    assert tts_volc("你好", voice="BV700")[0] == b"M"
    assert calls == ["BV700", "BV700_streaming"]


def test_other_errors_do_not_retry(monkeypatch):
    """不是音色问题的错误不要瞎重试（避免掩盖真原因）"""
    calls = []

    class R:
        status_code = 200
        def json(self):
            return {"code": 3002, "message": "quota exceeded", "data": None}

    def post(url, **kw):
        calls.append(kw["json"]["audio"]["voice_type"])
        return R()

    monkeypatch.setattr(_lib.requests, "post", post)
    audio, err = tts_volc("你好", voice="BV700_streaming")
    assert audio is None and "quota exceeded" in err and len(calls) == 1


# ---------- 接口层（需要 flask：用 venv 跑） ----------
flask = pytest.importorskip("flask")


def _client():
    import sys, pathlib
    sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "api"))
    import tts
    return tts.app.test_client(), tts


def test_api_tts_requires_key(monkeypatch):
    monkeypatch.setenv("PANEL_PASSWORD", "s3cret")
    c, _ = _client()
    assert c.post("/api/tts", json={"text": "hi"}).status_code == 401
    assert c.post("/api/tts", json={"text": "hi"},
                  headers={"X-Panel-Key": "s3cret"}).status_code != 401


def test_api_tts_returns_audio(monkeypatch):
    monkeypatch.delenv("PANEL_PASSWORD", raising=False)
    c, mod = _client()
    monkeypatch.setattr(mod, "tts_volc", lambda *a, **kw: (b"ID3ok", None))
    monkeypatch.setattr(mod, "volc_tts_ready", lambda: True)
    r = c.post("/api/tts", json={"text": "太热了", "voice": "BV001_streaming"})
    assert r.status_code == 200 and r.headers["Content-Type"] == "audio/mpeg" and r.data == b"ID3ok"


def test_api_tts_reports_upstream_error(monkeypatch):
    monkeypatch.delenv("PANEL_PASSWORD", raising=False)
    c, mod = _client()
    monkeypatch.setattr(mod, "tts_volc", lambda *a, **kw: (None, "火山报错 code=3001 x"))
    monkeypatch.setattr(mod, "volc_tts_ready", lambda: True)
    r = c.post("/api/tts", json={"text": "hi"})
    assert r.status_code == 502 and "3001" in r.get_json()["msg"]


def test_api_tts_info_lists_voices(monkeypatch):
    monkeypatch.delenv("PANEL_PASSWORD", raising=False)
    monkeypatch.setenv("VOLC_TTS_VOICES", json.dumps([{"id": "BV001_streaming", "name": "通用女声"}]))
    c, _ = _client()
    j = c.get("/api/tts").get_json()
    assert j["voices"] == [{"id": "BV001_streaming", "name": "通用女声", "group": ""}]
    assert "configured" in j
