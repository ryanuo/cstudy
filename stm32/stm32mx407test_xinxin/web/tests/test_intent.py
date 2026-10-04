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


# ---------------- 多轮追问（pending + 上下文） ----------------
def test_targetless_toggle_reports_pending():
    out = validate({"action": "toggle", "target": None, "value": True, "reply": "要开哪一路？"}, C3, K)
    assert out["action"] == "unknown"
    assert out["pending"] == {"action": "toggle", "target": None, "value": True}


def test_targetless_toggle_without_value_has_no_pending():
    out = validate({"action": "toggle", "target": None, "value": None}, C3, K)
    assert "pending" not in out          # 目标、开关两样都缺 → 追问不出有效动作，让用户重说


def test_toggle_with_target_but_no_value_reports_pending():
    # "灯1和灯3" 这种：知道目标、不知道开还是关 → 也要能接着追问
    out = validate({"action": "toggle", "target": "led1", "value": None}, C3, K)
    assert out["action"] == "unknown"
    assert out["pending"] == {"action": "toggle", "target": "led1", "value": None}


def test_toggle_many_with_targets_but_no_value_reports_pending():
    out = validate({"action": "toggle_many", "targets": ["led1", "led3"], "value": None}, C3, K)
    assert out["pending"] == {"action": "toggle_many", "targets": ["led1", "led3"], "value": None}


def test_chat_has_no_pending():
    assert "pending" not in validate({"action": "chat", "reply": "你好"}, C3, K)
    assert "pending" not in validate({"action": "refresh"}, C3, K)


def test_toggle_many_without_targets_reports_pending():
    out = validate({"action": "toggle_many", "targets": ["zzz"], "value": False}, C3, K)
    assert out["action"] == "unknown"
    assert out["pending"] == {"action": "toggle_many", "targets": None, "value": False}


def test_build_messages_carries_history():
    from _lib import build_messages
    hist = [{"role": "user", "content": "开灯"},
            {"role": "assistant", "content": '{"action":"unknown","reply":"要开哪一路？"}'},
            {"role": "bogus", "content": "x"}]
    m = build_messages("灯1", C3, K, hist)
    assert m[0]["role"] == "system" and m[-1] == {"role": "user", "content": "灯1"}
    assert [x["role"] for x in m[1:-1]] == ["user", "assistant"]      # 非法的 role 被丢掉
    assert build_messages("灯1", C3, K) [-1]["content"] == "灯1"


def test_build_messages_trims_history():
    from _lib import build_messages
    hist = [{"role": "user", "content": "n%d" % i} for i in range(20)]
    m = build_messages("hi", C3, K, hist)
    assert len(m) == 1 + 6 + 1          # 系统 + 最近 6 条 + 本轮


# ---------------- 一句话多件事（steps / 场景） ----------------
def test_steps_keeps_valid_multistep():
    out = validate({"action": "steps", "steps": [
        {"action": "toggle", "target": "fan", "value": True},
        {"action": "query", "target": "temperature"}], "reply": "已打开风扇"}, C3, K)
    assert out["action"] == "steps" and len(out["steps"]) == 2
    assert out["steps"][0] == {"action": "toggle", "target": "fan", "targets": None, "value": True}
    assert out["steps"][1]["action"] == "query" and out["steps"][1]["target"] == "temperature"


def test_steps_drops_illegal_inner_actions():
    # reboot / chat / steps 套 steps 都不允许出现在 steps 里
    out = validate({"action": "steps", "steps": [
        {"action": "toggle", "target": "fan", "value": True},
        {"action": "reboot"},
        {"action": "chat", "reply": "hi"},
        {"action": "steps", "steps": []}]}, C3, K)
    assert out["action"] == "toggle" and out["target"] == "fan"      # 只剩一步 → 回落


def test_steps_drops_hallucinated_target_step():
    out = validate({"action": "steps", "steps": [
        {"action": "toggle", "target": "led9", "value": True},
        {"action": "query", "target": "temperature"}]}, C3, K)
    assert out["action"] == "query" and out["target"] == "temperature"


def test_steps_all_invalid_becomes_unknown():
    out = validate({"action": "steps", "steps": [{"action": "reboot"}, {"action": "nope"}]}, C3, K)
    assert out["action"] == "unknown"
    assert validate({"action": "steps", "steps": "nope"}, C3, K)["action"] == "unknown"
    assert validate({"action": "steps"}, C3, K)["action"] == "unknown"


def test_steps_capped():
    many = [{"action": "toggle", "target": "led1", "value": True} for _ in range(9)]
    out = validate({"action": "steps", "steps": many}, C3, K)
    assert len(out["steps"]) == 3
