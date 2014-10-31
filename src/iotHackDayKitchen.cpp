#include <Arduino.h>
//#include "iotHackDayKitchen.h"
#include "Adafruit_NeoPixel.h"
#include "LPD8806.h"
#include <Servo.h>
#include "SimpleTimer.h"
#include "SPI.h"

int ledTimerId = 0;
uint8_t ledIndex = 0;
uint16_t ledTimeout = 1000;

SimpleTimer timer;
int timerId = 0;
uint16_t timeoutTime = 10000;
uint8_t success = 0;
uint8_t failed = 0;

Servo clockServo;
int clockPos = 0;    // variable to store the servo position

// 0 - 1024
const uint16_t kPotOffMin = 0;
const uint16_t kPotOffMax = 200;

const uint16_t kPotMedMin = 400;
const uint16_t kPotMedMax = 600;

const uint16_t kPotHotMin = 850;
const uint16_t kPotHotMax = 1200;

int potPinL = A0;    // select the input pin for the potentiometer
int potPinR = A1;    // select the input pin for the potentiometer
int valL = 0;       // variable to store the value coming from the sensor
int valR = 0;       // variable to store the value coming from the sensor

int waterTimerId = 0;
uint16_t waterTimeout = 2500;
int waterSwitchPin = 7;
int waterOn = 0;
bool waterHot = false;
bool waterFilled = false;

int waterLEDPin = 11;
Adafruit_NeoPixel waterLEDStrip = Adafruit_NeoPixel(3, waterLEDPin, NEO_GRB + NEO_KHZ800);

uint8_t touchOnePin = 14;
uint8_t touchTwoPin = 15;
uint8_t touchThreePin = 16;
uint8_t touchFourPin = 17;

const uint8_t kNoWater = 0;
const uint8_t kWater = 1;
const uint8_t kStoveCold = 0;
const uint8_t kStoveMedium = 1;
const uint8_t kStoveHot = 2;

const uint8_t kStepStateStart = 0;
const uint8_t kStepStateLoop = 1;
const uint8_t kStepStateEnd = 2;
const uint8_t kStepStateWait = 3;

// LED Indexes
const uint8_t kStoveLEDIndex = 0;
const uint8_t kFaucetLEDIndex = 1;

const uint8_t kCarrotLEDIndex = 3;
const uint8_t kEggplantLEDIndex = 4;
const uint8_t kCucumberLEDIndex = 5;
const uint8_t kPotatoLEDIndex = 6;
const uint8_t kPepperLEDIndex = 7;
const uint8_t kCornLEDIndex = 8;
const uint8_t kStatusLEDIndex = 9;

uint8_t veggies[] = { kCarrotLEDIndex, kEggplantLEDIndex, kCucumberLEDIndex, kPotatoLEDIndex, kPepperLEDIndex, kCornLEDIndex };
//uint8_t veggies[] = { 6, 13, 5, 10, 9, 11 };

uint8_t stoveTemp = 0;
uint8_t useWater = 0;
uint8_t veggieOne = 0;
uint8_t veggieTwo = 0;
uint8_t veggieThree = 0;
uint8_t veggieFour = 0;

uint8_t gameStep = 0;
uint8_t stepState = 0;

int statusDataPin  = 23;
int statusClockPin = 22;
LPD8806 statusStrip = LPD8806(10, statusDataPin, statusClockPin);

void shuffle(uint8_t *aArray, uint8_t aElements);
void succeeded();
void timedOut();
void ledTimedOut();
void waterTimedOut();
void colorWipe(Adafruit_NeoPixel strip, uint32_t c);
void statusColorWipe(LPD8806 strip, uint32_t c);
void theaterChase(Adafruit_NeoPixel strip, uint32_t c, uint8_t wait);

