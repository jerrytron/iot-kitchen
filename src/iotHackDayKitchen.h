
class Adafruit_NeoPixel;
class LPD8806;

void setup();
void loop();
void succeeded();
void timedOut();
void ledTimedOut();
void colorWipe(Adafruit_NeoPixel strip, uint32_t c);
void statusColorWipe(LPD8806 strip, uint32_t c);
void theaterChase(Adafruit_NeoPixel strip, uint32_t c, uint8_t wait);