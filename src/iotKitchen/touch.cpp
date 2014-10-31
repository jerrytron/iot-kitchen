/*#define NOT_TOUCHING 0
#define TOUCHING 1
#define ONE_INDEX 0
#define TWO_INDEX 1
#define THREE_INDEX 2
#define FOUR_INDEX 3

const uint16_t kSettleTime = 1000;
const uint8_t kSamples = 32;
const uint8_t kCarrot = 6;
const uint8_t kEggplant = 13;
const uint8_t kCucumber = 5;
const uint8_t kPotato = 10;
const uint8_t kPepper = 9;
const uint8_t kCorn = 11;

#define IsBitSet(val, bit) ((val) & (1 << (bit)) ? true : false)

//DigitalOut red(D5);
//DigitalOut blue(D8);
//DigitalOut green(D9);

//DigitalOut ledOne(LED2);
//DigitalOut ledTwo(LED3);
//DigitalOut ledThree(LED3);

//AnalogIn touchOne(A0);
//DigitalIn touchOne(D2, PullDown);
//DigitalIn touchTwo(D3, PullDown);
//DigitalIn touchThree(D4, PullDown);
//DigitalIn touchFour(D5, PullDown);

//Timer timer;

uint8_t oldTouches = 0;
uint8_t touches = 0;

uint8_t filterOne = 0;
uint8_t filterTwo = 0;
uint8_t filterThree = 0;
uint8_t filterFour = 0;

uint16_t timestamp = 0;

bool identified = false;

void checkVeggies();
void printBinary(uint8_t aValue);

void touchLoop() {
        //printBinary(touches);
        uint8_t oneRead = digitalRead(touchOnePin);
        if (oneRead == NOT_TOUCHING) {
            touches &= ~(1 << ONE_INDEX);
            filterOne = 0;
            //ledOne = 0;
        } else if (oneRead == TOUCHING) {
            //filterOne <<= 1;
            if (filterOne < kSamples) {
                filterOne++;
            }
            if (filterOne >= kSamples) {
                //printf("Onez: %d\n", filterOne);
                touches |= (1 << ONE_INDEX);
            }
        }
        
        uint8_t twoRead = digitalRead(touchTwoPin);
        if (twoRead == NOT_TOUCHING) {
            touches &= ~(1 << TWO_INDEX);
            //touches |= (0 << TWO_INDEX);
            filterTwo = 0;
            //ledOne = 0;
        } else if (twoRead == TOUCHING) {
            //filterTwo <<= 1;
            
            if (filterTwo < kSamples) {
                filterTwo++;
            }
            
            if (filterTwo >= kSamples) {
                //printf("Two\n");
                touches |= (1 << TWO_INDEX);
                //ledOne = 1;
            }
        }
        
        uint8_t threeRead = digitalRead(touchThreePin);
        if (threeRead == NOT_TOUCHING) {
            touches &= ~(1 << THREE_INDEX);
            //touches |= (0 << THREE_INDEX);
            filterThree = 0;
            //ledOne = 0;
        } else if (threeRead == TOUCHING) {
            //filterThree <<= 1;
            if (filterThree < kSamples) {
                filterThree++;
            }
            
            if (filterThree >= kSamples) {
                //printf("Three\n");
                touches |= (1 << THREE_INDEX);
                //ledOne = 1;
            }
        }
        
        uint8_t fourRead = digitalRead(touchFourPin);
        if (fourRead == NOT_TOUCHING) {
            touches &= ~(1 << FOUR_INDEX);
            //touches |= (0 << FOUR_INDEX);
            filterFour = 0;
            //ledOne = 0;
        } else if (fourRead == TOUCHING) {
            //filterFour <<= 1;
            if (filterFour < kSamples) {
                filterFour++;
            }
            
            if (filterFour >= kSamples) {
                //printf("Four\n");
                touches |= (1 << FOUR_INDEX);
            }
        }
        if (oldTouches != touches) {
            //printf("diff %d - %d\n", touches, oldTouches);
            identified = false;
        }

        if ((oldTouches == 0) &&
            (touches != 0)) {
            //printf("Timer start\n");
            timestamp = millis();        
        }
        //printf("time diff: %d\n", timer.read_ms());
        if ((touches != 0) &&
            ((millis() - timestamp) > kSettleTime)) {
            timestamp = 0;
            checkVeggies();
        }
        
        oldTouches = touches;
}

void checkVeggies() {
  //turnOffVeggieLeds();
    if (!identified && (touches == kCarrot)) {
        identified = true;
        //printf("CARROT\n");
        turnOffVeggieLeds();
        statusStrip.setPixelColor(3, statusStrip.Color(0, 0, 127));
        Serial.println("CARROT");
    } else if (!identified && (touches == kEggplant)) {
        identified = true;
        //printf("EGGPLANT\n");
        turnOffVeggieLeds();
        statusStrip.setPixelColor(4, statusStrip.Color(0, 0, 127));
        Serial.println("EGGPLANT");
    } else if (!identified && (touches == kCucumber)) {
        identified = true;
        //printf("CUCUMBER\n");
        turnOffVeggieLeds();
        statusStrip.setPixelColor(5, statusStrip.Color(0, 0, 127));
        Serial.println("CuCUMBER");
    } else if (!identified && (touches == kPotato)) {
        identified = true;
        //printf("POTATO\n");
        turnOffVeggieLeds();
        statusStrip.setPixelColor(6, statusStrip.Color(0, 0, 127));
        Serial.println("POTATO");
    } else if (!identified && (touches == kPepper)) {
        identified = true;
        //printf("PEPPER\n");
        turnOffVeggieLeds();
        statusStrip.setPixelColor(7, statusStrip.Color(0, 0, 127));
        Serial.println("PEPPER");
    } else if (!identified && (touches == kCorn)) {
        identified = true;
        //printf("CORN\n");
        turnOffVeggieLeds();
        statusStrip.setPixelColor(8, statusStrip.Color(0, 0, 127));
        Serial.println("CORN");
    }
    statusStrip.show();
}

void turnOffVeggieLeds() {
  statusStrip.setPixelColor(3, statusStrip.Color(0, 0, 0));
  statusStrip.setPixelColor(4, statusStrip.Color(0, 0, 0));
  statusStrip.setPixelColor(5, statusStrip.Color(0, 0, 0));
  statusStrip.setPixelColor(6, statusStrip.Color(0, 0, 0));
  statusStrip.setPixelColor(7, statusStrip.Color(0, 0, 0));
  statusStrip.setPixelColor(8, statusStrip.Color(0, 0, 0));
  statusStrip.show();
}

void printBinary(uint8_t aValue) {
    printf("touch: %d, old touch: %d, 0x%d%d%d%d%d%d%d%d\n", touches, oldTouches,
        IsBitSet(aValue, 7), IsBitSet(aValue, 6),
        IsBitSet(aValue, 5), IsBitSet(aValue, 4),
        IsBitSet(aValue, 3), IsBitSet(aValue, 2),
        IsBitSet(aValue, 1), IsBitSet(aValue, 0));
}*/
