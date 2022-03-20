/* Scoreboard Controller
    based on ESP8266 and WS2815 LED strip
    01-01-2022   
*/
#define FASTLED_INTERRUPT_RETRY_COUNT 1

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <FastLED.h>
#include "credentials.h"
#include "varDeclaration.h"
///////////////////////////////////////////////////////////////////////////////
String getContentType(String url) {
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
String formatBytes(size_t bytes) {
  if (bytes < 1024) return String(bytes)+"B";
  else return String(bytes/1024.0)+"KB";
} // end of formatBytes
///////////////////////////////////////////////////////////////////////////////
void updateLEDs() {
  int16_t indx, ledNo; 
  Serialprintf("\nxxxxxxxxxxxxxxx\n");
  //Serialprintf("Free heap:%i bytes in updateLEDs\tfragmentation:%i%%\n", ESP.getFreeHeap(), ESP.getHeapFragmentation());

  FastLED.setBrightness(brightness);
  for (uint8_t charNo = 0; charNo < sizeof(buffchr)-1; charNo++) {  // cycle through characters in buffchar
    if((indx = buffchr[charNo] - ASCII_ZERO) < 0) indx = 10;        // ascii to decimal, adjust space character
    //Serialprintf("\n\nChar No: %i Character: %i\n", charNo, indx);
    for (uint8_t segNo = 0; segNo < SEGMENTS; segNo++)              // cycle through segments in character
    {
      ledNo = led_mapping[charNo] + (segNo * LEDS_IN_SEGMENT);
      leds(ledNo, ledNo + LEDS_IN_SEGMENT - 1) = (seg_mapping[indx][segNo]) ? C_ON : C_OFF;
      //Serialprintf("seg:%i led:%3i val:%3i\t", segNo, ledNo, leds[ledNo].g);
    }
  }
 //  print full led array
  // Serialprintf("\n");
  // for (int i = 0; i < NUM_LEDS; i++) {
  //   if(i%42 == 0) Serialprintf("\n\ncharacter %i = '%c'\n", i/42, buffchr[i/42]);
  //   if(i%6 == 0) Serialprintf("\n");
  //   Serialprintf("led[%i] %3i\t", i, leds[i].g);
  // }
} // end of updateLEDs
///////////////////////////////////////////////////////////////////////////////
void updateTime() {
  uint32_t now = ((millis() - baseMillis)/1000) + baseSeconds;
  hours = (now/3600) % 24;
  if(hours != 12) hours = hours % 12;
  minutes = (now/60) % 60;
  buffchr[10] = (hours < 10) ? ' ': hours/10 + ASCII_ZERO;
  buffchr[11] = hours%10 + ASCII_ZERO;
  buffchr[12] = minutes/10 + ASCII_ZERO;
  buffchr[13] = minutes%10 + ASCII_ZERO;
  updateLEDs();
} // end of updateTime
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
void handleUpdate(AsyncWebServerRequest *request) {
  baseMillis  = millis();
  baseSeconds = (request->arg("seconds")).toInt();
  brightness  = map(request->arg("brightness").toInt(), 0, 10, 0, 255);
  snprintf (buffchr, sizeof(buffchr), PSTR("%3s%3s%2s%2s%2u%02u"), 
            request->arg(F("score")), request->arg(F("target")),
            request->arg(F("overs")), request->arg(F("wicket")),
            hours, minutes);
  updateLEDs();
  
  String response = style +
    "score:"    + request->arg("score")  + " overs:"  + request->arg("overs") +
    " wickets:" + request->arg("wicket") + " target:" + request->arg("target") +
    "</p></section>";
  request->send(200, "text/html",  response);
  Serialprintf("\nupdate complete\n");
} // end of handleUpdate
///////////////////////////////////////////////////////////////////////////////
void scheduler() {
  schedCount++;

  if(schedCount == 1) updateTime();         // update the time every 30 seconds

  leds(PULSE, PULSE+PULSE_WIDTH) = (schedCount%2 == 0) ? C_ON: C_OFF;  // pulse the clock :

  FastLED.show();                           // update display every 500 millis
  FastLED.delay(2);
  Serialprintf("Frames:%i\n", FastLED.getFPS());

  schedCount %= 60;                         // cycle every 30 seconds
} // end of scheduler
///////////////////////////////////////////////////////////////////////////////
void setup() {
  Serialbegin(115200);
//-------------------------------------------------------------
  // fastLED initialisation
  FastLED.addLeds<CHIPSET, DATA_PIN1, RGB_ORDER>(leds, 0, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<CHIPSET, DATA_PIN2, RGB_ORDER>(leds, 174, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<CHIPSET, DATA_PIN3, RGB_ORDER>(leds, 384, NUM_LEDS_PER_STRIP);
  FastLED.setMaxPowerInVoltsAndMilliamps(12, 1000);
  FastLED.setBrightness(brightness);
  FastLED.clear(true);
  FastLED.showColor(C_OFF);
//-------------------------------------------------------------
  // start LittleFS file system
  LittleFS.begin();               
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
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password, CHANNEL);    // try STA mode
  int status = WiFi.waitForConnectResult(6000);
  Serialprintf("WiFi Status:%i\n", status);
  if (status != WL_CONNECTED) {  // can't join network so start AP
    Serialprintf("ssid not available - switch to AP mode\n");
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(server_IP, gateway, subnet);
    WiFi.softAP(ssid, password, CHANNEL);
    masterNode = true;                            // Access Point mode
    Serialprintf("WiFi Access Point established! IP address: %s on %s\n", \
                  WiFi.softAPIP().toString().c_str(), ssid);
  } else {
    masterNode = false;                           // Station mode
    Serialprintf("\nStation mode started\nIP Address: %s on %s\n", \
                  WiFi.localIP().toString().c_str(), ssid);
  }
//-------------------------------------------------------------
  // Start web server
  server.on("/update", HTTP_GET, handleUpdate);
  server.onNotFound(handleServeFile);       // any other url
  server.begin();
} // end of setup
///////////////////////////////////////////////////////////////////////////////
void loop() {  
  EVERY_N_MILLIS (schedInt) {scheduler();}
}