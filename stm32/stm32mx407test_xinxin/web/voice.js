/* 语音控制层：浏览器 ASR（Web Speech API）→ 后端 /api/chat 出意图 → 调面板桥执行 → TTS 播报
 *
 * 依赖：config.js（PanelAPI）、app.js（window.__panel）
 * 设计：不做常开唤醒词，必须点按钮才开始听；重启这类破坏性动作不会被一句话直接执行。
 */
(function () {
  /* 注意：Vue 挂载 #app 时会重建里面的 DOM，所以节点一律**用时再取**，
     点击也用事件委托（不然抓到的是被丢弃的旧节点，点了没反应）。 */
  const $ = id => document.getElementById(id);

  const SR = window.SpeechRecognition || window.webkitSpeechRecognition;
  let rec = null, listening = false, busy = false;

  function show(msg, cls) {
    const bar = $('voiceBar'), text = $('voiceText');
    if (!bar || !text) return;
    bar.hidden = !msg;
    bar.className = 'voice-bar' + (cls ? ' ' + cls : '');
    text.textContent = msg || '';
  }
  function speak(msg) {
    if (!msg || !window.speechSynthesis) return;
    try {
      speechSynthesis.cancel();
      const u = new SpeechSynthesisUtterance(String(msg));
      u.lang = 'zh-CN';
      u.rate = 1.05;
      speechSynthesis.speak(u);
    } catch (e) { /* 播报失败不影响执行 */ }
  }
  function setLive(on) {
    listening = on;
    const btn = $('voiceBtn'), label = $('voiceLabel');
    if (btn) btn.classList.toggle('mic-live', on);
    if (label) label.textContent = on ? '在听' : '语音';
  }

  async function execute(intent) {
    const panel = window.__panel;
    if (!panel) return show('面板还没就绪', 'is-err');
    const { action, target, value, reply } = intent;

    switch (action) {
      case 'toggle': {
        if (target === null) { show(reply || '要控制哪一路？', 'is-err'); speak(reply || '要控制哪一路？'); return; }
        if (!panel.isOnline()) { show('设备离线，不能下发', 'is-err'); speak('设备离线'); return; }
        const ok = await panel.set(target, value);
        const name = (panel.controls.find(c => c.key === target) || {}).name || target;
        const msg = ok ? (reply || ((value ? '已打开' : '已关闭') + name)) : ('下发失败：' + name);
        show(msg, ok ? 'is-ok' : 'is-err');
        speak(msg);
        return;
      }
      case 'query': {
        if (target === null) { show(reply || '要查哪一项？', 'is-err'); speak(reply || '要查哪一项？'); return; }
        await panel.refresh();
        const v = panel.get(target);
        const card = panel.cards.find(c => c.id === target) || {};
        const val = (v === null || v === undefined || v === '') ? '暂时没有数据'
                  : (Number(v).toFixed ? Number(v).toFixed(1) + (card.unit || '') : String(v));
        const msg = (reply ? reply + '，' : '') + card.name + ' ' + val;
        show(msg, 'is-ok');
        speak(msg);
        return;
      }
      case 'refresh':
        panel.refresh();
        show(reply || '已刷新', 'is-ok');
        speak(reply || '已刷新');
        return;
      case 'reboot':
        /* 破坏性动作：不自动执行。把面板按钮推到“再点一次确认”，由人工点第二下 */
        if (!panel.isOnline()) { show('设备离线，不能重启', 'is-err'); speak('设备离线'); return; }
        panel.armReboot();
        const m = '识别到重启：请点面板上的「重启设备」按钮再确认一次';
        show(m, 'is-err');
        speak('要重启设备的话，请点两下面板上的重启按钮确认');
        return;
      default:
        show(reply || '没听懂，再说一次', 'is-err');
        speak(reply || '没听懂，再说一次');
    }
  }

  async function run(transcript) {
    if (busy) return;
    busy = true;
    show('解析中…' + transcript, 'is-live');
    try {
      const panel = window.__panel;
      const intent = await PanelAPI.chat(transcript, panel ? panel.controls : [], panel ? panel.cards : []);
      await execute(intent);
    } catch (err) {
      if (err.needKey) { show('口令失效，请刷新页面重新输入', 'is-err'); }
      else { show(String(err.message || err), 'is-err'); }
      console.error('[语音] 失败：', err);
    } finally {
      busy = false;
    }
  }

  function start() {
    if (!SR) return show('这个浏览器不支持语音识别（用 Chrome/Edge）', 'is-err');
    if (!PanelAPI.ensureKey()) return show('需要面板口令才能用', 'is-err');
    if (rec) { try { rec.stop(); } catch (e) {} }

    rec = new SR();
    Object.assign(rec, { lang: 'zh-CN', continuous: false, interimResults: true, maxAlternatives: 1 });
    rec.onstart  = () => { setLive(true); show('在听…说“打开灯 1”“温度多少”', 'is-live'); };
    rec.onresult = (e) => {
      let txt = '', final = false;
      for (let i = e.resultIndex; i < e.results.length; i++) {
        txt += e.results[i][0].transcript;
        if (e.results[i].isFinal) final = true;
      }
      if (!final) show(txt, 'is-live');
      if (final) { try { rec.stop(); } catch (err) {} run(txt.trim()); }
    };
    rec.onerror = (e) => {
      const m = ({
        network: '语音服务连不上（浏览器 ASR 的网络限制），可换网络或改用本地 whisper',
        'not-allowed': '麦克风被拒绝（需要 HTTPS/localhost 且允许麦克风）',
        'service-not-allowed': '系统/浏览器禁用了语音服务',
        'no-speech': '没听到说话，再点一次试试',
        aborted: ''
      })[e.error];
      if (m) show(m, 'is-err');
      else if (e.error !== 'aborted') show('识别出错：' + e.error, 'is-err');
    };
    rec.onend = () => setLive(false);
    try { rec.start(); } catch (e) { show('启动失败：' + e.message, 'is-err'); }
  }

  /* 事件委托：Vue 重建按钮后依然生效 */
  document.addEventListener('click', (e) => {
    const btn = e.target.closest && e.target.closest('#voiceBtn');
    if (!btn) return;
    if (listening) { try { rec.stop(); } catch (err) {} show(''); return; }
    start();
  });

  /* 不支持时把按钮置灰（同样等 Vue 渲染完再动） */
  if (!SR) {
    const arm = () => { const b = $('voiceBtn'); if (b) { b.disabled = true; b.title = '这个浏览器不支持语音识别'; } };
    if (document.readyState === 'complete') setTimeout(arm, 0); else window.addEventListener('load', arm);
  }
})();
