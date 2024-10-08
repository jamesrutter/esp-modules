#include <Arduino.h>
#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <WiFi.h>
#include "esp_camera.h"

#define CAMERA_MODEL_XIAO_ESP32S3 // Has PSRAM
#include "camera_pins.h"

const char *ssid = "Buckminster";
const char *password = "gooddog4";
const char *local_ws_server = "192.168.40.90";

const uint32_t fs = 0xABCDEF00;
const uint32_t fe = 0xABCDEF99;

WebSocketsClient websocket;

// WebSocket event handler
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[WEBSOCKET]: Disconnected.\n");
    break;
  case WStype_CONNECTED:
    Serial.printf("[WEBSOCKET]: Connected to url: %s\n", payload);
    break;
  case WStype_TEXT:
    Serial.printf("[WEBSOCKET] Received text: %s\n", payload);
    break;
  case WStype_BIN:
    Serial.printf("[WEBSOCKET] Received binary data\n");
    break;
  case WStype_ERROR:
    Serial.printf("[WEBSOCKET] Error: %s\n", payload);
    break;
  case WStype_PING:
    Serial.printf("[WEBSOCKET] Received ping\n");
    break;
  case WStype_PONG:
    Serial.printf("[WEBSOCKET] Received pong\n");
    break;
  }
}

void setup()
{
  // Serial.begin(115200);

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    return;
  }

  sensor_t *s = esp_camera_sensor_get();

  s->set_framesize(s, FRAMESIZE_HD);

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
  }

  // Set up WebSocket Connection
  websocket.beginSSL("maker.deno.dev", 443, "/ws/esp32"); // PRODUCTION
  // websocket.begin(local_ws_server, 8000, "/ws/esp32"); // LOCAL
  websocket.onEvent(webSocketEvent);
  websocket.setReconnectInterval(5000);
}

void loop()
{

  websocket.loop();

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb)
  {
    esp_camera_fb_return(fb);
    return;
  }

  // Implement a simple protocol for the binary data
  websocket.sendBIN((uint8_t *)&fs, sizeof(fs));
  websocket.sendBIN(fb->buf, fb->len);
  websocket.sendBIN((uint8_t *)&fe, sizeof(fe));

  esp_camera_fb_return(fb);

  delay(25);
}
