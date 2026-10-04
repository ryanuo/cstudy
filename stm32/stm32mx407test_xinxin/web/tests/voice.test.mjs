/* voice.js 的接线测试：不依赖浏览器，用最小 DOM 桩跑 config.js + voice.js。
 * 跑法：node --test web/tests/voice.test.mjs
 */
import { test } from 'node:test';
import assert from 'node:assert';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const WEB = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');

/* ---------------- 最小 DOM / 环境桩 ---------------- */
class El {
  constructor(id) {
    this.id = id; this.tagName = 'BUTTON'; this.textContent = '';
    this.hidden = false; this.disabled = false; this.title = '';
    this._cls = new Set(); this.listeners = {};
  }
  addEventListener(t, f) { (this.listeners[t] ||= []).push(f); }
  fire(t, ev) { (this.listeners[t] || []).forEach(f => f(ev)); }
  get classList() {
    const s = this._cls;
    return { toggle: (c, on) => { on ? s.add(c) : s.delete(c); }, contains: c => s.has(c), add: c => s.add(c), remove: c => s.delete(c) };
  }
  closest(sel) { return sel === '#' + this.id ? this : null; }
}

function makeEnv({ withSR = false } = {}) {
  const els = {
    voiceBtn: new El('voiceBtn'),
    voiceLabel: new El('voiceLabel'),
    voiceBar: new El('voiceBar'),
    voiceText: new El('voiceText'),
    voiceCfgBtn: new El('voiceCfgBtn'),
    voiceCfg: new El('voiceCfg'),
    voiceSel: new El('voiceSel'),
    voiceRate: new El('voiceRate'),
    voicePitch: new El('voicePitch'),
    voiceVol: new El('voiceVol'),
    voiceRateVal: new El('voiceRateVal'),
    voicePitchVal: new El('voicePitchVal'),
    voiceVolVal: new El('voiceVolVal'),
    voiceTest: new El('voiceTest'),
    voiceHint: new El('voiceHint')
  };
  els.voiceCfg.hidden = true;
  els.voiceBar.hidden = true;
  const doc = {
    listeners: {},
    addEventListener(t, f) { (this.listeners[t] ||= []).push(f); },
    getElementById: id => els[id] || null,
    querySelectorAll: () => [],
    readyState: 'complete'
  };
  const store = {};
  const calls = { fetch: [], set: [], speak: [], utterances: [] };

  globalThis.window = globalThis;
  globalThis.document = doc;
  globalThis.localStorage = {
    getItem: k => (k in store ? store[k] : null),
    setItem: (k, v) => { store[k] = String(v); },
    removeItem: k => { delete store[k]; }
  };
  globalThis.fetch = async (url, opts) => { calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  globalThis.speechSynthesis = {
    cancel() {},
    speak(u) { calls.speak.push(u.text); calls.utterances.push(u); if (u.onend) setTimeout(u.onend, 0); },
    getVoices() { return [{ name: 'Ting-Ting', lang: 'zh-CN' }, { name: 'Sin-ji', lang: 'zh-HK' }, { name: 'Alex', lang: 'en-US' }]; },
    onvoiceschanged: null
  };
  globalThis.location = { reload() { calls.reloaded = (calls.reloaded || 0) + 1; } };
  // 故意**不**提供 window.prompt：Electron 里就没有它，代码不该依赖
  const body = { children: [], appendChild(n) { this.children.push(n); globalThis.__gate = n; } };
  doc.body = body;
  doc.createElement = () => {
    const el = new El('tmp');
    el.innerHTML = '';
    el.querySelector = () => ({ addEventListener() {}, focus() {}, value: 'K' });
    el.appendChild = () => {};
    return el;
  };
  globalThis.SpeechSynthesisUtterance = class { constructor(t) { this.text = t; this.rate = 0; this.pitch = 0; this.volume = 0; this.voice = null; this.lang = ''; } };


  if (withSR) {
    class FakeSR {
      start() { this.started = true; env.lastSR = this; env.srStarts++; if (this.onstart) this.onstart(); }
      stop() { if (this.onend) this.onend(); }
    }
    globalThis.SpeechRecognition = FakeSR;
  } else {
    delete globalThis.SpeechRecognition;
    delete globalThis.webkitSpeechRecognition;
  }

  const env = {
    els, calls, doc, store,
    lastSR: null, srStarts: 0,
    fetchReply: async () => ({ ok: true, status: 200, json: async () => ({ intent: { action: 'unknown', reply: 'x' } }) })
  };
  return env;
}

function loadScripts() {
  for (const f of ['config.js', 'app.js.stub', 'voice.js']) {
    if (f === 'app.js.stub') continue;              // 面板桥由测试自己塞
    const code = fs.readFileSync(path.join(WEB, f), 'utf8');
    new Function(code).call(globalThis);            // 普通脚本语义
  }
}

/* ---------------- 测试 ---------------- */

test('不支持语音的浏览器：点按钮 → 状态条给明确提示，且不炸', async () => {
  const env = makeEnv({ withSR: false });
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));

  assert.equal(env.els.voiceBar.hidden, false, '状态条应显示出来');
  assert.match(env.els.voiceText.textContent, /不支持语音识别/);
});

