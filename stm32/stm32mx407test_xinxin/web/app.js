const { createApp, ref, reactive, computed, nextTick, onMounted, onBeforeUnmount } = Vue;

/* =========================================================
   配置（已适配你的设备）
   ========================================================= */
/* 配置来自 config.js（同源后端代理，前端不再持有设备 token）*/
const config = window.PANEL_CONFIG;
const API   = window.PanelAPI;

/* ---------------------------------------------------------
   属性下发时的取值约定（如与实际物模型不符，只改这里）
   - 灯 1/2/3   布尔型：true / false（若设备端用 'on'/'off' 就改下面两个常量）
   - buzzer/fan 布尔型：true / false

   标识符/类型必须与 OneNET 物模型一致（改物模型只改这一块）：
     led1 / led2 / led3  string(8)  → 'on' / 'off'
     buzzer / fan        bool       → true / false
--------------------------------------------------------- */
const BUZZER_ON   = true;
const BUZZER_OFF  = false;
const FAN_ON      = true;
const FAN_OFF     = false;
const LIGHT_ON    = 'on';
const LIGHT_OFF   = 'off';

/* ---------------------------------------------------------
   控制项清单（一个按钮一个开关）
   想改标识符 / 名字 / 颜色，只改这个数组即可
--------------------------------------------------------- */
const CONTROLS = [
  { key:'led1',   id:'led1',   name:'灯 1',     icon:'fa-lightbulb-o', color:'#f59e0b', on:LIGHT_ON,  off:LIGHT_OFF  },
  { key:'led2',   id:'led2',   name:'灯 2',     icon:'fa-lightbulb-o', color:'#10b981', on:LIGHT_ON,  off:LIGHT_OFF  },
  { key:'led3',   id:'led3',   name:'灯 3',     icon:'fa-lightbulb-o', color:'#8b5cf6', on:LIGHT_ON,  off:LIGHT_OFF  },
  { key:'buzzer', id:'buzzer', name:'蜂鸣器',   icon:'fa-bell-o',      color:'#dc2626', on:BUZZER_ON, off:BUZZER_OFF },
  { key:'fan',    id:'fan',    name:'风扇',     icon:'fa-snowflake-o', color:'#2563eb', on:FAN_ON,    off:FAN_OFF    }
];

/* 属性卡片（温度 / 湿度）—— 点击可看历史曲线；灯在下面的控制区 */
const CARDS = [
  { id:'temperature', name:'温度', unit:'℃',   type:'float',  icon:'fa-thermometer-half', color:'#3b82f6' },
  { id:'humidity',    name:'湿度', unit:'%RH', type:'float',  icon:'fa-tint',             color:'#10b981' }
];

/* 判断一个云端值算不算“开” */
function isOn(v){
  return v === true || v === 1 || v === '1' ||
         v === 'on' || v === 'ON' || v === 'true' || v === 'True';
}

