
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
uint8_t       schedInt    = 1;        // scheduler time interval
uint8_t       schedCount  = 0;        // scheduler counter
uint8_t       pwmDutyCycle = 70;      // PWM frequency (0 - 100)