test('支持语音：点击→识别→后端意图→执行 toggle→播报', async () => {
  const env = makeEnv({ withSR: true });
  env.fetchReply = async (url) => {
    if (url === '/api/chat') {
      return { ok: true, status: 200, json: async () => ({ intent: { action: 'toggle', target: 'led1', value: true, reply: '已打开灯 1' } }) };
    }
    return { ok: true, status: 200, json: async () => ({}) };
  };
  globalThis.fetch = async (url, opts) => { env.calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  globalThis.__panel = {
    controls: [{ key: 'led1', name: '灯 1' }],
    cards: [{ id: 'temperature', name: '温度', unit: '℃' }],
    isOnline: () => true,
    set: async (k, v) => { env.calls.set.push(k + ':' + v); return true; },
    get: () => 27.5,
    refresh: async () => {},
    armReboot: () => true
  };
  env.store.panelKey = 'test-key';          // 已有口令，直接走识别
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  assert.ok(env.lastSR && env.lastSR.started, '识别应已启动');

  // 模拟一句最终识别结果
  const results = [[{ transcript: '打开灯 1' }]];
  results[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results });
  await new Promise(r => setTimeout(r, 30));

  const chat = env.calls.fetch.find(c => c.url === '/api/chat');
  assert.ok(chat, '应该调了 /api/chat');
  assert.equal(JSON.parse(chat.opts.body).text, '打开灯 1');
  assert.deepEqual(env.calls.set, ['led1:true'], '应下发 led1=true');
  assert.match(env.els.voiceText.textContent, /已打开灯 1|已打开/);
  assert.ok(env.calls.speak.some(s => /已打开/.test(s)), '应该语音播报');
});

test('重启意图不会自动执行，只把面板按钮推到确认态', async () => {
  const env = makeEnv({ withSR: true });
  env.fetchReply = async () => ({ ok: true, status: 200, json: async () => ({ intent: { action: 'reboot', target: null, value: null, reply: '重启' } }) });
  globalThis.fetch = async (url, opts) => { env.calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  let armed = 0;
  globalThis.__panel = {
    controls: [], cards: [], isOnline: () => true,
    set: async () => { throw new Error('重启不应该走 set'); },
    get: () => null, refresh: async () => {}, armReboot: () => { armed++; return true; }
  };
  env.store.panelKey = 'test-key';
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  const results = [[{ transcript: '重启设备' }]];
  results[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results });
  await new Promise(r => setTimeout(r, 30));

  assert.equal(armed, 1, '应该调用 armReboot');
  assert.match(env.els.voiceText.textContent, /确认/);
  assert.ok(env.calls.speak.some(s => /点两下|确认/.test(s)));
});

