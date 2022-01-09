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

#define DATA_PIN        4
#define RGB_ORDER       GRB
#define CHIPSET         WS2812  // may need to be WS2813
#define SEGMENTS        7
#define LEDS_IN_SEGMENT 6
#define NUM_LEDS        594     // 14 characters * 7 segments * 6 LEDs + 6 LEDs(colon)
#define PULSE           588     // clock pulse starting LED number

CRGBArray<NUM_LEDS>     leds;
const CRGB              C_ON   = CRGB::Yellow;
const CRGB              C_OFF  = CRGB::Black;
CRGB                    colour;
AsyncWebServer          server(80);

const uint8_t  seg_mapping[16][7] = {
// F A B C D E G   
  {1,1,1,1,1,1,0,},     // Digit 0
  {0,0,1,1,0,0,0,},     // Digit 1
  {0,1,1,0,1,1,1,},     // Digit 2
  {0,1,1,1,1,0,1,},     // Digit 3
  {1,0,1,1,0,0,1,},     // Digit 4
  {1,1,0,1,1,0,1,},     // Digit 5
  {1,1,0,1,1,1,1,},     // Digit 6
  {0,1,1,1,0,0,0,},     // Digit 7
  {1,1,1,1,1,1,1,},     // Digit 8
  {1,1,1,1,1,0,1,},     // Digit 9
  {0,0,0,0,0,0,0,},     // blank
  {0,0,0,0,0,0,1,},     // hyphen
  {1,1,1,1,0,1,1,},     // A
  {1,1,0,0,1,1,0,},     // C
  {1,1,0,0,0,1,1,},     // F
  {1,1,1,0,0,0,1,}      // degree
};
const uint16_t led_mapping[14] = {
  0,    // 0 score
  42,   // 1
  84,   // 2
  126,  // 3 target
  168,  // 4
  210,  // 5
  252,  // 6 overs
  294,  // 7
  336,  // 8 wicket
  378,  // 9
  420,  // 10 hours
  462,  // 11
  504,  // 12 minutes
  546   // 13
};
uint32_t       baseSeconds = 0;        // time stamp in seconds
uint32_t       baseMillis  = 0;        // time stamp in millis
uint8_t        hours       = 0;        // display time hours
uint8_t        minutes     = 0;        // display time minutes
uint8_t        brightness  = 64;       // display brightness level
uint16_t       schedInt    = 500;      // scheduler time interval (ms)
uint8_t        schedCount  = 0;        // scheduler counter
char           buffchr[15];            // character buffer for character display
static const String style = "<section style='font-family:verdana;font-size:12px;'><p>Last update:<br>";
