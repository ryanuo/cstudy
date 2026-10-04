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

  /* 多轮：追问"要开哪一路？"之后要能听懂用户只答一句"灯 1"。
     所以把最近几轮带给后端，并在追问后自动重新开麦。 */
  let context = [];          // [{role:'user'|'assistant', content}]，最多 6 条
  let lastPending = null;    // 上一轮"还差什么"：显式回传给后端，比只靠 history 准
  let relistenGuard = 0;     // 连续追问次数上限，防死循环
  const MAX_RELISTEN = 3;

  const ICON = { 'is-live': 'fa-microphone', 'is-ok': 'fa-check-circle', 'is-err': 'fa-exclamation-circle' };
  let hideTimer = null;
  let wantVisible = false;      // 用状态判断，而不是查 class：
                                // 否则淡入的 rAF 回调晚于"关闭"执行时会把 is-show 加回来，浮层卡住不消失

  /* 浮层：淡入 → 几秒后自动淡出；识别中(live) 不自动消失 */
  function show(msg, cls) {
    const bar = $('voiceBar'), text = $('voiceText'), icon = $('voiceIcon');
    if (!bar || !text) return;
    clearTimeout(hideTimer);

    if (!msg) {
      wantVisible = false;
      bar.classList.remove('is-show');
      hideTimer = setTimeout(() => { if (!wantVisible) bar.hidden = true; }, 200);
      return;
    }
    wantVisible = true;
    const shown = bar.classList.contains('is-show');
    bar.hidden = false;
    bar.className = 'voice-bar' + (cls ? ' ' + cls : '');
    if (shown) bar.classList.add('is-show');          // 已在显示：保持住，别每次 interim 都重放淡入
    if (icon) icon.className = 'fa ' + (ICON[cls] || 'fa-microphone');
    text.textContent = msg;
    if (!shown) {
      const raf = window.requestAnimationFrame || (f => setTimeout(f, 0));
      raf(() => { if (wantVisible) bar.classList.add('is-show'); });   // 期间被关掉就别再加回来
    }

    if (cls !== 'is-live') {                          // 结果类：8 秒后自己收掉
      hideTimer = setTimeout(() => show(''), 8000);
    }
  }
  /* 朗读用浏览器自带 TTS（免费、不需要后端）。Chrome 的音色列表是异步加载的，
     所以先挑一次 + 监听 voiceschanged 再挑，尽量拿到中文音色。 */
  /* 朗读参数：音色 / 语速 / 音调 / 音量 —— 存本机 localStorage，可在面板里调 */
  const VCFG_KEY = 'panelVoice';
  const vcfg = Object.assign({ name: '', rate: 1.05, pitch: 1, volume: 1 },
    (function () { try { return JSON.parse(localStorage.getItem(VCFG_KEY) || '{}') || {}; } catch (e) { return {}; } })());
  function saveVcfg() { try { localStorage.setItem(VCFG_KEY, JSON.stringify(vcfg)); } catch (e) {} }

  let zhVoice = null;
  function allVoices() {
    try { return (window.speechSynthesis && speechSynthesis.getVoices()) || []; } catch (e) { return []; }
  }
  function zhVoices() {
    const vs = allVoices();
    const zh = vs.filter(v => /zh/i.test(v.lang));
    return zh.length ? zh : vs;      // 系统没有中文就退回全部，让用户自己挑
  }
  function pickVoice() {
    const vs = zhVoices();
    zhVoice = vs.find(x => x.name === vcfg.name)
           || vs.find(x => /zh[-_]?(CN|Hans)/i.test(x.lang) || /Chinese|中文|普通话/i.test(x.name))
           || vs[0] || null;
    if (zhVoice && !vcfg.name) { vcfg.name = zhVoice.name; saveVcfg(); }
    fillVoiceCfg();
  }

  /* 播报；onEnd 在"念完"后回调（没有 TTS 时用时长估算兜底），只触发一次 */
  function speak(msg, onEnd) {
    let done = false;
    const fire = () => { if (!done) { done = true; if (onEnd) onEnd(); } };
    if (!msg || !window.speechSynthesis) {
      if (onEnd) setTimeout(fire, Math.min(4000, String(msg || '').length * 120 + 400));
      return;
    }
    try {
      speechSynthesis.cancel();
      const u = new SpeechSynthesisUtterance(String(msg));
      u.lang = 'zh-CN';
      u.rate = Number(vcfg.rate) || 1.05;
      u.pitch = Number(vcfg.pitch) >= 0 ? Number(vcfg.pitch) : 1;
      u.volume = Number(vcfg.volume) >= 0 ? Number(vcfg.volume) : 1;
      if (zhVoice) u.voice = zhVoice;
      u.onend = fire;
      u.onerror = fire;
      speechSynthesis.speak(u);
      setTimeout(fire, Math.min(8000, String(msg).length * 220 + 900));   // 兜底：onend 没回来也能接上
    } catch (e) { fire(); }
  }

  function remember(text, intent) {
    context.push({ role: 'user', content: text });
    context.push({ role: 'assistant', content: JSON.stringify({
      action: intent.action, target: intent.target, targets: intent.targets,
      value: intent.value, reply: intent.reply }) });
    if (context.length > 6) context = context.slice(-6);
  }

  /* 需要补全槽位（例如只说了"开灯"）：念完问题后自动接着听 */
  function askAgain(reply) {
    const text = reply || '没听懂，再说一次';
    show(text, 'is-err');
    speak(text, () => {
      if (!SR) return;
      if (relistenGuard >= MAX_RELISTEN) {
        relistenGuard = 0;
        show('连着几轮没听懂，点麦克风可以重来', 'is-err');
        return;
      }
      relistenGuard++;
      show('在听…请回答上一句', 'is-live');
      start();
    });
  }
  /* ---------- 语音设置面板 ---------- */
  function fillVoiceCfg() {
    const sel = $('voiceSel'), hint = $('voiceHint');
    if (!sel) return;
    const vs = zhVoices();
    const cur = (zhVoice && zhVoice.name) || vcfg.name || '';
    if (vs.length) {
      /* 系统音色名很长（"Eddy (中文（中国大陆）)"），全塞进下拉框会撑破面板：
         显示成「名字 · 语言」，完整名字放 title 里，其余交给 CSS 省略号 */
      sel.innerHTML = vs.map(v => {
        const full = v.name + '（' + v.lang + '）';
        const label = String(v.name || '').split(/[(（]/)[0].trim() || v.name;
        const lang = ({ 'zh-CN': '普通话', 'zh-Hans': '普通话', 'zh-TW': '台湾', 'zh-HK': '粤语' })[v.lang] || v.lang;
        return '<option value="' + v.name.replace(/"/g, '&quot;') + '"' + (v.name === cur ? ' selected' : '') +
               ' title="' + full.replace(/"/g, '&quot;') + '">' + label + ' · ' + lang + '</option>';
      }).join('');
    } else {
      sel.innerHTML = '<option value="">（系统还没有可用音色）</option>';
    }
    const r = $('voiceRate'), p = $('voicePitch'), vo = $('voiceVol');
    if (r) { r.value = vcfg.rate; $('voiceRateVal').textContent = Number(vcfg.rate).toFixed(2); }
    if (p) { p.value = vcfg.pitch; $('voicePitchVal').textContent = Number(vcfg.pitch).toFixed(2); }
    if (vo) { vo.value = vcfg.volume; $('voiceVolVal').textContent = Number(vcfg.volume).toFixed(2); }
    if (hint) {
      hint.textContent = vs.length
        ? '音色来自系统；要更多（如「婷婷/美佳」）去系统设置里下载中文语音'
        : '系统没装中文语音：macOS 系统设置 → 辅助功能 → 朗读内容 → 系统声音 → 管理声音';
    }
  }

  function bindVoiceCfg() {
    const sel = $('voiceSel');
    if (!sel || sel.__bound) return;
    sel.__bound = true;
    const onRange = (el, key) => {
      if (!el) return;
      el.addEventListener('input', () => {
        vcfg[key] = Number(el.value);
        const span = $('voice' + key[0].toUpperCase() + key.slice(1) + 'Val');
        if (span) span.textContent = Number(el.value).toFixed(2);
        saveVcfg();
      });
    };
    sel.addEventListener('change', () => {
      vcfg.name = sel.value;
      saveVcfg();
      pickVoice();
      speak('音色已切换');
    });
    onRange($('voiceRate'), 'rate');
    onRange($('voicePitch'), 'pitch');
    onRange($('voiceVol'), 'volume');
    const test = $('voiceTest');
    if (test) test.addEventListener('click', () => speak('已打开 风扇，温度 26.7 度'));
  }

  function toggleVoiceCfg() {
    const box = $('voiceCfg'), btn = $('voiceCfgBtn');
    if (!box) return;
    const open = !box.hidden;
    if (open) {
      box.classList.remove('is-show');
      setTimeout(() => { if (!box.classList.contains('is-show')) box.hidden = true; }, 170);
    } else {
      fillVoiceCfg();
      bindVoiceCfg();
      box.hidden = false;
      const raf = window.requestAnimationFrame || (f => setTimeout(f, 0));
      raf(() => box.classList.add('is-show'));
    }
    if (btn) btn.classList.toggle('is-open', !open);
  }

  function setLive(on) {
    listening = on;
    const btn = $('voiceBtn'), label = $('voiceLabel');
    if (btn) btn.classList.toggle('mic-live', on);
    if (label) label.textContent = on ? '在听' : '语音';
  }

  /* 读一个属性：返回展示值 + 适合朗读的说法（℃ 念"度"，%RH 念"%"） */
  const SPEAK_UNIT = { '℃': '度', '°C': '度', '%RH': '%', '%': '%' };
  function readValue(id) {
    const panel = window.__panel;
    const card = (panel.cards || []).find(c => c.id === id) || {};
    const raw = panel.get(id);
    if (raw === null || raw === undefined || raw === '') return { name: card.name || id, text: '暂无数据', speak: (card.name || id) + '暂无数据' };
    const n = Number(raw);
    const text = isNaN(n) ? String(raw) : (card.unit === '℃' ? n.toFixed(1) : String(raw));
    const unit = card.unit || '';
    return { name: card.name || id, text: text + unit, speak: (card.name || id) + text + (SPEAK_UNIT[unit] || unit) };
  }

  /* 一句话里做多件事（"太热了" = 开风扇 + 报温度）：按顺序执行，最后合成一句播报 */
  async function runSteps(steps) {
    const panel = window.__panel;
    const parts = [];
    for (const st of (steps || [])) {
      if (st.action === 'toggle' || st.action === 'toggle_many') {
        const keys = st.action === 'toggle' ? [st.target] : (st.targets || []);
        if (!keys.filter(Boolean).length) continue;
        if (!panel.isOnline()) { parts.push('设备离线'); continue; }
        const r = await panel.setMany(keys, st.value);
        const names = (r.names || keys.map(k => (panel.controls.find(c => c.key === k) || {}).name || k)).join('、');
        parts.push(r.ok ? ((st.value ? '已打开 ' : '已关闭 ') + names) : ('下发失败：' + names));
      } else if (st.action === 'query') {
        await panel.refresh();                       // 先拉最新值再播报
        const rv = readValue(st.target);
        parts.push(rv.text === '暂无数据' ? (rv.name + ' 暂无数据') : rv.speak);
      } else if (st.action === 'refresh') {
        panel.refresh();
      }
    }
    return parts;
  }

  async function execute(intent) {
    const panel = window.__panel;
    if (!panel) return show('面板还没就绪', 'is-err');
    const { action, target, value, reply } = intent;

    switch (action) {
      case 'toggle': {
        if (target === null) { lastPending = intent.pending || null; askAgain(reply || '要控制哪一路？'); return; }
        relistenGuard = 0;
        if (!panel.isOnline()) { show('设备离线，不能下发', 'is-err'); speak('设备离线'); return; }
        const ok = await panel.set(target, value);
        const name = (panel.controls.find(c => c.key === target) || {}).name || target;
        const msg = ok ? ((value ? '已打开 ' : '已关闭 ') + name) : ('下发失败：' + name);
        show(msg, ok ? 'is-ok' : 'is-err');
        speak(msg);
        return;
      }
      case 'toggle_many': {
        const keys = intent.targets || [];
        if (!keys.length) { lastPending = intent.pending || null; askAgain(reply || '要动哪几个？'); return; }
        relistenGuard = 0;
        if (!panel.isOnline()) { show('设备离线，不能下发', 'is-err'); speak('设备离线'); return; }
        const r = await panel.setMany(keys, value);
        const names = (r.names || keys.map(k => (panel.controls.find(c => c.key === k) || {}).name || k)).join('、');
        const msg = r.ok
          ? (r.changed ? ((value ? '已打开 ' : '已关闭 ') + names) : ('这些已经是' + (value ? '打开' : '关闭') + '状态了'))
          : ('下发失败：' + names);
        show(msg, r.ok ? 'is-ok' : 'is-err');
        speak(msg);
        return;
      }
      case 'query': {
        if (target === null) { lastPending = intent.pending || null; askAgain(reply || '要查哪一项？'); return; }
        relistenGuard = 0;
        await panel.refresh();
        const rv = readValue(target);
        const msg = rv.name + ' ' + rv.text;     // 自己拼，别让模型的口径掺进来
        show(msg, 'is-ok');
        speak(msg);
        return;
      }
      case 'steps': {
        if (!panel.isOnline()) { show('设备离线，做不了', 'is-err'); speak('设备离线'); return; }
        const parts = await runSteps(intent.steps);
        const msg = parts.join('，') || (reply || '已执行');
        relistenGuard = 0;
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
        /* 两种信号任一命中就接着听：
           ① 后端给了 pending（结构化地告诉我们差什么）
           ② 回复是个问句（模型在等用户回答，例如「要开哪一路？」）
           纯粹闲聊（无问号、无 pending）则不追问，避免自说自话 */
        if (intent.pending || /[？?]\s*$/.test((reply || '').trim())) {
          askAgain(reply || '再说一次？');
          return;
        }
        relistenGuard = 0;
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
      const intent = await PanelAPI.chat(transcript, panel ? panel.controls : [],
                                         panel ? panel.cards : [], context, lastPending);
      remember(transcript, intent);
      lastPending = intent.pending || null;
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

  /* 快捷键：⌘/Ctrl+K 开始或停止听，Esc 立刻停（在输入框里打字时不触发） */
  document.addEventListener('keydown', (e) => {
    const tag = (e.target && e.target.tagName || '').toLowerCase();
    if (tag === 'input' || tag === 'textarea' || tag === 'select' || (e.target && e.target.isContentEditable)) return;
    if (e.key === 'Escape') {
      if (listening) { try { rec.stop(); } catch (err) {} show(''); }
      return;
    }
    if ((e.key === 'k' || e.key === 'K') && (e.metaKey || e.ctrlKey)) {
      e.preventDefault();
      if (listening) { try { rec.stop(); } catch (err) {} show(''); }
      else { relistenGuard = 0; start(); }
    }
  });

  /* 事件委托：Vue 重建按钮后依然生效 */
  document.addEventListener('click', (e) => {
    if (e.target.closest && e.target.closest('#voiceCfgBtn')) { toggleVoiceCfg(); return; }
    const btn = e.target.closest && e.target.closest('#voiceBtn');
    if (!btn) return;
    if (listening) { try { rec.stop(); } catch (err) {} show(''); return; }
    relistenGuard = 0;                  // 手动点 = 重新开始
    start();
  });

  pickVoice();
  if (window.speechSynthesis) {
    try { speechSynthesis.onvoiceschanged = pickVoice; } catch (e) {}   // Chrome 音色列表是异步加载的
  }

  /* 不支持时把按钮置灰（同样等 Vue 渲染完再动） */
  if (!SR) {
    const arm = () => { const b = $('voiceBtn'); if (b) { b.disabled = true; b.title = '这个浏览器不支持语音识别'; } };
    if (document.readyState === 'complete') setTimeout(arm, 0); else window.addEventListener('load', arm);
  }
})();
