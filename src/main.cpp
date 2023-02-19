/* Scoreboard Controller
    based on ESP8266 and WS2815 LED strip
    01-01-2022   
*/
#define FASTLED_INTERRUPT_RETRY_COUNT 1

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include "LittleFS.h"
#include <FastLED.h>
#include <WEMOS_DHT12.h>
#include "ESPClock.h"
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
  return "text/plain";
} // end of getContentType
///////////////////////////////////////////////////////////////////////////////
void updateLEDs() {
  int16_t index, ledNo; 
  CRGB onColour = BLANK_COLOUR;
  //Serialprintf("\nxxxxxxxxxxxxxxx\n");
  //Serialprintf("Free heap:%i bytes in updateLEDs\tfragmentation:%i%%\n", ESP.getFreeHeap(), ESP.getHeapFragmentation());

  FastLED.setBrightness(brightness);
  for (uint8_t charNo = 0; charNo < sizeof(buffchr)-1; charNo++) {  // cycle through characters in buffchar
    onColour = (charNo < 4) ? TIME_COLOUR : SCORE_COLOUR;            // set time/score colour
    if(buffchr[2] == deg) onColour = TEMP_COLOUR;                   // set temperature colour
    if((index = buffchr[charNo] - ASCII_ZERO) < 0) index = 10;      // ascii to decimal, adjust space character
    //Serialprintf("\n\nChar No: %i Character: %i colour:%3i:%3i:%3i\n", charNo, index, onColour.r, onColour.g, onColour.b);
    for (uint8_t segNo = 0; segNo < SEGMENTS; segNo++)              // cycle through segments in character
    {
      ledNo = char_mapping_LED[charNo] + (segNo * LEDS_IN_SEGMENT);
      leds(ledNo, ledNo + LEDS_IN_SEGMENT - 1) = ((seg_mapping_LED[index] >> segNo) & 0b00000001) ? onColour : BLANK_COLOUR;
      //Serialprintf("seg:%i led:%3i val:%3i\t", segNo, ledNo, leds[ledNo].g);
    }
  }
 //  print full led array
  // Serialprintf("\n");
  // for (int i = 0; i < NUM_LEDS; i++) {
  //   if(i%6 == 0) Serialprintf("\n");
  //   Serialprintf("led[%i] %3i\t", i, leds[i].g);
  // }
} // end of updateLEDs
///////////////////////////////////////////////////////////////////////////////
bool updateTemperature() {
  if(dht12.get() != 0) return false;
  uint8_t degrees = dht12.cTemp;
  buffchr[0] = (degrees < 10) ? ' ': degrees/10 + ASCII_ZERO;
  buffchr[1] = degrees%10 + ASCII_ZERO;
  buffchr[2] = deg;
  buffchr[3] = C;
  return true;
}
///////////////////////////////////////////////////////////////////////////////
bool updateTime() {
  if(!myClock.isSet()) return false;
  uint8_t hours = myClock.getHours();
  uint8_t minutes  = myClock.getMinutes();
  buffchr[0] = (hours < 10) ? ' ': 1 + ASCII_ZERO;
  buffchr[1] = hours%10 + ASCII_ZERO;
  buffchr[2] = minutes/10 + ASCII_ZERO;
  buffchr[3] = minutes%10 + ASCII_ZERO;
  return true;
}
///////////////////////////////////////////////////////////////////////////////
void handleServeFile(AsyncWebServerRequest *request) {
  String url = request->url();
  if(url == "/") url = "/scoreboard.html";          // default page
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
  uint32_t baseSeconds = (request->arg("seconds")).toInt();
  myClock.setTime(baseSeconds);
  brightness  = map(request->arg("brightness").toInt(), 0, 10, 0, 255);
  snprintf (buffchr + 4, sizeof(buffchr), PSTR("%2s%3s%2s%3s"), 
            request->arg(F("wicket")), request->arg(F("score")),
            request->arg(F("overs")),  request->arg(F("target"))
            );
  updateTime();
  updateLEDs();

  char response[144];       // response max chars 139
  const char style[] = "<section style='font-family:verdana;font-size:1em;'><p>Last update: ";
  snprintf (response, sizeof(response),
    PSTR("%sTime:%s<br>score:%s overs:%s wickets:%s target:%s</p></section>"),
            style, myClock.getTime().c_str(),
            request->arg("score"), request->arg("overs"),
            request->arg("wicket"), request->arg("target")
            );
  request->send(200, "text/html",  response);
  Serialprintf("\nupdate complete: %s\n", response);
} // end of handleUpdate
///////////////////////////////////////////////////////////////////////////////
void LEDStartup(){
  static uint8_t hue = 0;
  FastLED.showColor(CHSV(hue, 255, 127));
  //Serialprintf("Startup hue:%i\n", hue);
  hue = (hue + 5) % 255;
}
///////////////////////////////////////////////////////////////////////////////
void scheduler() {
  static uint8_t schedCount;
  static CRGB pulseColour = PULSE_COLOUR;
  switch (schedCount) {
    case 0:
      if(updateTemperature()){
        updateLEDs();
        pulseColour = BLANK_COLOUR;
      } 
      break;
    case 10:
      if(updateTime()) {
        updateLEDs();
        pulseColour = PULSE_COLOUR;
      } 
      break;
  }

  if(!myClock.isSet()) {
    LEDStartup();
  } 
  else {
    leds(PULSE, PULSE + PULSE_WIDTH) = (schedCount % 2 == 0) ? pulseColour : BLANK_COLOUR;  // pulse the clock :
    //Serialprintf("pulse:%i\n", leds[PULSE].g);
    FastLED.show();                           // update display every 500 millis
    FastLED.delay(10);                        // shouldn't be necessary
  }

  //Serialprintf("Frames:%i\n", FastLED.getFPS());
  schedCount++;
  schedCount %= 60;                           // cycle every 30 seconds             
}
///////////////////////////////////////////////////////////////////////////////
void setup_FileSystem() {
  LittleFS.begin();                 // start LittleFS file system            
  #if DEBUG_STATE
    Dir dir = LittleFS.openDir("/");
    Serialprintln("\nLittleFS File System:");
    while (dir.next()) {
      Serialprintf(" File: %-28s size: %8u bytes\n",
                  dir.fileName().c_str(), dir.fileSize());
    }
    Serialprintf("file system setup\n");
  #endif
}
///////////////////////////////////////////////////////////////////////////////
void setup_FastLED() {
  // fastLED initialisation
  FastLED.addLeds<CHIPSET, DATA_PIN1, RGB_ORDER>(leds, STRIP1_START, NUM_LEDS_CLOCK);
  FastLED.addLeds<CHIPSET, DATA_PIN2, RGB_ORDER>(leds, STRIP2_START, LEDS_PER_STRIP);
  FastLED.addLeds<CHIPSET, DATA_PIN3, RGB_ORDER>(leds, STRIP3_START, LEDS_PER_STRIP);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTAGE, LED_CURRENT);
  FastLED.setBrightness(brightness);
  FastLED.clear(true);
  FastLED.showColor(BLANK_COLOUR);
  Serialprintf("FastLED Display setup\n");
}
///////////////////////////////////////////////////////////////////////////////
void setup_WiFi() {
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, password, CHANNEL);    // try STA mode
  // int status = WiFi.waitForConnectResult(5000);
  // Serialprintf("WiFi Status:%i\n", status);
  // if (status != WL_CONNECTED) {  // can't join network so start AP
  //  Serialprintf("ssid not available - switch to AP mode\n");
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(server_IP, gateway, subnet);
    WiFi.softAP(ssid, password, CHANNEL);
    // Access Point mode
    Serialprintf("WiFi Access Point established! IP address: %s on %s\n", \
                  WiFi.softAPIP().toString().c_str(), ssid);
  // } else {       // Station mode 
  //   Serialprintf("\nStation mode started\nIP Address: %s on %s\n", WiFi.localIP().toString().c_str(), ssid);
  // }
}
///////////////////////////////////////////////////////////////////////////////
void setup_Server() {
  server.on("/scoreboardupdate", HTTP_GET, handleUpdate);
  server.onNotFound(handleServeFile);       // any other url
  server.begin();
}
///////////////////////////////////////////////////////////////////////////////
void setup() {
  Serialbegin(115200);
  setup_FastLED();                      // fastLED initialisation
  setup_FileSystem();                   // start LittleFS file system
  setup_WiFi();                         // Start WiFi
  if(WiFi.getMode() == WIFI_AP) AsyncElegantOTA.begin(&server); // Start AsyncElegantOTA
  setup_Server();                       // Start web server
}
///////////////////////////////////////////////////////////////////////////////
void loop() {  
  EVERY_N_MILLIS(SCHED_INT) scheduler();
}
