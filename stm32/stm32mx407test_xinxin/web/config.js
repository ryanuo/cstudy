/* 面板配置 + 同源 API 封装
 *
 * 设备 token / DashScope key 全在后端（Vercel 环境变量），前端只持有：
 *   1) 同源端点 /api/onenet（后端做 op 白名单）
 *   2) 面板口令（localStorage，首次打开时问一次）
 */
window.PANEL_CONFIG = {
  productId: 'WW0f6843I6',
  deviceName: 'humi_temp',
  apiBase: '/api/onenet',
  chatUrl: '/api/chat',
  ttsUrl: '/api/tts'
  /* 口令不在这里：统一由 PanelAPI.key() 现取（localStorage） */
};

window.PanelAPI = {
  key() { return localStorage.getItem('panelKey') || ''; },
  setKey(k) { localStorage.setItem('panelKey', k || ''); },
  clearKey() { localStorage.removeItem('panelKey'); },

  /* 统一入口：op = deviceDetail|getProperty|getHistory|setProperty|callService */
  async post(op, params, body) {
    const res = await fetch(window.PANEL_CONFIG.apiBase, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'X-Panel-Key': this.key()
      },
      body: JSON.stringify({ op, params: params || null, body: body || null })
    });
    if (res.status === 401) {
      this.clearKey();
      const err = new Error('口令错误或已失效');
      err.needKey = true;
      throw err;
    }
    if (res.status === 429) throw new Error('请求太快，稍后再试');
    const data = await res.json().catch(() => null);
    if (data === null) throw new Error('后端返回的不是 JSON（' + res.status + '）');
    return data;
  },

  /* 口令失效/没口令时，在页面上开一个输入条（**不要用 window.prompt**：
     Electron/内嵌 webview 里没有 prompt，会直接抛错，表现成"点了没反应"） */
  ensureKey() {
    if (this.key()) return true;
    this.showKeyGate();
    return false;
  },

  showKeyGate() {
    if (document.getElementById('keyGate')) return;
    const box = document.createElement('div');
    box.id = 'keyGate';
    box.className = 'key-gate';
    box.innerHTML = '<span class="key-gate-tip">面板口令</span>'
      + '<input id="keyGateInput" type="password" placeholder="输入口令（存本机浏览器）" />'
      + '<button id="keyGateOk" type="button" class="btn btn-outline btn-sm">保存</button>';
    document.body.appendChild(box);
    const input = box.querySelector('#keyGateInput');
    const save = () => {
      const v = (input.value || '').trim();
      if (!v) { input.focus(); return; }
      this.setKey(v);
      location.reload();          // 重新加载，让面板/语音都带上新口令
    };
    box.querySelector('#keyGateOk').addEventListener('click', save);
    input.addEventListener('keydown', e => { if (e.key === 'Enter') save(); });
    input.focus();
  },

  /* 火山朗读：POST 一句文本 → mp3 Blob。音色在后端 env 里固定，这里只传文本和滑条。
     失败会把后端的原话抛出来（调用方据此决定回退还是提示）。 */
  async tts(text, rate, pitch, volume) {
    const res = await fetch(window.PANEL_CONFIG.ttsUrl, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json', 'X-Panel-Key': this.key() },
      body: JSON.stringify({ text, rate, pitch, volume })
    });
    if (res.status === 401) {
      this.clearKey();
      const err = new Error('口令错误或已失效');
      err.needKey = true;
      throw err;
    }
    if (!res.ok) {
      const d = await res.json().catch(() => null);
      throw new Error((d && d.msg) || ('合成失败（' + res.status + '）'));
    }
    return await res.blob();
  },

  async chat(text, controls, cards, history, pending) {
    const res = await fetch(window.PANEL_CONFIG.chatUrl, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json', 'X-Panel-Key': this.key() },
      body: JSON.stringify({ text, controls, cards, history: history || [], pending: pending || null })
    });
    if (res.status === 401) {
      this.clearKey();
      const err = new Error('口令错误或已失效');
      err.needKey = true;
      throw err;
    }
    if (res.status === 429) throw new Error('说太快了，稍等一下');
    const data = await res.json().catch(() => null);
    if (!data || !data.intent) throw new Error((data && data.msg) || '意图解析失败');
    return data.intent;
  }
};