test('没有口令时：点按钮 → 弹出页面内输入条（不用 window.prompt）', async () => {
  const env = makeEnv({ withSR: true });
  globalThis.__panel = { controls: [], cards: [], isOnline: () => true, set: async () => true, get: () => null, refresh: async () => {}, armReboot: () => false };
  loadScripts();
  assert.equal(typeof globalThis.__gate === 'undefined' || !globalThis.__gate, true);
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  assert.ok(globalThis.__gate, '应该创建口令输入条');
  assert.match(env.els.voiceText.textContent, /口令/);
});

test('缺目标时不下发、播报追问、并自动接着听（真实后端形状）', async () => {
  const env = makeEnv({ withSR: true });
  env.fetchReply = async () => ({ ok: true, status: 200, json: async () => ({
    intent: { action: 'unknown', target: null, targets: null, value: null,
              reply: '要开哪一路？', pending: { action: 'toggle', value: true } } }) });
  globalThis.fetch = async (url, opts) => { env.calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  const set = [];
  globalThis.__panel = {
    controls: [{ key: 'led1', name: '灯 1' }], cards: [], isOnline: () => true,
    set: async (k, v) => { set.push(k + ':' + v); return true; },
    setMany: async () => ({ ok: true, changed: 0 }), get: () => null,
    refresh: async () => {}, armReboot: () => false
  };
  env.store.panelKey = 'test-key';
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  const results = [[{ transcript: '开灯' }]];
  results[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results });
  await new Promise(r => setTimeout(r, 80));

  assert.deepEqual(set, [], '不该下发任何东西');
  assert.ok(env.calls.speak.some(x => /要开哪一路/.test(x)), '应该把追问念出来');
  assert.equal(env.srStarts, 2, '念完应自动重新开麦（第二次 start）');
});


test('toggle_many：一条下发多个目标（走 panel.setMany）', async () => {
  const env = makeEnv({ withSR: true });
  env.fetchReply = async () => ({ ok: true, status: 200,
    json: async () => ({ intent: { action: 'toggle_many', target: null, targets: ['led1', 'led2', 'led3'], value: false, reply: '关闭三路灯' } }) });
  globalThis.fetch = async (url, opts) => { env.calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  const calls = [];
  globalThis.__panel = {
    controls: [{ key: 'led1', name: '灯 1' }, { key: 'led2', name: '灯 2' }, { key: 'led3', name: '灯 3' }],
    cards: [], isOnline: () => true,
    set: async () => { throw new Error('多目标不该走单目标 set'); },
    setMany: async (keys, value) => { calls.push(keys.join('+') + ':' + value); return { ok: true, changed: keys.length, names: ['灯 1', '灯 2', '灯 3'] }; },
    get: () => null, refresh: async () => {}, armReboot: () => false
  };
  env.store.panelKey = 'test-key';
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  const results = [[{ transcript: '把灯都关了吧' }]];
  results[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results });
  await new Promise(r => setTimeout(r, 30));

  assert.deepEqual(calls, ['led1+led2+led3:false'], '应一条下发三个目标');
  assert.match(env.els.voiceText.textContent, /已关闭|关闭/);
});


