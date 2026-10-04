#include "app_device.h"

#include "app_config.h"
#include "app_onenet.h"
#include "board_pins.h"
#include "beep.h"
#include "bsp_dht11.h"
#include "cJSON.h"
#include "fan.h"
#include "led.h"

#include <stdio.h>
#include <string.h>

/* ============================================================
 *  本地状态（物模型的当前值）
 * ============================================================ */
static uint8_t s_led = 0;
static uint8_t s_buzzer = 0;
static uint8_t s_fan = 0;

/* ============================================================
 *  工具
 * ============================================================ */

/* 把云端来的 id 限制成 [0-9A-Za-z_-]，其它字符丢掉；空则用 "0"。
 * 直接把任意内容 snprintf 进 JSON 会被引号/反斜杠破坏整个回复报文。 */
static void sanitize_id(const char *src, char *dst, size_t n) {
  size_t k = 0;

  if (n == 0)
    return;

  if (src != NULL) {
    for (; *src != '\0' && k + 1 < n; src++) {
      char c = *src;
      if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') ||
          (c >= 'a' && c <= 'z') || c == '_' || c == '-')
        dst[k++] = c;
    }
  }
  if (k == 0)
    dst[k++] = '0';
  dst[k] = '\0';
}

static uint8_t str_is_on(const char *v) {
  return v && (!strcmp(v, "on") || !strcmp(v, "1") || !strcmp(v, "open") ||
               !strcmp(v, "true"));
}

static uint8_t str_is_off(const char *v) {
  return v && (!strcmp(v, "off") || !strcmp(v, "0") || !strcmp(v, "close") ||
               !strcmp(v, "false"));
}

/* 属性设置回执（id 用清洗过的，code/msg 固定） */
static void reply_property_set(const char *id) {
  char resp[160];

  snprintf(resp, sizeof resp, "{\"id\":\"%s\",\"code\":200,\"msg\":\"success\"}",
           id);
  if (OneNET_Publish("/thing/property/set_reply", resp) != ONENET_OK)
    printf("[DEV] set_reply 发送失败\r\n");
}

/* ============================================================
 *  下行：属性设置（LED 字符串 / buzzer 布尔 / fan 布尔）
 * ============================================================ */
static void on_property_set(const char *topic, const char *payload, size_t len) {
  char id[24];
  const cJSON *jid, *params, *item;
  cJSON *root;

  (void)topic;

  root = cJSON_ParseWithLength(payload, len);
  if (root == NULL) {
    printf("[DEV] set 报文不是合法 JSON（%u 字节）：%s\r\n", (unsigned)len,
           payload);
    return;
  }

  jid = cJSON_GetObjectItem(root, "id");
  sanitize_id(cJSON_IsString(jid) ? jid->valuestring : NULL, id, sizeof id);

  params = cJSON_GetObjectItem(root, "params");
  if (!cJSON_IsObject(params)) {
    printf("[DEV] set 报文里没有 params 对象\r\n");
    cJSON_Delete(root);
    reply_property_set(id);
    return;
  }

  /* ---- LED ---- */
  item = cJSON_GetObjectItem(params, "LED");
  if (cJSON_IsString(item)) {
    if (str_is_on(item->valuestring)) {
      s_led = 1;
      LED_On(BOARD_APP_LED_INDEX);
    } else if (str_is_off(item->valuestring)) {
      s_led = 0;
      LED_Off(BOARD_APP_LED_INDEX);
    } else {
      printf("[DEV] LED 未知指令：%s\r\n", item->valuestring);
    }
    printf("[DEV] LED=%s -> %u\r\n", item->valuestring, (unsigned)s_led);
  } else if (cJSON_IsBool(item)) {
    s_led = cJSON_IsTrue(item) ? 1 : 0;
    LED_Set(BOARD_APP_LED_INDEX, s_led);
    printf("[DEV] LED=%u\r\n", (unsigned)s_led);
  }

  /* ---- buzzer ---- */
  item = cJSON_GetObjectItem(params, "buzzer");
  if (cJSON_IsBool(item)) {
    s_buzzer = cJSON_IsTrue(item) ? 1 : 0;
    BEEP_Set(s_buzzer);
    printf("[DEV] buzzer=%u\r\n", (unsigned)s_buzzer);
  }

  /* ---- fan ---- */
  item = cJSON_GetObjectItem(params, "fan");
  if (cJSON_IsBool(item)) {
    s_fan = cJSON_IsTrue(item) ? 1 : 0;
    FAN_Set(s_fan);
    printf("[DEV] fan=%u\r\n", (unsigned)s_fan);
  }

  cJSON_Delete(root);
  reply_property_set(id);
}

/* ============================================================
 *  下行：命令（cmd/request/<id>）—— 需在 OneNET 侧订阅后才有数据
 * ============================================================ */
static void on_cmd(const char *topic, const char *payload, size_t len) {
  const char *p = strstr(topic, "/cmd/request/");
  char id[24];
  char resp_topic[64];
  char resp[160];
  cJSON *root;

  sanitize_id(p ? p + strlen("/cmd/request/") : NULL, id, sizeof id);

  root = cJSON_ParseWithLength(payload, len);
  if (root != NULL) {
    const cJSON *cmd = cJSON_GetObjectItem(root, "cmd");
    if (cJSON_IsString(cmd)) {
      if (!strcmp(cmd->valuestring, "led_on")) {
        s_led = 1;
        LED_On(BOARD_APP_LED_INDEX);
      } else if (!strcmp(cmd->valuestring, "led_off")) {
        s_led = 0;
        LED_Off(BOARD_APP_LED_INDEX);
      } else {
        printf("[DEV] 未知 cmd：%s\r\n", cmd->valuestring);
      }
    }
    cJSON_Delete(root);
  }

  snprintf(resp_topic, sizeof resp_topic, "/cmd/response/%s", id);
  snprintf(resp, sizeof resp, "{\"id\":\"%s\",\"code\":200,\"msg\":\"success\"}",
           id);
  if (OneNET_Publish(resp_topic, resp) != ONENET_OK)
    printf("[DEV] cmd 回执发送失败\r\n");
}

/* ============================================================
 *  注册 / 上报
 * ============================================================ */
static const OnenetHandler s_handlers[] = {
    {.topic_suffix = "/thing/property/set", .handler = on_property_set,
     .match_anywhere = 0},
    /* topic 尾部是变量 id，用子串匹配 */
    {.topic_suffix = "/cmd/request/", .handler = on_cmd, .match_anywhere = 1},
};

void Device_Init(void) {
  /* 只注册下行处理表。引脚的时钟/模式/初始电平由 CubeMX 的 MX_GPIO_Init()
   * 配置，这里不做任何 GPIO 初始化。 */
  OneNET_SetHandlers(s_handlers, sizeof s_handlers / sizeof s_handlers[0]);
}

void Device_ReportTempHumi(void) {
  DHT11_Data_t d = DHT11_GetData();
  char params[96];
  int n;

  n = snprintf(params, sizeof params,
               "{\"temperature\":{\"value\":%d.%d},"
               "\"humidity\":{\"value\":%d.%d}}",
               d.temp_int, d.temp_dec, d.humi_int, d.humi_dec);
  if (n <= 0 || n >= (int)sizeof params) {
    printf("[DEV] 上报 params 组装失败\r\n");
    return;
  }

  if (OneNET_PublishProperty(params) != ONENET_OK)
    printf("[DEV] 温湿度上报失败\r\n");
}