createApp({
  setup() {
    /* ---------- 状态 ---------- */
    const online     = ref(null);
    const refreshing = ref(false);

    // 重启按钮：rebootArm = 已进入确认态；rebooting = 请求中；rebootMsg = 结果提示
    const rebootArm = ref(false);
    const rebooting = ref(false);
    const rebootMsg = ref('');
    let rebootTimer = null, rebootTipTimer = null;

    // “指令已下发” / “下发失败”提示：tip[控制项 key] = true/false
    const tip    = reactive({});
    const tipErr = reactive({});
    const timers = {};

    // 从云端读取到的当前值
    const state = reactive({
      temperature: null,
      humidity:    null
    });
    CARDS.forEach(c => { state[c.id] = null; });
    CONTROLS.forEach(c => { state[c.id] = null; });

    // 本地乐观值：点一下先让开关动起来，等云端回读到真实值再交还
    // （设备没上报过该属性时云端会是 null，所以不能拿 null 直接覆盖）
    const local   = reactive({});   // id -> 值
    const localAt = {};             // id -> 时间戳
    const LOCAL_HOLD_MS = 8000;

    const hist = reactive({
      open:false, loading:false, empty:false,
      emptyText:'暂无历史数据',
      title:'', unit:'', color:'#3b82f6', id:''
    });

    let chart = null;
    let pollTimer = null, reqId = 0;

    /* ---------- 计算属性 ---------- */
    const statusText = computed(() =>
      online.value === null ? '检测中' : (online.value ? '在线' : '离线')
    );
    const statusColor = computed(() =>
      online.value === null ? '#f59e0b' : (online.value ? '#10b981' : '#f43f5e')
    );

    // 某个属性当前该显示成什么：本地乐观值（限时）优先，否则用云端值
    function currentValue(id) {
      const t = localAt[id];
      if (local[id] !== undefined && t && Date.now() - t < LOCAL_HOLD_MS) return local[id];
      return state[id];
    }

    // { led1:true, led2:false, buzzer:true, fan:false, ... }
    const controlOn = computed(() => {
      const m = {};
      for (const c of CONTROLS) m[c.key] = isOn(currentValue(c.id));
      return m;
    });

    /* ---------- 工具 ---------- */
    function hexA(hex, a) {
      const h = hex.replace('#', '');
      const r = parseInt(h.slice(0, 2), 16);
      const g = parseInt(h.slice(2, 4), 16);
      const b = parseInt(h.slice(4, 6), 16);
      return `rgba(${r},${g},${b},${a})`;
    }

    /* 所有 OneNET 调用都走后端 op 白名单；前端不再拼 URL、不带 token */
    async function api(op, params, body) {
      try {
        return await API.post(op, params, body);
      } catch (err) {
        if (err.needKey && API.ensureKey()) return await API.post(op, params, body);
        throw err;
      }
    }

    function fmtTime(ts) {
      const d = new Date(ts);
      const p = n => String(n).padStart(2, '0');
      return `${p(d.getMonth() + 1)}-${p(d.getDate())} ${p(d.getHours())}:${p(d.getMinutes())}`;
    }

    /* ---------- 1. 设备在线状态 ---------- */
    async function refreshStatus() {
      try {
        const data = await api('deviceDetail');
        if (data && data.code === 0 && data.data) {
          online.value = Number(data.data.status) === 1;
        } else {
          online.value = false;
          console.warn('[设备状态]', data && data.msg);
        }
      } catch (err) {
        console.error('[设备状态] 请求失败：', err);
        online.value = false;
      }
    }

    /* ---------- 2. 拉取属性值 ---------- */
    async function refreshProperties() {
      try {
        const data = await api('getProperty');
        if (!data || data.code !== 0) {
          console.warn('[属性]', data && data.msg);
          return;
        }
        const list = Array.isArray(data.data) ? data.data : [];
        list.forEach(item => {
          const id = item.identifier;
          if (!Object.prototype.hasOwnProperty.call(state, id)) return;
          // 设备从没上报过的属性，平台不返回 value：不要用它把已知状态清空
          if (item.value === null || item.value === undefined) return;
          state[id] = item.value;
          // 云端已给出真实值：撤掉本地乐观值（值一致立刻撤；不一致等超时）
          if (local[id] !== undefined &&
              (isOn(item.value) === isOn(local[id]) ||
               Date.now() - (localAt[id] || 0) > LOCAL_HOLD_MS)) {
            delete local[id];
            delete localAt[id];
          }
        });
      } catch (err) {
        console.error('[属性] 请求失败：', err);
      }
    }

    async function refreshAll() {
      if (refreshing.value) return;
      refreshing.value = true;
      await Promise.allSettled([refreshStatus(), refreshProperties()]);
      refreshing.value = false;
    }

    /* ---------- 卡片数值展示 ---------- */
    function displayValue(card) {
      const raw = state[card.id];
      if (raw === null || raw === undefined || raw === '') return '--';
      if (card.type === 'string') return String(raw);
      const n = Number(raw);
      if (isNaN(n)) return String(raw);
      return card.type === 'int' ? String(Math.round(n)) : n.toFixed(1);
    }

    /* ---------- 下发（离线拦截） ---------- */
    function sendProperty(params, label) {
      if (!online.value) {
        console.warn('[下发指令] 设备离线，已阻止下发');
        return Promise.resolve(false);
      }
      return api('setProperty', null, params).then(data => {
        if (!data || data.code !== 0) {
          console.warn('[下发指令] 平台拒绝：', label, data && (data.msg || ('code=' + data.code)));
          return false;
        }
        return true;
      }).catch(err => {
        console.error('[下发指令] 请求异常：', err);
        return false;
      });
    }

    /* 通用：显示“指令已下发”2 秒 */
    function flashTip(key) {
      tip[key] = true;
      clearTimeout(timers[key]);
      timers[key] = setTimeout(() => { tip[key] = false; }, 2000);
    }

    /* 通用：显示“下发失败”3 秒 */
    function flashErr(key) {
      tipErr[key] = true;
      clearTimeout(timers[key + ':err']);
      timers[key + ':err'] = setTimeout(() => { tipErr[key] = false; }, 3000);
    }

    /* ---------- 3. 单按钮切换 ---------- */
    async function toggleControl(c) {
      if (!online.value) return;

      const next  = !controlOn.value[c.key];
      const value = next ? c.on : c.off;

      // 先动手：本地立刻翻转，避免“点了没反应”
      local[c.id]   = value;
      localAt[c.id] = Date.now();
      tip[c.key]    = false;
      tipErr[c.key] = false;

      const ok = await sendProperty({ [c.id]: value }, c.name);

      if (!ok) {
        // 下发失败（属性不存在 / 设备离线 / 平台拒绝）：回退开关并提示
        delete local[c.id];
        delete localAt[c.id];
        flashErr(c.key);
        return;
      }

      flashTip(c.key);
      // 下发成功后回读一次（设备上报有延迟，隔两档拉）
      setTimeout(refreshProperties, 1200);
      setTimeout(refreshProperties, 3500);
    }

    /* ---------- 3.5 物模型服务调用（重启按钮走这条） ---------- */
    async function callService(identifier, params) {
      try {
        const data = await api('callService', null, { identifier, params: params || {} });
        if (!data || data.code !== 0) {
          console.warn('[服务调用] 失败：', identifier, data && (data.msg || data.code));
          return { ok: false, msg: (data && data.msg) || '' };
        }
        return { ok: true };
      } catch (err) {
        console.error('[服务调用] 请求异常：', err);
        return { ok: false, msg: String(err) };
      }
    }

    /* 重启：两段式点击（第一次进入确认态，4 秒内再点才真发），避免误触复位板子 */
    function onRebootClick() {
      if (!online.value || rebooting.value) return;

      if (!rebootArm.value) {
        rebootArm.value = true;
        clearTimeout(rebootTimer);
        rebootTimer = setTimeout(() => { rebootArm.value = false; }, 4000);
        return;
      }

      clearTimeout(rebootTimer);
      rebootArm.value = false;
      doReboot();
    }

    async function doReboot() {
      rebooting.value = true;
      rebootMsg.value = '';
      const r = await callService('reboot', {});
      rebooting.value = false;

      if (r.ok) {
        rebootMsg.value = '重启指令已下发，设备约十几秒后重新上线';
        setTimeout(refreshAll, 12000);
        setTimeout(refreshAll, 25000);
      } else {
        rebootMsg.value = '重启失败：' + (r.msg || '物模型里建 reboot 服务了吗？');
      }
      clearTimeout(rebootTipTimer);
      rebootTipTimer = setTimeout(() => { rebootMsg.value = ''; }, 20000);
    }

    /* ---------- 4. 历史数据 ---------- */
    function handleResize() { if (chart) chart.resize(); }

    async function openHistory(card) {
      const my = ++reqId;

      hist.open      = true;
      hist.id        = card.id;
      hist.title     = card.name;
      hist.unit      = card.unit;
      hist.color     = card.color;
      hist.empty     = false;
      hist.emptyText = '暂无历史数据';
      hist.loading   = true;

      await nextTick();

      const el = document.getElementById('historyChart');
      if (!el) { hist.loading = false; return; }

      if (chart) { chart.dispose(); chart = null; }
      chart = echarts.init(el);
      window.removeEventListener('resize', handleResize);
      window.addEventListener('resize', handleResize);

      try {
        const end   = Date.now();
        const start = end - 24 * 60 * 60 * 1000;
        const params = {
          identifier: card.id,
          start_time: start,
          end_time: end,
          limit: 100
        };

        const data = await api('getHistory', params);
        if (my !== reqId) return;

        if (!data || data.code !== 0) {
          throw new Error((data && data.msg) || '获取历史数据失败');
        }

        const raw  = (data.data && data.data.list) || [];
        const list = raw
          .map(it => ({ t: Number(it.time), v: it.value }))
          .filter(it => !isNaN(it.t))
          .sort((a, b) => a.t - b.t);

        hist.loading = false;

        if (!list.length) {
          hist.empty = true;
          hist.emptyText = '暂无历史数据';
          chart.clear();
          return;
        }

        renderChart(list, card);
      } catch (err) {
        if (my !== reqId) return;
        console.error('[历史数据] 请求失败：', err);
        hist.loading = false;
        hist.empty = true;
        hist.emptyText = '历史数据加载失败';
        chart.clear();
      }
    }

    function renderChart(list, card) {
      const isNum = v =>
        v !== '' && v !== null && v !== undefined &&
        typeof v !== 'boolean' && !isNaN(Number(v));

      const numeric = list.every(it => isNum(it.v));

      const times  = list.map(it => fmtTime(it.t));
      const values = numeric ? list.map(it => Number(it.v)) : list.map(it => String(it.v));

      chart.setOption({
        grid: { left: 4, right: 16, top: 26, bottom: 4, containLabel: true },
        tooltip: {
          trigger: 'axis',
          backgroundColor: 'rgba(255,255,255,.97)',
          borderColor: '#e4e4e7',
          borderWidth: 1,
          padding: [8, 12],
          textStyle: { color: '#18181b', fontSize: 12 },
          valueFormatter: v =>
            (v === null || v === undefined ? '--' : v) + (card.unit ? ' ' + card.unit : '')
        },
        xAxis: {
          type: 'category',
          boundaryGap: false,
          data: times,
          axisLine: { lineStyle: { color: '#e4e4e7' } },
          axisTick: { show: false },
          axisLabel: { color: '#a1a1aa', fontSize: 11, hideOverlap: true }
        },
        yAxis: {
          type: numeric ? 'value' : 'category',
          scale: numeric,
          name: card.unit || '',
          nameTextStyle: { color: '#a1a1aa', fontSize: 11 },
          splitLine: { lineStyle: { color: '#f4f4f5' } },
          axisLabel: { color: '#a1a1aa', fontSize: 11 },
          axisLine: { show: false },
          axisTick: { show: false }
        },
        series: [{
          name: card.name,
          type: 'line',
          smooth: numeric,
          showSymbol: list.length <= 30,
          symbolSize: 6,
          data: values,
          lineStyle: { width: 2, color: card.color },
          itemStyle: { color: card.color },
          areaStyle: numeric ? {
            color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [
              { offset: 0, color: hexA(card.color, .28) },
              { offset: 1, color: hexA(card.color, .02) }
            ])
          } : undefined
        }]
      }, true);

      chart.resize();
    }

    function closeHistory() {
      hist.open = false;
      reqId++;
      if (chart) { chart.dispose(); chart = null; }
      window.removeEventListener('resize', handleResize);
    }

    function onKeydown(e) {
      if (e.key === 'Escape' && hist.open) closeHistory();
    }

    /* ---------- 4.5 面板桥：给语音层用（复用同一套乐观更新/回读/两段确认） ---------- */
    async function setControl(key, value) {
      const c = CONTROLS.find(x => x.key === key);
      if (!c || !online.value) return false;

      const want = !!value;
      if (controlOn.value[c.key] === want) return true;   // 已经是要的状态，不重复下发

      const v = want ? c.on : c.off;
      local[c.id]   = v;
      localAt[c.id] = Date.now();

      const ok = await sendProperty({ [c.id]: v }, c.name);
      if (!ok) {
        delete local[c.id];
        delete localAt[c.id];
        flashErr(c.key);
        return false;
      }
      flashTip(c.key);
      setTimeout(refreshProperties, 1200);
      setTimeout(refreshProperties, 3500);
      return true;
    }

    /* 多目标：合成**一条** setProperty（固件 on_property_set 会遍历所有匹配项），
       不要循环发多条 —— ESP8266 背靠背发 MQTTPUBRAW 会回 ERROR（踩过）。 */
    async function setControls(keys, value) {
      const list = (keys || [])
        .map(k => CONTROLS.find(c => c.key === k))
        .filter(c => c && controlOn.value[c.key] !== !!value);   // 已经是这个状态的跳过
      if (!online.value) return { ok: false, changed: 0, msg: '设备离线' };
      if (!list.length) return { ok: true, changed: 0 };

      const params = {};
      list.forEach(c => {
        params[c.id] = value ? c.on : c.off;
        local[c.id]   = params[c.id];
        localAt[c.id] = Date.now();
      });

      const ok = await sendProperty(params, list.map(c => c.name).join('/'));
      if (!ok) {
        list.forEach(c => { delete local[c.id]; delete localAt[c.id]; flashErr(c.key); });
        return { ok: false, changed: 0 };
      }
      list.forEach(c => flashTip(c.key));
      setTimeout(refreshProperties, 1200);
      setTimeout(refreshProperties, 3500);
      return { ok: true, changed: list.length, names: list.map(c => c.name) };
    }

    const panelBridge = {
      controls: CONTROLS.map(c => ({ key: c.key, name: c.name })),
      cards:    CARDS.map(c => ({ id: c.id, name: c.name, unit: c.unit })),
      isOnline: () => online.value === true,
      get:      (key) => currentValue(key),
      set:      setControl,          // 返回值：true 成功 / false 失败
      setMany:  setControls,         // 一次多条：{ok, changed, names}
      refresh:  refreshAll,
      /* 重启这类破坏性动作：只把面板按钮推到"再点一次确认"状态，真正的执行仍要人工点第二下 */
      armReboot() { if (!rebootArm.value && online.value) onRebootClick(); return rebootArm.value; }
    };
    window.__panel = panelBridge;

    /* ---------- 生命周期 ---------- */
    onMounted(() => {
      refreshAll();
      pollTimer = setInterval(refreshAll, 10000);
      window.addEventListener('keydown', onKeydown);
    });

    onBeforeUnmount(() => {
      clearInterval(pollTimer);
      clearTimeout(rebootTimer);
      clearTimeout(rebootTipTimer);
      Object.values(timers).forEach(t => clearTimeout(t));
      window.removeEventListener('keydown', onKeydown);
      window.removeEventListener('resize', handleResize);
      if (chart) { chart.dispose(); chart = null; }
      if (window.__panel === panelBridge) delete window.__panel;
    });

    return {
      cards: CARDS,
      controls: CONTROLS,
      deviceName: config.deviceName,
      productId: config.productId,

      online, statusText, statusColor,
      refreshing, refreshAll,

      rebootArm, rebooting, rebootMsg, onRebootClick,

      state, displayValue, hexA,
      controlOn, toggleControl,
      tip, tipErr,

      hist, openHistory, closeHistory
    };
  }
}).mount('#app');
