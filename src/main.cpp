/* Scoreboard Controller
    based on ESP8266 and WS2815 LED strip
    01-01-2022   
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <FastLED.h>
#include "credentials.h"
#include "varDeclaration.h"
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
void updateLEDs(){
  int indx, ledNo;
  Serialprintf("\nxxxxxxxxxxxxxxx");
  for (uint8_t charNo = 0; charNo < sizeof(buffchr)-1; charNo++) {  // cycle through characters in buffchar
    if((indx = buffchr[charNo]-48) < 0) indx = 10;           // convert ascii to decimal, adjust space character
    //Serialprintf("\nCharacter: %i\n", indx);
    for (int segNo = 0; segNo < SEGMENTS; segNo++)           // cycle through segments in character
    {
      colour = (seg_mapping[indx][segNo]) ? C_ON : C_OFF;
      ledNo = led_mapping[charNo] + (segNo * LEDS_IN_SEGMENT);
      leds(ledNo, ledNo+5) = colour;
      //Serialprintf("char:%i seg:%i led:%i value:%i\n", charNo, segNo, ledNo, colour.g);
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
  //uint8_t buffsize = sizeof(bufftime); 
  uint32_t now = ((millis() - baseMillis)/1000) + baseSeconds;
  hours = (now/3600) % 24;
  if(hours != 12) hours = hours % 12;
  minutes = (now/60) % 60;
  buffchr[10] = (hours < 10) ? ' ': hours/10 + 48;
  buffchr[11] = hours%10 + 48;
  buffchr[12] = minutes/10 + 48;
  buffchr[13] = minutes%10 + 48;
  updateLEDs();
} // end of updateTime
///////////////////////////////////////////////////////////////////////////////
void updateScore(AsyncWebServerRequest *request) {
  uint8_t buffsize = sizeof(buffchr);
  snprintf (buffchr, buffsize, PSTR("%3s%3s%2s%2s%2u%02u"), 
            request->arg(F("score")), request->arg(F("target")),
            request->arg(F("overs")), request->arg(F("wicket")),
            hours, minutes);
  updateLEDs();
} // end of updateScore
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
  String brightness = request->arg("brightness"); //TODO do something with this
  baseSeconds = (request->arg("seconds")).toInt();
  baseMillis = millis();
  updateScore(request);
  String response = style + "score:" + request->arg("score") +
   " overs:" + request->arg("overs") +
   " wickets:" + request->arg("wicket") +
   " target:" + request->arg("target") +
   "</p></section>";
  request->send(200, "text/html",  response);
  Serialprintln("update complete");
} // end of handleUpdate
///////////////////////////////////////////////////////////////////////////////
void scheduler() {
  schedCount++;
  digitalWrite(LED_BUILTIN, schedCount%2);  // blink builtin LED 1Hz cycle

  if(schedCount == 1) updateTime();         // update the time every 30 seconds

  colour = (schedCount%2 == 0) ? C_ON : C_OFF;
  leds(pulse, pulse+5) = colour;            // pulse the clock :

  FastLED.show();                           // update display every 500 millis

  schedCount = schedCount % 60;
} // end of scheduler
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
  delay(200);
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
  // WiFi.mode(WIFI_AP);
  // WiFi.softAPConfig(local_IP, gateway, subnet);
  // bool apConnected = WiFi.softAP(ssid, password);
  // if (apConnected) {
  //     Serialprintf("WiFi IP: %s\n", WiFi.softAPIP().toString().c_str());
  // }
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
  server.onNotFound(handleServeFile);       // any other url
  server.begin();
} // end of setup
///////////////////////////////////////////////////////////////////////////////
void loop() {
  EVERY_N_MILLIS (schedInt) {scheduler();}
}