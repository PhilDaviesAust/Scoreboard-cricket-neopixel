#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <FastLED.h>
#include "credentials.h"
#include "varDeclaration.h"
//#include "TimeKeeper.h"

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

#define DATA_PIN        2
#define RGB_ORDER       GRB
#define CHIPSET         WS2812
#define NUM_LEDS        594
#define LEDS_IN_SEGMENT 6
#define SEGMENTS        7
#define BRIGHTNESS      50

CRGBArray<NUM_LEDS> leds;
const CRGB Green = CRGB::Green;
const CRGB Black = CRGB::Black;

//TimeKeeper tk;

AsyncWebServer server(80);
int blink;

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
  else return String(bytes/1024.0)+"KB";
} // end of formatBytes
///////////////////////////////////////////////////////////////////////////////
void updateTime(){
  int indx, ledNo, offset= 10 * SEGMENTS * LEDS_IN_SEGMENT;
  CRGB colour;
  uint32_t now = ((millis() - baseMillis)/1000) + baseSeconds;
  uint8_t hours = (now/3600) % 12;
  uint8_t minutes = (now/60) % 60;
  //uint8_t seconds = now % 3600;
  snprintf(bufftime, sizeof(bufftime), PSTR("%2u%02u"),hours, minutes);
  for (uint8_t charNo = 0; charNo < 4; charNo++){   // cycle through characters in bufftime
    if((indx = (int)bufftime[charNo]-48) < 0) indx = 10;
    //Serialprintf("Character: %i\n", indx);
    for (int segNo = 0; segNo < SEGMENTS; segNo++)  // cycle through segments in character
    {
      if(seg_mapping[indx][segNo]) colour = Green; else colour = Black;
      ledNo = charNo * SEGMENTS * LEDS_IN_SEGMENT + segNo * LEDS_IN_SEGMENT + offset;
      leds(ledNo, ledNo+5) = colour;
      //Serialprintf("char: %i seg: %i led: %i value: %i\n", charNo, segNo, ledNo, colour.g);
    }
  } 
}
///////////////////////////////////////////////////////////////////////////////
void updateLEDs(){
  int indx, ledNo;
  CRGB colour;
  for (uint8_t charNo = 0; charNo < sizeof(buffchr)-1; charNo++) {    // cycle through characters in buffchar
    if((indx = (int)buffchr[charNo]-48) < 0) indx = 10;
    Serialprintf("Character: %i\n", indx);
    for (int segNo = 0; segNo < SEGMENTS; segNo++)                // cycle through segments in character
    {
      if(seg_mapping[indx][segNo]) colour = Green; else colour = Black;
      ledNo = charNo * SEGMENTS * LEDS_IN_SEGMENT + segNo * LEDS_IN_SEGMENT;
      leds(ledNo, ledNo+5) = colour;
      Serialprintf("char: %i seg: %i led: %i value: %i\n", charNo, segNo, ledNo, colour.g);
    }
  }
  for (int i = 0; i < NUM_LEDS; i++)
  {
    if(i%42 == 0) Serialprintf("\ncharacter %i\n", i/42);
    if(i%6 == 0) Serialprintln(i);
    Serialprintf("led[%i] %i\n", i, leds[i].g);
  }
  
  FastLED.show();     // TODO don't want to do this here scheduler should take care of updates
} // end of updateLEDs
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
} // end of handleServeFile
///////////////////////////////////////////////////////////////////////////////
void handleUpdate( AsyncWebServerRequest *request){
  String brightness = request->arg("brightness");

  snprintf (buffchr, sizeof(buffchr), PSTR("%3s%3s%2s%2s"), 
            request->arg(F("score")),
            request->arg(F("target")),
            request->arg(F("overs")),
            request->arg(F("wicket")));
  
  int start = millis();
  updateLEDs();
  int finish = millis() - start;

  request->send(200, "text/plain",  "score:" + request->arg(F("score")) + \
                                    " overs:" + request->arg(F("overs")) + \
                                    " wickets:" + request->arg(F("wicket")) + \
                                    " target:" + request->arg(F("target")) + \
                                    " updatetime:" + finish);
  Serialprintln("update complete");
} // end of handleUpdate
///////////////////////////////////////////////////////////////////////////////
void handleTimeUpdate(AsyncWebServerRequest *request){
  baseSeconds = (request->arg("seconds")).toInt();
  baseMillis = millis();
  updateTime();
  Serialprintf("time updated %i, %i\n", baseSeconds, baseMillis);
  request->send(200, "text/plain", "time updated");
} // end of handleTimeUpdate
///////////////////////////////////////////////////////////////////////////////
void setup() {
  Serialbegin(115200);
//-------------------------------------------------------------
  // fastLED initialisation
  FastLED.addLeds<CHIPSET, DATA_PIN, RGB_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness( BRIGHTNESS );
  FastLED.clear();
  FastLED.show();
  pinMode(LED_BUILTIN, OUTPUT);
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
  server.on("/update", HTTP_GET, handleUpdate);
  server.on("/time", HTTP_GET, handleTimeUpdate);
  server.onNotFound(handleServeFile);       // any other url
  server.begin();
} // end of setup
///////////////////////////////////////////////////////////////////////////////
void loop() {
  EVERY_N_MILLIS (500) {
    blink++;
    digitalWrite(LED_BUILTIN, blink%2);
  };
}