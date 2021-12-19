#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <FastLED.h>
#include "credentials.h"
#include "varDeclaration.h"

#define DEBUG_STATE true
#if DEBUG_STATE
  #define Serialprint(...) Serial.print(__VA_ARGS__)
  #define Serialprintln(...) Serial.println(__VA_ARGS__)
  #define Serialprintf(...) Serial.printf(__VA_ARGS__)
  #define Serialbegin(baud) Serial.begin(baud)
#else
  #define Serialprint(...)
  #define Serialprintln(...)
  #define Serialprintf(...)
  #define Serialbegin(baud)
#endif

#define DATA_PIN    2
#define RGB_ORDER   GRB
#define CHIPSET     WS2812
#define NUM_LEDS    594
#define BRIGHTNESS  50

CRGB leds[NUM_LEDS];
AsyncWebServer server(80);

///////////////////////////////////////////////////////////////////////////////
String getContentType(String url){
  if(url.endsWith(".htm"))       return "text/html";
  else if(url.endsWith(".html")) return "text/html";
  else if(url.endsWith(".css"))  return "text/css";
  else if(url.endsWith(".js"))   return "application/javascript";
  else if(url.endsWith(".png"))  return "image/png";
  else if(url.endsWith(".gif"))  return "image/gif";
  else if(url.endsWith(".jpg"))  return "image/jpeg";
  else if(url.endsWith(".ico"))  return "image/x-icon";
  else if(url.endsWith(".xml"))  return "text/xml";
  else if(url.endsWith(".pdf"))  return "application/x-pdf";
  else if(url.endsWith(".zip"))  return "application/x-zip";
  else if(url.endsWith(".gz"))   return "application/x-gzip";
  return "text/plain";
} // end of getContentType
///////////////////////////////////////////////////////////////////////////////
String formatBytes(size_t bytes){
  if (bytes < 1024) return String(bytes)+"B";
  else if(bytes < (1024 * 1024)) return String(bytes/1024.0)+"KB";
  else if(bytes < (1024 * 1024 * 1024)) return String(bytes/1024.0/1024.0)+"MB";
  else return String(bytes/1024.0/1024.0/1024.0)+"GB";
} // end of formatBytes
///////////////////////////////////////////////////////////////////////////////
void handleServeFile(AsyncWebServerRequest *request) {
  String url = request->url();
  if(url == "/") url = "/scoreboard.html";          // default page
  if(LittleFS.exists(url + ".gz")) url += ".gz";    // use gzip version if available
  if(LittleFS.exists(url)){
    String contentType = getContentType(url);
    request->send(LittleFS, url, contentType, false);
  }
  else {
    request->send(404, "text/plain", url + " Not found");
  }
}
///////////////////////////////////////////////////////////////////////////////
void setup() {
  Serialbegin(115200);
//-------------------------------------------------------------
  // start LittleFS file system
  LittleFS.begin();               
  delay(1000);
  {
    Dir dir = LittleFS.openDir(F("/"));
    Serialprintln("\nFile System:");
    while (dir.next()) {
      Serialprintf(" File: %-28s size: %8s\n", dir.fileName().c_str(), \
                   formatBytes(dir.fileSize()).c_str());
    }
    Serialprintf("\n");
  }
//-------------------------------------------------------------
  // Start WiFi
  WiFi.mode(WIFI_STA);            //TODO change to AP mode
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serialprintf("WiFi Failed!\n");
      return;
  }
  Serialprintf("\nIP Address: %s\n", WiFi.localIP().toString().c_str());
//-------------------------------------------------------------
  // Start web server
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    String score = request->arg("score");
    String overs = request->arg("overs");
    String wicket = request->arg("wicket");
    String target = request->arg("target");

    request->send(200, "text/plain", "update request - score:" + score + \
                                      " overs:" + overs + " wickets:" + \
                                      wicket + " target:" + target);
    Serialprintln("update complete");
  });
  server.onNotFound(handleServeFile);  // any other url
  server.begin();
//-------------------------------------------------------------
  // fastLED initialisation
  FastLED.addLeds<CHIPSET, DATA_PIN, RGB_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness( BRIGHTNESS );
  pinMode(LED_BUILTIN, OUTPUT);

  int buff[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS; i++)
  {
    //turn on led
    buff[i] = i*2%2;
  }
  for (int i = 0; i < NUM_LEDS; i++)
  {
    (buff[i]==1) ? leds[i] = CRGB::Green : leds[i] = CRGB::Black;
  }

}
///////////////////////////////////////////////////////////////////////////////
void loop() {

  leds[0] = CRGB::Red;
  leds[1] = CRGB::Green;
  leds[2] = CRGB::Blue;
  leds[3] = CRGB::Black;
  FastLED.show();

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  Serialprintln(leds[2]);

}