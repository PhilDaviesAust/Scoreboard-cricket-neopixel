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

#define SCL_PIN         D1      // DHT12 clock pin (GPIO5)
#define SDA_PIN         D2      // DHT12 data pin (GPIO4)
#define DATA_PIN1       D5
#define DATA_PIN2       D6
#define DATA_PIN3       D7
#define RGB_ORDER       RGB
#define CHIPSET         WS2812B  // best match for WS2815
#define SEGMENTS        7
#define LEDS_IN_SEGMENT 6
#define NUM_STRIPS      3
#define LEDS_PER_STRIP  210
#define NUM_LEDS_CLOCK  174
#define NUM_LEDS        594     // 14 characters * 7 segments * 6 LEDs + 6 LEDs(colon)
#define STRIP1_START    0       // LED strip 1 starting LED number
#define STRIP2_START    174     // LED strip 2 starting LED number
#define STRIP3_START    384     // LED strip 3 starting LED number
#define PULSE           84      // clock pulse starting LED number
#define PULSE_WIDTH     5       // number of LEDs in pulse -1
#define LED_VOLTAGE     12      // LED strip voltage
#define LED_CURRENT     8000    // LED strip max current (milliamps)
#define CHANNEL         1
#define ASCII_ZERO      48
#define SCHED_INT       500     // scheduler time interval (ms)

#define PULSE_COLOUR    CRGB::YellowGreen
#define TIME_COLOUR     CRGB::YellowGreen
#define SCORE_COLOUR    CRGB::Aqua
#define TEMP_COLOUR     CRGB::OrangeRed
#define BLANK_COLOUR    CRGB::Black

CRGBArray<NUM_LEDS>     leds;
AsyncWebServer          server(80);
DHT12                   dht12;  // use default pins SCL=D1 SDA=D2
ESPClock                myClock;

const uint8_t seg_mapping_LED[13] = {   //used by FastLED
//  XGEDCBAF      Segments
  0b00111111,     // 0  Digit 0  ascii 48
  0b00001100,     // 1  Digit 1  ascii 49        AA
  0b01110110,     // 2  Digit 2  ascii 50      F    B
  0b01011110,     // 3  Digit 3  ascii 51      F    B
  0b01001101,     // 4  Digit 4  ascii 52        GG
  0b01011011,     // 5  Digit 5  ascii 53      E    C
  0b01111011,     // 6  Digit 6  ascii 54      E    C
  0b00001110,     // 7  Digit 7  ascii 55        DD
  0b01111111,     // 8  Digit 8  ascii 56
  0b01011111,     // 9  Digit 9  ascii 57
  0b00000000,     // 10 blank    ascii 32
  0b01000111,     // 11 ° degree ascii 59 ;
  0b00110011      // 12 C        ascii 60 <
};
const char    deg = ';';      // ascii 59
const char    C   = '<';      // ascii 60

const uint16_t char_mapping_LED[14] = {
  0,    // 0 hours * 10     temp * 10
  42,   // 1 hours * 1      temp * 1
  90,   // 2 minutes * 10   deg
  132,  // 3 minutes * 1    C
  174,  // 4 wicket * 10
  216,  // 5 wicket * 1
  258,  // 6 score * 100
  300,  // 7 score * 10
  342,  // 8 score * 1
  384,  // 9 overs * 10
  426,  // 10 overs * 1
  468,  // 11 target * 100
  510,  // 12 target * 10
  552   // 13 target * 1
};

char           buffchr[15];            // character buffer for character display

//                      hours  minutes  wicket   score        overs   target           
// Buffchr[15]	        H1	H2	M1	M2	 W1	W2	 S1 S2  S3	  O1	O2	T1	T2	T3	\0
// char_mapping_LED[14] 0   42	90	132	 174 216 258 300 342	384	426	468 510	552			
// PULSE                84

uint8_t        brightness  = 128;      // display brightness level