// touch
uint8_t oldTouches = 0;
uint8_t touches = 0;
uint8_t filterOne = 0;
uint8_t filterTwo = 0;
uint8_t filterThree = 0;
uint8_t filterFour = 0;
uint16_t timestamp = 0;
bool identified = false;

#define NOT_TOUCHING 0
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

uint8_t veggieLEDToValue(uint8_t aVegLEDIndex);
void touchLoop();
bool checkVeggies(uint8_t aVegLEDIndex);
void turnOffVeggieLeds();
void printBinary(uint8_t aValue);

void setup() {
  Serial.begin(9600);

  randomSeed(analogRead(A0) + analogRead(A1));

  stoveTemp = random(1, 3);
  Serial.print("Stove Temp: ");
  Serial.println(stoveTemp);
  useWater = random(2);
  Serial.print("Use water: ");
  Serial.println(useWater);
  //veggieOne = random(3, 9);
  //veggieTwo = random(3, 9);
  //veggieThree = random(3, 9);
  //veggieFour = random(3, 9);
  shuffle(veggies, 6);

  clockServo.attach(9);

  pinMode(potPinL, INPUT);
  pinMode(potPinR, INPUT);

  pinMode(waterSwitchPin, OUTPUT);

  pinMode(touchOnePin, INPUT);
  pinMode(touchTwoPin, INPUT);
  pinMode(touchThreePin, INPUT);
  pinMode(touchFourPin, INPUT);

  timerId = timer.setTimeout(timeoutTime, timedOut);
  ledTimerId = timer.setTimeout(ledTimeout, ledTimedOut);
  waterTimerId = timer.setTimeout(waterTimeout, waterTimedOut);
  //timer.disable(timerId);
  //timer.disable(ledTimerId);
  //timer.disable(waterTimerId);

  waterLEDStrip.begin();
  waterLEDStrip.show();
  colorWipe(waterLEDStrip, waterLEDStrip.Color(0, 0, 255));

  statusStrip.begin();
  statusStrip.show();
  //statusColorWipe(statusStrip, waterLEDStrip.Color(0, 0, 255));
}

