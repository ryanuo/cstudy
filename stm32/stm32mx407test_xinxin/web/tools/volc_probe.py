"""火山 TTS 探针：直接用你的凭据验证通道，不走浏览器/面板。

它调用的是 web/api/_lib.py 里面板**同一份**实现，所以结果就代表面板能不能用。

用法（凭据放环境变量，别写进命令行历史）：
    export VOLC_TTS_APPID=xxx VOLC_TTS_TOKEN=xxx VOLC_TTS_CLUSTER=volcano_tts
    .venv/bin/python web/tools/volc_probe.py BV700_streaming "你好，试一下音色"

    # 豆包 2.0 / Vivi 2.0（走 V3 WebSocket，用新版控制台的 API Key）
    export VOLC_TTS_API_KEY=xxx
    .venv/bin/python web/tools/volc_probe.py zh_female_vv_uranus_bigtts "你好"

    # 想完全照官方示例发（data=json.dumps，不带 Content-Type）对比一下：
    .venv/bin/python web/tools/volc_probe.py BV700_streaming "你好" --style=sample

成功会打印音频字节数并写到 /tmp/volc_probe.mp3；失败会把上游原样返回的 code/message 打出来。
"""
import os
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "api"))
import _lib                                    # noqa: E402


def load_env_local():
    p = pathlib.Path(__file__).resolve().parents[1] / ".env.local"
    if not p.exists():
        return
    for line in p.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if line and not line.startswith("#") and "=" in line:
            k, v = line.split("=", 1)
            if v.strip() and not os.environ.get(k.strip()):
                os.environ[k.strip()] = v.strip()


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    style = "json"
    for a in sys.argv[1:]:
        if a.startswith("--style="):
            style = a.split("=", 1)[1]
    voice = args[0] if args else "BV700_streaming"
    text = args[1] if len(args) > 1 else "你好，这是一次音色测试。"

    load_env_local()
    print("通道判断：%s → %s" % (voice, "V3 WebSocket" if _lib._is_v3_voice(voice) else "V1 HTTP"))
    print("凭据：API_KEY=%s APPID=%s TOKEN=%s CLUSTER=%s" % (
        "有" if os.environ.get("VOLC_TTS_API_KEY") else "无",
        "有" if os.environ.get("VOLC_TTS_APPID") else "无",
        "有" if os.environ.get("VOLC_TTS_TOKEN") else "无",
        os.environ.get("VOLC_TTS_CLUSTER", "volcano_tts")))
    print("音色：%s   文本：%s" % (voice, text))

    if style == "sample" and not _lib._is_v3_voice(voice):
        # 完全照官方示例：data=json.dumps(...) 且不带 Content-Type
        import base64, uuid, requests
        payload = {
            "app": {"appid": os.environ.get("VOLC_TTS_APPID", ""),
                    "token": os.environ.get("VOLC_TTS_TOKEN", ""),
                    "cluster": os.environ.get("VOLC_TTS_CLUSTER", "volcano_tts")},
            "user": {"uid": "volc-probe"},
            "audio": {"voice_type": voice, "encoding": "mp3",
                      "speed_ratio": 1.0, "volume_ratio": 1.0, "pitch_ratio": 1.0},
            "request": {"reqid": str(uuid.uuid4()), "text": text, "text_type": "plain",
                        "operation": "query", "with_frontend": 1, "frontend_type": "unitTson"},
        }
        r = requests.post("https://openspeech.bytedance.com/api/v1/tts",
                          json.dumps(payload),
                          headers={"Authorization": "Bearer;" + os.environ.get("VOLC_TTS_TOKEN", "")},
                          timeout=20)
        j = r.json()
        print("上游原始返回：", json.dumps({k: (v[:60] + "…" if k == "data" and isinstance(v, str) else v)
                                       for k, v in j.items()}, ensure_ascii=False)[:600])
        if j.get("code") == 3000:
            open("/tmp/volc_probe.mp3", "wb").write(base64.b64decode(j["data"]))
            print("✅ 成功，音频写到 /tmp/volc_probe.mp3")
        else:
            print("❌ 失败（按官方示例发的，说明示例写法也不通，问题在凭据/参数）")
        return

    audio, err = _lib.tts_volc(text, voice=voice, speed=1.0, pitch=1.0, volume=1.0)
    if err:
        print("❌ 失败：%s" % err)
        print("   （把上面这行原文贴出来就能定位：多半是 cluster / 音色 id / 服务没开通）")
        return
    pathlib.Path("/tmp/volc_probe.mp3").write_bytes(audio)
    print("✅ 成功：%d 字节，已写 /tmp/volc_probe.mp3（用 afplay /tmp/volc_probe.mp3 听一下）" % len(audio))


if __name__ == "__main__":
    main()
