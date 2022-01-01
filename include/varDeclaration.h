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
#define NUM_LEDS        594
#define LEDS_IN_SEGMENT 6
#define SEGMENTS        7
#define BRIGHTNESS      50

CRGBArray<NUM_LEDS>     leds;
const CRGB              C_ON   = CRGB::Yellow;
const CRGB              C_OFF  = CRGB::Black;
CRGB                    colour;
AsyncWebServer          server(80);

const uint8_t seg_mapping[16][7] = {
//   A B C D E F G 
    {1,1,1,1,1,1,0},    // 0
    {0,1,1,0,0,0,0},    // 1
    {1,1,0,1,1,0,1},    // 2
    {1,1,1,1,0,0,1},    // 3
    {0,1,1,0,0,1,1},    // 4
    {1,0,1,1,0,1,1},    // 5
    {1,0,1,1,1,1,1},    // 6
    {1,1,1,0,0,0,0},    // 7
    {1,1,1,1,1,1,1},    // 8
    {1,1,1,1,0,1,1},    // 9
    {0,0,0,0,0,0,0},    // ' ' blank
    {0,0,0,0,0,0,1},    // - hyphen
    {1,1,1,0,1,1,1},    // A
    {1,0,0,1,1,1,0},    // C
    {1,0,0,0,1,1,1},    // F
    {1,1,0,0,0,1,1}     // ° degrees
};
char          buffchr[11];            // character buffer for character display
char          bufftime[5];            // character buffer for time display
uint32_t      baseSeconds = 0;        // time stamp in seconds
uint32_t      baseMillis  = 0;        // time stamp in millis
uint16_t      schedInt    = 500;      // scheduler time interval (ms)
uint8_t       schedCount  = 0;        // scheduler counter

