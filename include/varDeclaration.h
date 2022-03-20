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

#define DATA_PIN1       D5
#define DATA_PIN2       D6
#define DATA_PIN3       D7
#define RGB_ORDER       RGB
#define CHIPSET         WS2812  // best match for WS2815
#define SEGMENTS        7
#define LEDS_IN_SEGMENT 6
#define NUM_STRIPS      3
#define NUM_LEDS_PER_STRIP 210
#define NUM_LEDS        594     // 14 characters * 7 segments * 6 LEDs + 6 LEDs(colon)
#define PULSE           84      // clock pulse starting LED number
#define PULSE_WIDTH     5       // number of LEDs in pulse -1
#define CHANNEL         1
#define ASCII_ZERO      48

CRGBArray<NUM_LEDS>     leds;
const CRGB              C_ON   = CRGB::Green;
const CRGB              C_OFF  = CRGB::Black;
AsyncWebServer          server(80);

const uint8_t  seg_mapping[16][7] = {
// F A B C D E G        // Segments
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
  {0,0,0,0,0,0,1,},     // - hyphen
  {1,1,1,1,0,1,1,},     // A
  {1,1,0,0,1,1,0,},     // C
  {1,1,0,0,0,1,1,},     // F
  {1,1,1,0,0,0,1,}      // ° degree
};
// const uint16_t led_mapping[14] = {
//   0,    // 0 score * 100
//   42,   // 1 score * 10
//   84,   // 2 score * 1
//   126,  // 3 target * 100
//   168,  // 4 target * 10
//   210,  // 5 target * 1
//   252,  // 6 overs * 10
//   294,  // 7 overs * 1
//   336,  // 8 wicket * 10
//   378,  // 9 wicket * 1
//   420,  // 10 hours * 10
//   462,  // 11 hours * 1
//   504,  // 12 minutes * 10
//   546   // 13 minutes * 1
// };
const uint16_t led_mapping[14] = {
  258,    // 0 score * 100
  300,   // 1 score * 10
  342,   // 2 score * 1
  468,  // 3 target * 100
  510,  // 4 target * 10
  552,  // 5 target * 1
  384,  // 6 overs * 10
  426,  // 7 overs * 1
  174,  // 8 wicket * 10
  216,  // 9 wicket * 1
  0,    // 10 hours * 10
  42,   // 11 hours * 1
  90,   // 12 minutes * 10
  132   // 13 minutes * 1
};

//                 score      target      overs   wicket  hours   minutes
// Buffchr[15]	   S1	  S2	S3	T1	T2	T3	O1	O2	W1	W2	H1	H2	M1	M2	\0
// led_mapping[14] 258  300	342	468	510	552	384	426	174	216	0	  42	90	132			
// PULSE           84

bool           masterNode;
uint32_t       baseSeconds = 0;        // time stamp in seconds
uint32_t       baseMillis  = 0;        // time stamp in millis
uint8_t        hours       = 0;        // display time hours
uint8_t        minutes     = 0;        // display time minutes
uint8_t        brightness  = 128;      // display brightness level
uint16_t       schedInt    = 500;      // scheduler time interval (ms)
uint8_t        schedCount  = 0;        // scheduler counter
char           buffchr[15];            // character buffer for character display
static const String style = "<section style='font-family:verdana;font-size:12px;'><p>Last update:<br>";
