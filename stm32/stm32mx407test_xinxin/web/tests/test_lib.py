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


def test_onenet_call_post_injects_product_and_device(monkeypatch):
    """★ 回归：POST 必须自己补 product_id/device_name，漏了平台只回 10001 parameter error"""
    seen = {}

    def fake_request(method, url, **kw):
        seen.update(method=method, url=url, kw=kw)
        class R:
            status_code = 200
            def json(self):
                return {"code": 0}
        return R()

    monkeypatch.setenv("ONENET_PRODUCT_ID", "PID")
    monkeypatch.setenv("ONENET_DEVICE_NAME", "DID")
    monkeypatch.setattr("_lib.requests.request", fake_request)

    # ① 前端只给属性表 -> 必须被塞进 params（平台要 {product_id,device_name,params:{…}}）
    onenet_call("setProperty", None, {"led1": "off", "led2": "off"})
    body = seen["kw"]["json"]
    assert seen["method"] == "POST"
    assert body["product_id"] == "PID" and body["device_name"] == "DID"
    assert body["params"] == {"led1": "off", "led2": "off"}
    assert "led1" not in body               # 不能平铺在顶层（平台回 10001 Params required）

    # ② 调用方已经包好 params 也不重复包
    seen.clear()
    onenet_call("setProperty", None, {"params": {"fan": True}})
    assert seen["kw"]["json"]["params"] == {"fan": True}

    # ③ callService：顶层字段 + params 默认空对象
    seen.clear()
    onenet_call("callService", None, {"identifier": "reboot"})
    body = seen["kw"]["json"]
    assert body["identifier"] == "reboot" and body["params"] == {}
    assert body["product_id"] == "PID" and body["device_name"] == "DID"

    # ④ 调用方自己传了 product_id 就不覆盖
    seen.clear()
    onenet_call("setProperty", None, {"product_id": "OTHER", "params": {"led1": "on"}})
    assert seen["kw"]["json"]["product_id"] == "OTHER"


def test_op_map_is_the_whitelist():
    assert set(OP_MAP) == {"deviceDetail", "getProperty", "getHistory", "setProperty", "callService"}
