import os

import pytest

from _lib import OP_MAP, key_ok, onenet_call, rate_ok


class FakeRequest:
    def __init__(self, headers=None, ip="1.2.3.4"):
        self.headers = headers or {}
        self.remote_addr = ip


@pytest.fixture(autouse=True)
def _env(monkeypatch):
    monkeypatch.setenv("PANEL_PASSWORD", "s3cret")
    monkeypatch.delenv("UPSTASH_REDIS_REST_URL", raising=False)
    monkeypatch.delenv("UPSTASH_REDIS_REST_TOKEN", raising=False)


def test_key_ok():
    assert key_ok(FakeRequest({"X-Panel-Key": "s3cret"}))
    assert not key_ok(FakeRequest({"X-Panel-Key": "wrong"}))
    assert not key_ok(FakeRequest({}))


def test_rate_ok_passes_without_upstash():
    assert rate_ok(FakeRequest())          # 没配 Redis 时放行（本地开发）


def test_onenet_call_rejects_unknown_op():
    data, code = onenet_call("device/list", {}, None)
    assert code == 403 and data["code"] == 403


def test_onenet_call_get_uses_query(monkeypatch):
    seen = {}

    def fake_request(method, url, **kw):
        seen.update(method=method, url=url, kw=kw)
        class R:
            status_code = 200
            def json(self):
                return {"code": 0, "data": {"x": 1}}
        return R()

    monkeypatch.setenv("ONENET_PRODUCT_ID", "PID")
    monkeypatch.setenv("ONENET_DEVICE_NAME", "DID")
    monkeypatch.setenv("ONENET_TOKEN", "TOK")
    monkeypatch.setattr("_lib.requests.request", fake_request)
    data, code = onenet_call("getProperty", {"extra": "v"}, None)
    assert code == 200 and data["code"] == 0
    assert seen["method"] == "GET"
    assert seen["url"] == "https://iot-api.heclouds.com/thingmodel/query-device-property"
    assert seen["kw"]["params"] == {"product_id": "PID", "device_name": "DID", "extra": "v"}
    assert seen["kw"]["headers"]["authorization"] == "TOK"
    assert "json" not in seen["kw"]


def test_onenet_call_post_sends_body(monkeypatch):
    seen = {}

    def fake_request(method, url, **kw):
        seen.update(method=method, kw=kw)
        class R:
            status_code = 200
            def json(self):
                return {"code": 0}
        return R()

    monkeypatch.setattr("_lib.requests.request", fake_request)
    onenet_call("callService", None, {"identifier": "reboot", "params": {}})
    assert seen["method"] == "POST" and seen["kw"]["json"]["identifier"] == "reboot"


def test_op_map_is_the_whitelist():
    assert set(OP_MAP) == {"deviceDetail", "getProperty", "getHistory", "setProperty", "callService"}
