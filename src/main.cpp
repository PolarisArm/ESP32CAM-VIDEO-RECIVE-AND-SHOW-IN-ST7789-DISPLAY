#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <html.h>
#include <TJpg_Decoder.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "free_font.h"

const char* ssid = "ESP";
const char* password = "12345678";
const char* mdnsAdd = "esp";

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(82);

TFT_eSPI tft = TFT_eSPI();


unsigned long lastMillis = 0;
char buf[50];

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if(y >= tft.height()) {return 0;}
  tft.pushImage(x,y,w,h,bitmap);
  return 1;
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Client disconnected\n", num);
      break;
      
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connection from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
      break;
    }

    case WStype_TEXT:
      Serial.printf("[%u] Received text: %s\n", num, payload);
      break;

    case WStype_BIN:
    {
      Serial.printf("[%u] Received binary data, length: %u, free Heap: %u\n", num, length,ESP.getFreeHeap());
      
      uint16_t  widthI = 0;
      uint16_t heightI = 0;
      
      TJpgDec.getJpgSize(&widthI,&heightI,(const uint8_t*)payload,length);
      TJpgDec.drawJpg(0,0,(const uint8_t*)payload,length);

      unsigned long time = millis()-lastMillis;
      lastMillis = millis();
      sprintf(buf,"Time: %lu ms",time);
      tft.setTextColor(TFT_WHITE, TFT_TRANSPARENT);
      tft.drawString(buf,80,220);
      break;
    }

  }
}

void setup() {
  Serial.begin(115200);
  delay(20);

  WiFi.softAP(ssid,password);
  IPAddress softApIp = WiFi.softAPIP();
  Serial.println(softApIp); 

  // Start mDNS
  if (!MDNS.begin(mdnsAdd)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.println("mDNS responder started at esp.local");
  }

  // Configure HTTP server
  server.on("/", HTTP_GET, []() {
    String html = index_html;
    server.send(200, "text/html", html);
  });

  server.begin();

  tft.begin();
  tft.setTextColor(0xFFFF,0x0000);
  tft.fillScreen(TFT_GREENYELLOW);
  tft.setFreeFont(FM12);
  
  // Image Scaling
  TJpgDec.setJpgScale(1);

 // The byte order can be swapped (set true for TFT_eSPI)
  TJpgDec.setSwapBytes(true);

  TJpgDec.setCallback(tft_output);

  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}