test('追问"要开哪一路"后自动重新开麦，且第二轮带上上下文', async () => {
  const env = makeEnv({ withSR: true });
  let round = 0;
  env.fetchReply = async () => {
    round++;
    const intent = round === 1
      ? { action: 'unknown', target: null, targets: null, value: null, reply: '要开哪一路？', pending: { action: 'toggle', value: true } }
      : { action: 'toggle', target: 'led1', targets: null, value: true, reply: '已开启' };
    return { ok: true, status: 200, json: async () => ({ intent }) };
  };
  globalThis.fetch = async (url, opts) => { env.calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  const set = [];
  globalThis.__panel = {
    controls: [{ key: 'led1', name: '灯 1' }], cards: [], isOnline: () => true,
    set: async (k, v) => { set.push(k + ':' + v); return true; },
    setMany: async () => ({ ok: true, changed: 0 }), get: () => null,
    refresh: async () => {}, armReboot: () => false
  };
  env.store.panelKey = 'test-key';
  loadScripts();

  // 第一轮："开灯" → 追问
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  const r1 = [[{ transcript: '开灯' }]]; r1[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results: r1 });
  await new Promise(r => setTimeout(r, 80));

  assert.ok(env.calls.speak.some(x => /要开哪一路/.test(x)), '应该念出追问');
  assert.equal(env.srStarts, 2, '应该自动重新开麦');
  assert.match(env.els.voiceText.textContent, /在听/, '重新开麦后状态条显示"在听…"');
  assert.equal(typeof env.lastSR.onresult, 'function');

  // 第二轮："灯1" → 补全并执行
  const r2 = [[{ transcript: '灯1' }]]; r2[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results: r2 });
  await new Promise(r => setTimeout(r, 80));

  assert.deepEqual(set, ['led1:true'], '第二轮应补全成 led1=true');
  const second = JSON.parse(env.calls.fetch[1].opts.body);
  assert.equal(second.text, '灯1');
  assert.ok(Array.isArray(second.history) && second.history.length >= 2, '第二轮必须带上下文');
  assert.equal(second.history[0].content, '开灯');
});

test('闲聊（无 pending）不会自动重新开麦', async () => {
  const env = makeEnv({ withSR: true });
  env.fetchReply = async () => ({ ok: true, status: 200,
    json: async () => ({ intent: { action: 'chat', target: null, targets: null, value: null, reply: '你好呀' } }) });
  globalThis.fetch = async (url, opts) => { env.calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  globalThis.__panel = { controls: [], cards: [], isOnline: () => true, set: async () => true,
    setMany: async () => ({ ok: true }), get: () => null, refresh: async () => {}, armReboot: () => false };
  env.store.panelKey = 'test-key';
  loadScripts();
  let starts = 0;
  const origStart = Object.getPrototypeOf; // 不折腾原型，直接数 onstart 次数
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  const r = [[{ transcript: '今天天气怎么样' }]]; r[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results: r });
  await new Promise(res => setTimeout(res, 120));
  assert.equal(env.calls.fetch.length, 1, '闲聊只应发一次请求，不该自动继续听');
});


test('模型只回问句（无 pending）也会自动接着听，并把 pending 回传', async () => {
  const env = makeEnv({ withSR: true });
  let round = 0;
  env.fetchReply = async () => {
    round++;
    const intent = round === 1
      ? { action: 'unknown', target: null, targets: null, value: null, reply: '要开哪一路？' }   // 无 pending，但是问句
      : { action: 'toggle', target: 'led1', targets: null, value: true, reply: '已开启' };
    return { ok: true, status: 200, json: async () => ({ intent }) };
  };
  globalThis.fetch = async (url, opts) => { env.calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  const set = [];
  globalThis.__panel = { controls: [{ key: 'led1', name: '灯 1' }], cards: [], isOnline: () => true,
    set: async (k, v) => { set.push(k + ':' + v); return true; }, setMany: async () => ({ ok: true }),
    get: () => null, refresh: async () => {}, armReboot: () => false };
  env.store.panelKey = 'test-key';
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  const r1 = [[{ transcript: '开灯' }]]; r1[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results: r1 });
  await new Promise(r => setTimeout(r, 80));
  assert.equal(env.srStarts, 2, '问句 → 自动重新开麦');

  const r2 = [[{ transcript: '灯1' }]]; r2[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results: r2 });
  await new Promise(r => setTimeout(r, 80));
  assert.deepEqual(set, ['led1:true']);
  const body = JSON.parse(env.calls.fetch[1].opts.body);
  assert.ok('pending' in body, '后续请求要带 pending 字段（可能为 null）');
  assert.equal(body.history.length >= 2, true);
});

