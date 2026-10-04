from _lib import build_prompt, parse_json_loose, validate

C = [{"key": "led1", "name": "灯 1"}, {"key": "fan", "name": "风扇"}]
K = [{"id": "temperature", "name": "温度", "unit": "℃"}]


def test_hallucinated_target_dropped():
    assert validate({"action": "toggle", "target": "led4", "value": True}, C, K)["target"] is None


def test_bad_action_rejected():
    out = validate({"action": "shutdown", "value": 1, "reply": "关机"}, C, K)
    assert out["action"] == "unknown" and out["value"] is None


def test_toggle_needs_bool_and_target():
    assert validate({"action": "toggle", "target": "led1", "value": "yes"}, C, K)["action"] == "unknown"
    assert validate({"action": "toggle", "target": None, "value": True}, C, K)["action"] == "unknown"


def test_valid_toggle_passes():
    out = validate({"action": "toggle", "target": "led1", "value": True, "reply": "开灯"}, C, K)
    assert out["target"] == "led1" and out["value"] is True and out["action"] == "toggle"


def test_reboot_keeps_value_null():
    out = validate({"action": "reboot", "value": True, "reply": "重启"}, C, K)
    assert out["action"] == "reboot" and out["value"] is None and out["target"] is None


def test_query_with_target():
    out = validate({"action": "query", "target": "temperature", "value": 1}, C, K)
    assert out["target"] == "temperature" and out["value"] is None


def test_reply_truncated():
    assert len(validate({"action": "chat", "reply": "啊" * 100}, C, K)["reply"]) == 40


def test_parse_json_loose():
    assert parse_json_loose('{"action":"chat"}')["action"] == "chat"
    assert parse_json_loose('```json\n{"action":"refresh"}\n```')["action"] == "refresh"
    assert parse_json_loose('好的：{"action":"query","target":"temperature"}')["target"] == "temperature"
    assert parse_json_loose("完全不是 json") is None


def test_prompt_lists_allowed_keys_only():
    p = build_prompt(C, K)
    assert "led1" in p and "temperature" in p and "只能取" in p


# ---------------- 多目标（toggle_many） ----------------
C3 = [{"key": "led1", "name": "灯 1"}, {"key": "led2", "name": "灯 2"},
      {"key": "led3", "name": "灯 3"}, {"key": "fan", "name": "风扇"}]


def test_toggle_many_keeps_all_valid_targets():
    out = validate({"action": "toggle_many", "targets": ["led1", "led2", "led3"], "value": False,
                    "reply": "关闭三路灯"}, C3, K)
    assert out["action"] == "toggle_many"
    assert out["targets"] == ["led1", "led2", "led3"]
    assert out["value"] is False and out["target"] is None


def test_toggle_many_drops_hallucinated_and_dupes():
    # led9 是幻觉、led1 重复 -> 只留 led1+led2（保持出现顺序，仍是多目标）
    out = validate({"action": "toggle_many", "targets": ["led1", "led9", "led2", "led1"], "value": True}, C3, K)
    assert out["action"] == "toggle_many" and out["targets"] == ["led1", "led2"]


def test_toggle_many_single_target_falls_back_to_toggle():
    out = validate({"action": "toggle_many", "targets": ["fan", "nope"], "value": True}, C3, K)
    assert out["action"] == "toggle" and out["target"] == "fan"


def test_toggle_many_requires_bool_and_targets():
    assert validate({"action": "toggle_many", "targets": ["led1"], "value": "off"}, C3, K)["action"] == "unknown"
    assert validate({"action": "toggle_many", "targets": [], "value": True}, C3, K)["action"] == "unknown"
    assert validate({"action": "toggle_many", "targets": "led1", "value": True}, C3, K)["action"] == "unknown"
    assert validate({"action": "toggle_many", "targets": ["zzz"], "value": True}, C3, K)["action"] == "unknown"


def test_toggle_many_capped():
    many = [{"key": "k%d" % i} for i in range(20)]
    out = validate({"action": "toggle_many", "targets": [c["key"] for c in many], "value": True}, many, [])
    assert len(out["targets"]) == 8


def test_all_actions_have_full_schema():
    for it in [{"action": "chat", "reply": "hi"}, {"action": "refresh"}, {"action": "query", "target": "temperature"}]:
        out = validate(it, C3, K)
        assert set(out) >= {"action", "target", "targets", "value", "reply"}
