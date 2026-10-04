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
  closest(sel) { return sel === '#voiceBtn' && this.id === 'voiceBtn' ? this : null; }
}

function makeEnv({ withSR = false } = {}) {
  const els = {
    voiceBtn: new El('voiceBtn'),
    voiceLabel: new El('voiceLabel'),
    voiceBar: new El('voiceBar'),
    voiceText: new El('voiceText')
  };
  els.voiceBar.hidden = true;
  const doc = {
    listeners: {},
    addEventListener(t, f) { (this.listeners[t] ||= []).push(f); },
    getElementById: id => els[id] || null,
    querySelectorAll: () => [],
    readyState: 'complete'
  };
  const store = {};
  const calls = { fetch: [], set: [], speak: [] };

  globalThis.window = globalThis;
  globalThis.document = doc;
  globalThis.localStorage = {
    getItem: k => (k in store ? store[k] : null),
    setItem: (k, v) => { store[k] = String(v); },
    removeItem: k => { delete store[k]; }
  };
  globalThis.fetch = async (url, opts) => { calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  globalThis.speechSynthesis = { cancel() {}, speak(u) { calls.speak.push(u.text); } };
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
  globalThis.SpeechSynthesisUtterance = class { constructor(t) { this.text = t; } };

  if (withSR) {
    class FakeSR {
      start() { this.started = true; env.lastSR = this; if (this.onstart) this.onstart(); }
      stop() { if (this.onend) this.onend(); }
    }
    globalThis.SpeechRecognition = FakeSR;
  } else {
    delete globalThis.SpeechRecognition;
    delete globalThis.webkitSpeechRecognition;
  }

  const env = {
    els, calls, doc, store,
    lastSR: null,
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

test('toggle 缺少目标时不下发，只提示', async () => {
  const env = makeEnv({ withSR: true });
  env.fetchReply = async () => ({ ok: true, status: 200, json: async () => ({ intent: { action: 'toggle', target: null, value: null, reply: '要开哪一路？' } }) });
  globalThis.fetch = async (url, opts) => { env.calls.fetch.push({ url, opts }); return env.fetchReply(url, opts); };
  globalThis.__panel = {
    controls: [{ key: 'led1', name: '灯 1' }], cards: [], isOnline: () => true,
    set: async () => { throw new Error('不该下发'); }, get: () => null, refresh: async () => {}, armReboot: () => false
  };
  env.store.panelKey = 'test-key';
  loadScripts();
  env.doc.listeners.click.forEach(f => f({ target: env.els.voiceBtn }));
  const results = [[{ transcript: '开灯' }]];
  results[0].isFinal = true;
  env.lastSR.onresult({ resultIndex: 0, results });
  await new Promise(r => setTimeout(r, 30));
  assert.match(env.els.voiceText.textContent, /要开哪一路？/);
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