test('一次动作做完就不再自动听（避免自说自话）', async () => {
  const env = makeEnv({ withSR: true });
  env.fetchReply = async () => ({ ok: true, status: 200,
    json: async () => ({ intent: { action: 'toggle', target: 'led1', targets: null, value: true, reply: '已开启' } }) });
  globalThis.fetch = async (url, opts) => { env.calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  globalThis.__panel = { controls: [{ key: 'led1', name: '灯 1' }], cards: [], isOnline: () => true,
    set: async () => true, setMany: async () => ({ ok: true }), get: () => null,
    refresh: async () => {}, armReboot: () => false };
  env.store.panelKey = 'test-key';
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  const r = [[{ transcript: '打开灯1' }]]; r[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results: r });
  await new Promise(res => setTimeout(res, 120));
  assert.equal(env.srStarts, 1, '执行成功不该再自动开麦');
});


test('steps：开风扇 + 报温度，合成一句播报（含度数）', async () => {
  const env = makeEnv({ withSR: true });
  env.fetchReply = async () => ({ ok: true, status: 200, json: async () => ({ intent: {
    action: 'steps', target: null, targets: null, value: null, reply: '已打开风扇',
    steps: [{ action: 'toggle', target: 'fan', value: true },
            { action: 'query', target: 'temperature' }] } }) });
  globalThis.fetch = async (url, opts) => { env.calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  const sets = [];
  let refreshed = 0;
  globalThis.__panel = {
    controls: [{ key: 'fan', name: '风扇' }],
    cards: [{ id: 'temperature', name: '温度', unit: '℃' }],
    isOnline: () => true,
    set: async (k, v) => { sets.push(k + ':' + v); return true; },
    setMany: async (keys, value) => { keys.forEach(k => sets.push(k + ':' + value)); return { ok: true, changed: keys.length, names: ['风扇'] }; },
    get: (id) => (id === 'temperature' ? 26.7 : null),
    refresh: async () => { refreshed++; },
    armReboot: () => false
  };
  env.store.panelKey = 'test-key';
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  const r = [[{ transcript: '太热了' }]]; r[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results: r });
  await new Promise(res => setTimeout(res, 60));

  assert.deepEqual(sets, ['fan:true'], '应该开风扇');
  assert.equal(refreshed, 1, '查询步要先刷新再读');
  const spoken = env.calls.speak.join(' | ');
  assert.match(spoken, /已打开 风扇/, '播报要包含动作');
  assert.match(spoken, /温度/, '播报要包含温度');
  assert.match(spoken, /26\.7/, '播报要带具体数值');
  assert.match(spoken, /度/, '℃ 要念成"度"');
  assert.equal(env.srStarts, 1, '场景执行完不自动续听');
});


/* ---------------- 语音设置（音色 / 语速 / 音调 / 音量） ---------------- */

test('保存过的朗读参数会应用到播报（音色 + 语速/音调/音量）', async () => {
  const env = makeEnv({ withSR: true });
  env.store.panelVoice = JSON.stringify({ name: 'Sin-ji', rate: 1.6, pitch: 0.7, volume: 0.4 });
  env.fetchReply = async () => ({ ok: true, status: 200,
    json: async () => ({ intent: { action: 'chat', reply: '你好呀' } }) });
  globalThis.fetch = async (url, opts) => { env.calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  globalThis.__panel = { controls: [], cards: [], isOnline: () => true, set: async () => true,
    setMany: async () => ({ ok: true }), get: () => null, refresh: async () => {}, armReboot: () => false };
  env.store.panelKey = 'test-key';
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  const r = [[{ transcript: '你好' }]]; r[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results: r });
  await new Promise(res => setTimeout(res, 40));

  const u = env.calls.utterances[env.calls.utterances.length - 1];
  assert.equal(u.rate, 1.6, '语速应来自本地配置');
  assert.equal(u.pitch, 0.7, '音调应来自本地配置');
  assert.equal(u.volume, 0.4, '音量应来自本地配置');
  assert.equal(u.voice && u.voice.name, 'Sin-ji', '应按名字选中保存的音色');
  assert.equal(u.lang, 'zh-CN');
});