void loop() {
  timer.run();
  touchLoop();
  if (gameStep == 0) {
    if (stepState == kStepStateStart) {
      Serial.println("STOVE");
      timerId = timer.setTimeout(timeoutTime, timedOut);
      if (stoveTemp == kStoveMedium) {
        statusStrip.setPixelColor(kStoveLEDIndex, statusStrip.Color(127, 0, 127));
      } else if (stoveTemp == kStoveHot) {
        statusStrip.setPixelColor(kStoveLEDIndex, statusStrip.Color(127, 0, 0));
      }
      statusStrip.show();
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      uint16_t potLVal = analogRead(potPinL);
      //Serial.println(potLVal);
      if (stoveTemp == kStoveMedium) {
        if ((potLVal >= kPotMedMin) && (potLVal <= kPotMedMax)) {
          Serial.print("Medium: ");
          Serial.println(potLVal);
          succeeded();
        }
      } else if (stoveTemp == kStoveHot) {
        if ((potLVal >= kPotHotMin) && (potLVal <= kPotHotMax)) {
          Serial.print("Hot: ");
          Serial.println(potLVal);
          succeeded();
        }
      }

    } else if (stepState == kStepStateEnd) {
      statusStrip.setPixelColor(kStoveLEDIndex, statusStrip.Color(0, 0, 0));
      statusStrip.show();
      stepState = kStepStateStart;
      gameStep++;
    }

  } else if (gameStep == 1) {
    if (stepState == kStepStateStart) {
      timerId = timer.setTimeout(timeoutTime, timedOut);
      if (useWater) {
        Serial.println("WATER");
        statusStrip.setPixelColor(kFaucetLEDIndex, statusStrip.Color(0, 127, 0));
      } else {
        Serial.println("VEGGIE 1");
        statusStrip.setPixelColor(veggies[0], statusStrip.Color(0, 0, 127));
      }
      statusStrip.show();
      stepState =  kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (useWater) {
        uint8_t oldValue = waterOn;
        waterOn = digitalRead(waterSwitchPin);

        if (waterOn && (waterOn != oldValue)) {
          Serial.println("Water turned on.");
          waterTimerId = timer.setTimeout(waterTimeout, waterTimedOut);
          theaterChase(waterLEDStrip, waterLEDStrip.Color(0, 0, 127), 0); // Blue
        }
        if (waterOn && !waterHot && (touches > 0)) {
          Serial.println("Cold water filled!");
          waterFilled = true;
        }
        if (!waterOn && waterFilled && (waterOn != oldValue)) {
          Serial.println("Water turned off!");
          succeeded();
        }
      } else {
        if (checkVeggies(veggieLEDToValue(veggies[0]))) {
          succeeded();
        } else {
          timedOut();
        }
      }
    } else if (stepState == kStepStateEnd) {
      //timer.disable(waterTimerId);
      waterFilled = false;
      waterHot = false;
      if (useWater) {
        statusStrip.setPixelColor(kFaucetLEDIndex, statusStrip.Color(0, 0, 0));
      } else {
        statusStrip.setPixelColor(veggies[0], statusStrip.Color(0, 0, 0));
      }
      statusStrip.show();
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 2) {
    if (stepState == kStepStateStart) {
      Serial.println("VEGGIE 2");
      timerId = timer.setTimeout(timeoutTime, timedOut);
      statusStrip.setPixelColor(veggies[1], statusStrip.Color(0, 0, 127));
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (checkVeggies(veggieLEDToValue(veggies[1]))) {
        succeeded();
      } else {
        timedOut();
      }
    } else if (stepState == kStepStateEnd) {
      statusStrip.setPixelColor(veggies[1], statusStrip.Color(0, 0, 0));
      statusStrip.show();
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 3) {
    if (stepState == kStepStateStart) {
      Serial.println("VEGGIE 3");
      statusStrip.setPixelColor(veggies[2], statusStrip.Color(0, 0, 127));
      timerId = timer.setTimeout(timeoutTime, timedOut);
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (checkVeggies(veggieLEDToValue(veggies[2]))) {
        succeeded();
      } else {
        timedOut();
      }
    } else if (stepState == kStepStateEnd) {
      statusStrip.setPixelColor(veggies[2], statusStrip.Color(0, 0, 0));
      statusStrip.show();
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 4) {
    if (stepState == kStepStateStart) {
      Serial.println("VEGGIE 4");
      timerId = timer.setTimeout(timeoutTime, timedOut);
      statusStrip.setPixelColor(veggies[3], statusStrip.Color(0, 0, 127));
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (checkVeggies(veggieLEDToValue(veggies[3]))) {
        succeeded();
      } else {
        timedOut();
      }
    } else if (stepState == kStepStateEnd) {
      statusStrip.setPixelColor(veggies[3], statusStrip.Color(0, 0, 0));
      statusStrip.show();
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 5) {
    if (stepState == kStepStateStart) {
      Serial.println("STOVE OFF");
      timerId = timer.setTimeout(timeoutTime, timedOut);
      statusStrip.setPixelColor(kStoveLEDIndex, statusStrip.Color(0, 127, 0));
      statusStrip.show();
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      uint16_t potLVal = analogRead(potPinL);
      //Serial.println(potLVal);
      if (potLVal <= kPotOffMax) {
        Serial.print("stove now off: ");
        Serial.println(potLVal);
        succeeded();
      }
    } else if (stepState == kStepStateEnd) {
      statusStrip.setPixelColor(kStoveLEDIndex, statusStrip.Color(0, 0, 0));
      statusStrip.show();
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 6) {
    timerId = timer.setTimeout(timeoutTime, timedOut);
    if (stepState == kStepStateStart) {
      Serial.println("HOT WATER");
      statusStrip.setPixelColor(kFaucetLEDIndex, statusStrip.Color(127, 0, 0));
      statusStrip.show();
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      uint8_t oldValue = waterOn;
      waterOn = digitalRead(waterSwitchPin);

      if (waterOn && (waterOn != oldValue)) {
        waterTimerId = timer.setTimeout(waterTimeout, waterTimedOut);
        theaterChase(waterLEDStrip, waterLEDStrip.Color(0, 0, 127), 0); // Blue
      }
      if (waterOn && waterHot && (touches > 0)) {
        Serial.println("Hot water filled!");
        waterFilled = true;
      }
      if (!waterOn && waterFilled && (waterOn != oldValue)) {
        succeeded();
      }
    } else if (stepState == kStepStateEnd) {
      //timer.disable(waterTimerId);
      waterFilled = false;
      waterHot = false;
      statusStrip.setPixelColor(kFaucetLEDIndex, statusStrip.Color(0, 0, 0));
      statusStrip.show();
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 7) {
    if (stepState == kStepStateStart) {
      Serial.print("Game over - Succeeded: ");
      Serial.print(success);
      Serial.print(", Failed: ");
      Serial.println(failed);
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {

      succeeded();
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      gameStep++;
    }

  }

  //valL = analogRead(potPinL);
  //valR = analogRead(potPinR);
  //Serial.println("Reading: " + String(valL) + " " + String(valR));

  /*clockServo.write(clockPos);
  clockPos++;
  if (clockPos > 180){
    clockPos = 0;
  }*/

  //waterOn = digitalRead(waterSwitchPin);
  //Serial.println("Water on: " + String(waterOn));

 // theaterChase(waterLEDStrip, waterLEDStrip.Color(0, 0, 127), 50); // Blue
 // theaterChase(waterLEDStrip, waterLEDStrip.Color(127, 0, 0), 50); // Red

  //delay(15);
}

void succeeded() {
  //timer.disable(timerId);
  success++;
  statusStrip.setPixelColor(kStatusLEDIndex, statusStrip.Color(0, 0, 127));
  statusStrip.show();
  ledIndex = gameStep;
  //timer.enable(ledTimerId);
  ledTimerId = timer.setTimeout(ledTimeout, ledTimedOut);
  identified = false;
  stepState = kStepStateWait;
}

void timedOut() {
  Serial.println("Event Timeout");
  failed++;
  identified = false;
  stepState = kStepStateWait;
  statusStrip.setPixelColor(kStatusLEDIndex, statusStrip.Color(127, 0, 0));
  statusStrip.show();
  ledTimerId = timer.setTimeout(ledTimeout, ledTimedOut);
  //timer.disable(timerId);
}

void shuffle(uint8_t *aArray, uint8_t aElements) {
    if (aElements > 1)
    {
        uint8_t i;
        for (i = 0; i < aElements - 1; i++)
        {
          uint8_t j = i + rand() / (RAND_MAX / (aElements - i) + 1);
          uint8_t t = aArray[j];
          aArray[j] = aArray[i];
          aArray[i] = t;
        }
    }
}

void waterTimedOut() {
  Serial.println("Water now HOT.");
  waterHot = true;
  theaterChase(waterLEDStrip, waterLEDStrip.Color(127, 0, 0), 0); // Red
}

void ledTimedOut() {
    statusStrip.setPixelColor(kStatusLEDIndex, statusStrip.Color(0, 0, 0));
    statusStrip.show();
    stepState = kStepStateEnd;
}

void colorWipe(Adafruit_NeoPixel strip, uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
  }
}

void statusColorWipe(LPD8806 strip, uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
  }
}

// THIS NEEDS TO BE A NON-BLOCKING FUNCTION, MEANING NO DELAYS.
// Must be rewritten to update over time in the main loop.
void theaterChase(Adafruit_NeoPixel strip, uint32_t c, uint8_t wait) {
  for (uint8_t j=0; j<10; j++) {  //do 10 cycles of chasing
    for (uint8_t q=0; q < 3; q++) {
      for (uint8_t i=0; i < waterLEDStrip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint8_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

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

//void checkVeggies();
//void printBinary(uint8_t aValue);

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
        }

        oldTouches = touches;
}

uint8_t veggieLEDToValue(uint8_t aVegLEDIndex) {
  uint8_t result = 0;
  switch (aVegLEDIndex) {
    case kCarrotLEDIndex:
      Serial.println("Need Carrot");
      result = kCarrot;
      break;
    case kEggplantLEDIndex:
      Serial.println("Need Eggplant");
      result = kEggplant;
      break;
    case kCucumberLEDIndex:
      Serial.println("Need Cucumber");
      result = kCucumber;
      break;
    case kPotatoLEDIndex:
      Serial.println("Need Potato");
      result = kPotato;
      break;
    case kPepperLEDIndex:
      Serial.println("Need Pepper");
      result = kPepper;
      break;
    case kCornLEDIndex:
      Serial.println("Need Corn");
      result = kCorn;
      break;
  }
  return result;
}

bool checkVeggies(uint8_t aVeggie) {
  //turnOffVeggieLeds();
  bool result = false;
    if (!identified && (touches == kCarrot)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setPixelColor(kCarrotLEDIndex, statusStrip.Color(0, 0, 127));
        Serial.println("CARROT");
        result = (aVeggie == kCarrot) ? true : false;
    } else if (!identified && (touches == kEggplant)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setPixelColor(kEggplantLEDIndex, statusStrip.Color(0, 0, 127));
        Serial.println("EGGPLANT");
        result = (aVeggie == kEggplant) ? true : false;
    } else if (!identified && (touches == kCucumber)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setPixelColor(kCucumberLEDIndex, statusStrip.Color(0, 0, 127));
        Serial.println("CUCUMBER");
        result = (aVeggie == kCucumber) ? true : false;
    } else if (!identified && (touches == kPotato)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setPixelColor(kPotatoLEDIndex, statusStrip.Color(0, 0, 127));
        Serial.println("POTATO");
        result = (aVeggie == kPotato) ? true : false;
    } else if (!identified && (touches == kPepper)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setPixelColor(kPepperLEDIndex, statusStrip.Color(0, 0, 127));
        Serial.println("PEPPER");
        result = (aVeggie == kPepper) ? true : false;
    } else if (!identified && (touches == kCorn)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setPixelColor(kCornLEDIndex, statusStrip.Color(0, 0, 127));
        Serial.println("CORN");
        result = (aVeggie == kCorn) ? true : false;
    }
    //statusStrip.show();
    return result;
}

void turnOffVeggieLeds() {
  statusStrip.setPixelColor(kCarrotLEDIndex, statusStrip.Color(0, 0, 0));
  statusStrip.setPixelColor(kEggplantLEDIndex, statusStrip.Color(0, 0, 0));
  statusStrip.setPixelColor(kCucumberLEDIndex, statusStrip.Color(0, 0, 0));
  statusStrip.setPixelColor(kPotatoLEDIndex, statusStrip.Color(0, 0, 0));
  statusStrip.setPixelColor(kPepperLEDIndex, statusStrip.Color(0, 0, 0));
  statusStrip.setPixelColor(kCornLEDIndex, statusStrip.Color(0, 0, 0));
  statusStrip.show();
}

void printBinary(uint8_t aValue) {
    printf("touch: %d, old touch: %d, 0x%d%d%d%d%d%d%d%d\n", touches, oldTouches,
        IsBitSet(aValue, 7), IsBitSet(aValue, 6),
        IsBitSet(aValue, 5), IsBitSet(aValue, 4),
        IsBitSet(aValue, 3), IsBitSet(aValue, 2),
        IsBitSet(aValue, 1), IsBitSet(aValue, 0));
}