test('齿轮按钮打开设置面板并列出系统音色', async () => {
  const env = makeEnv();
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceCfgBtn }));
  assert.equal(env.els.voiceCfg.hidden, false, '面板应打开');
  assert.match(env.els.voiceSel.innerHTML, /Ting-Ting/, '应列出声色');
  assert.match(env.els.voiceSel.innerHTML, /selected/, '当前音色应被选中');
  assert.ok(/音色来自系统|系统没装中文语音/.test(env.els.voiceHint.textContent), '应有提示文案');
});

test('拖动语速滑块会存入 localStorage，并影响后续播报', async () => {
  const env = makeEnv();
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceCfgBtn }));   // 打开 → 绑定
  env.els.voiceRate.value = '1.5';
  env.els.voiceRate.fire('input', {});
  assert.equal(JSON.parse(env.store.panelVoice).rate, 1.5, '应写回本地配置');
  assert.equal(env.els.voiceRateVal.textContent, '1.50', '数值要显示出来');
});


/* ---------------- 快捷键 + 音色标签 ---------------- */

function fireKey(env, ev) { env.doc.listeners.keydown.forEach(f => f(ev)); }

test('⌘/Ctrl+K 开始听，再按一次停止；在输入框里不触发', async () => {
  const env = makeEnv({ withSR: true });
  globalThis.__panel = { controls: [], cards: [], isOnline: () => true, set: async () => true,
    setMany: async () => ({ ok: true }), get: () => null, refresh: async () => {}, armReboot: () => false };
  env.store.panelKey = 'test-key';
  loadScripts();

  fireKey(env, { key: 'k', metaKey: true, target: { tagName: 'BODY' }, preventDefault() {} });
  assert.equal(env.srStarts, 1, '⌘K 应该开始听');
  fireKey(env, { key: 'k', metaKey: true, target: { tagName: 'BODY' }, preventDefault() {} });
  assert.equal(env.srStarts, 1, '再按一次应该停止（不再 start）');

  fireKey(env, { key: 'k', ctrlKey: true, target: { tagName: 'INPUT' }, preventDefault() {} });
  assert.equal(env.srStarts, 1, '在输入框里按不该触发');

  fireKey(env, { key: 'k', ctrlKey: true, target: { tagName: 'BODY' }, preventDefault() {} });
  assert.equal(env.srStarts, 2, 'Ctrl+K 在 Windows/Linux 也要能用');
});

test('Esc 停止听', async () => {
  const env = makeEnv({ withSR: true });
  globalThis.__panel = { controls: [], cards: [], isOnline: () => true, set: async () => true,
    setMany: async () => ({ ok: true }), get: () => null, refresh: async () => {}, armReboot: () => false };
  env.store.panelKey = 'test-key';
  loadScripts();
  fireKey(env, { key: 'k', metaKey: true, target: { tagName: 'BODY' }, preventDefault() {} });
  assert.equal(env.srStarts, 1);
  fireKey(env, { key: 'Escape', target: { tagName: 'BODY' } });
  assert.match(env.els.voiceLabel.textContent, /语音/, '停止后按钮标签回到"语音"');
  await new Promise(r => setTimeout(r, 260));            // 浮层是淡出后再收起（170~200ms）
  assert.equal(env.els.voiceBar.hidden, true, '状态条收起来');
});

test('下拉框标签变短（名字 · 语言），完整名字放 title', async () => {
  const env = makeEnv();
  globalThis.speechSynthesis.getVoices = () => [
    { name: '婷婷', lang: 'zh-CN' },
    { name: 'Eddy (中文（中国大陆）)', lang: 'zh-CN' },
    { name: 'Sin-ji', lang: 'zh-HK' }];
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceCfgBtn }));
  const html = env.els.voiceSel.innerHTML;
  assert.match(html, /婷婷 · 普通话/);
  assert.match(html, /Eddy · 普通话/, '长名字应被截短');
  assert.match(html, /Sin-ji · 粤语/, '语言应中文化');
  assert.ok(!/（zh-CN）/.test(html.replace(/title="[^"]*"/g, '')), '可见文本里不该出现 (zh-CN)');
  assert.match(html, /title="Eddy \(中文（中国大陆）\)（zh-CN）"/, '完整名字要在 title 里');
});
