#include <Arduino.h>
//#include "iotHackDayKitchen.h"
#include "Adafruit_NeoPixel.h"
#include <Servo.h>
#include "SimpleTimer.h"
#include "SPI.h"
#include "led_set.h"

int ledTimerId = -1;
uint16_t ledTimeout = 1000;

SimpleTimer timer;
int timerId = -1;
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

int waterTimerId = -1;
uint16_t waterTimeout = 4000;
int waterSwitchPin = 7;
int waterOn = 0;
bool waterHot = false;
bool waterFilled = false;

int waterLEDPin = 11;
//Adafruit_NeoPixel setOne = Adafruit_NeoPixel(3, waterLEDPin, NEO_GRB + NEO_KHZ800);
LedSet waterLEDStrip = LedSet();

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
const uint8_t kCarrotLEDIndex = 2;
const uint8_t kEggplantLEDIndex = 3;
const uint8_t kCucumberLEDIndex = 4;
const uint8_t kPotatoLEDIndex = 5;
const uint8_t kPepperLEDIndex = 6;
const uint8_t kCornLEDIndex = 7;
const uint8_t kStatusLEDIndex = 8;

uint8_t veggies[] = { kCarrotLEDIndex, kEggplantLEDIndex, kCucumberLEDIndex, kPotatoLEDIndex, kPepperLEDIndex, kCornLEDIndex };
//uint8_t veggies[] = { 6, 13, 5, 10, 9, 11 };

uint8_t stoveTemp = 0;
uint8_t useWater = 0;
uint8_t oldWater = 0;
uint8_t veggieOne = 0;
uint8_t veggieTwo = 0;
uint8_t veggieThree = 0;
uint8_t veggieFour = 0;

uint8_t gameStep = 0;
uint8_t stepState = 0;

int statusLEDPin = 22;
//Adafruit_NeoPixel setTwo = Adafruit_NeoPixel(9, statusLEDPin, NEO_GRB + NEO_KHZ800);
LedSet statusStrip = LedSet();

void gameSetup();
void shuffle(uint8_t *aArray, uint8_t aElements);
void succeeded();
void timedOut();
void ledTimedOut();
void waterTimedOut();
//void colorWipe(Adafruit_NeoPixel strip, uint32_t c);
//void theaterChase(Adafruit_NeoPixel strip, uint32_t c, uint8_t wait);
void resetGame();
void updateFaucet();

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
uint8_t checkVeggies(uint8_t aVegLEDIndex);
void turnOffVeggieLeds();
void printBinary(uint8_t aValue);

void setup() {
  Serial.begin(19200);

  pinMode(potPinL, INPUT);
  pinMode(potPinR, INPUT);

  pinMode(waterSwitchPin, OUTPUT);

  pinMode(touchOnePin, INPUT);
  pinMode(touchTwoPin, INPUT);
  pinMode(touchThreePin, INPUT);
  pinMode(touchFourPin, INPUT);

  clockServo.writeMicroseconds(1475);
  clockServo.attach(9);

  gameSetup();
}

void gameSetup() {
  uint16_t seed = micros();
  randomSeed(seed);
  Serial.print("Seed: ");
  Serial.println(seed);

  stoveTemp = random(1, 3);
  Serial.print("Stove Temp: ");
  Serial.println(stoveTemp);
  useWater = random(2);
  Serial.print("Use water: ");
  Serial.println(useWater);
  shuffle(veggies, 6);

  waterLEDStrip.initialize(3, waterLEDPin, NEO_GRB + NEO_KHZ800);
  statusStrip.initialize(9, statusLEDPin, NEO_GRB + NEO_KHZ800);

  //timerId = timer.setTimeout(timeoutTime, timedOut);
  //timer.deleteTimer(timerId);
}

void loop() {
  timer.run();
  waterLEDStrip.updateState();
  statusStrip.updateState();
  updateFaucet();
  touchLoop();
  if (gameStep == 0) {
    if (stepState == kStepStateStart) {
      Serial.println("STOVE");
      timerId = timer.setTimeout(timeoutTime, timedOut);
      if (stoveTemp == kStoveMedium) {
        statusStrip.setColor(kStoveLEDIndex, Color(127, 50, 0));
      } else if (stoveTemp == kStoveHot) {
        statusStrip.setColor(kStoveLEDIndex, Color(127, 0, 0));
      }

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
      statusStrip.setColor(kStoveLEDIndex, Color(0, 0, 0));

      stepState = kStepStateStart;
      gameStep++;
    }

  } else if (gameStep == 1) {
    if (stepState == kStepStateStart) {
      timerId = timer.setTimeout(timeoutTime, timedOut);
      if (useWater) {
        Serial.println("WATER");
        statusStrip.setColor(kFaucetLEDIndex, Color(0, 0, 127));
      } else {
        Serial.print("VEGGIE 1: ");
        Serial.println(veggies[0]);
        statusStrip.setColor(veggies[0], Color(0, 127, 0));
      }

      stepState =  kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (useWater) {
        if (waterOn && !waterHot && !waterFilled && (touches > 0)) {
          Serial.println("Cold water filled!");
          Animation water = {};
          water.startColor = Color(127, 127, 127);
          water.endColor = Color(0, 0, 127);
          water.repeats = 3;
          water.yoyo = false;
          water.tweenTime = 400;
          water.delayTime = 0;
          water.repeatDelay = 0;
          water.ease = EASE_LINEAR;
          water.repeatForever = false;
          statusStrip.animateLed(kFaucetLEDIndex, water);

          waterFilled = true;
        }
        if (!waterOn && waterFilled) {
          Serial.println("Water turned off!");
          succeeded();
        }
      } else {
        if (checkVeggies(veggieLEDToValue(veggies[0])) == 1) {
          succeeded();
        } else if (checkVeggies(veggieLEDToValue(veggies[0])) == 0) {
          timedOut();
        }
      }
    } else if (stepState == kStepStateEnd) {
      waterFilled = false;

      if (useWater) {
        statusStrip.setColor(kFaucetLEDIndex, Color(0, 0, 0));
      } else {
        statusStrip.setColor(veggies[0], Color(0, 0, 0));
      }

      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 2) {
    if (stepState == kStepStateStart) {
      Serial.print("VEGGIE 2: ");
      Serial.println(veggies[1]);
      timerId = timer.setTimeout(timeoutTime, timedOut);
      statusStrip.setColor(veggies[1], Color(0, 127, 0));

      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (checkVeggies(veggieLEDToValue(veggies[1])) == 1) {
        Serial.println("veg 2 success");
        succeeded();
      } else if (checkVeggies(veggieLEDToValue(veggies[1])) == 0) {
        Serial.println("veg 2 fail");
        timedOut();
      }
    } else if (stepState == kStepStateEnd) {
      statusStrip.setColor(veggies[1], Color(0, 0, 0));

      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 3) {
    if (stepState == kStepStateStart) {
      Serial.print("VEGGIE 3: ");
      Serial.println(veggies[2]);
      statusStrip.setColor(veggies[2], Color(0, 127, 0));

      timerId = timer.setTimeout(timeoutTime, timedOut);
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (checkVeggies(veggieLEDToValue(veggies[2])) == 1) {
        Serial.println("veg 3 success");
        succeeded();
      } else if (checkVeggies(veggieLEDToValue(veggies[2])) == 0) {
        Serial.println("veg 3 fail");
        timedOut();
      }
    } else if (stepState == kStepStateEnd) {
      statusStrip.setColor(veggies[2], Color(0, 0, 0));

      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 4) {
    if (stepState == kStepStateStart) {
      Serial.print("VEGGIE 4: ");
      Serial.println(veggies[3]);
      timerId = timer.setTimeout(timeoutTime, timedOut);
      statusStrip.setColor(veggies[3], Color(0, 127, 0));

      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (checkVeggies(veggieLEDToValue(veggies[3])) == 1) {
        Serial.println("veg 4 success");
        succeeded();
      } else if (checkVeggies(veggieLEDToValue(veggies[3])) == 0) {
        Serial.println("veg 4 fail");
        timedOut();
      }
    } else if (stepState == kStepStateEnd) {
      statusStrip.setColor(veggies[3], Color(0, 0, 0));

      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 5) {
    if (stepState == kStepStateStart) {
      Serial.println("STOVE OFF");
      timerId = timer.setTimeout(timeoutTime, timedOut);
      statusStrip.setColor(kStoveLEDIndex, Color(0, 0, 127));
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
      statusStrip.setColor(kStoveLEDIndex, Color(0, 0, 0));

      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 6) {
    if (stepState == kStepStateStart) {
      Serial.println("HOT WATER");
      timerId = timer.setTimeout(timeoutTime, timedOut);
      statusStrip.setColor(kFaucetLEDIndex, Color(127, 0, 0));

      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (waterOn && waterHot && !waterFilled && (touches > 0)) {
        Serial.println("Hot water filled!");
        Animation water = {};
        water.startColor = Color(127, 127, 127);
        water.endColor = Color(127, 0, 0);
        water.repeats = 3;
        water.yoyo = false;
        water.tweenTime = 400;
        water.delayTime = 0;
        water.repeatDelay = 0;
        water.ease = EASE_LINEAR;
        water.repeatForever = false;
        statusStrip.animateLed(kFaucetLEDIndex, water);

        waterFilled = true;
      }
      if (!waterOn && waterFilled) {
        succeeded();
      }
    } else if (stepState == kStepStateEnd) {
      waterFilled = false;
      waterHot = false;
      statusStrip.setColor(kFaucetLEDIndex, Color(0, 0, 0));

      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 7) {
    if (stepState == kStepStateStart) {
      Animation status = {};
      status.startColor = Color(0, 0, 0);
      status.endColor = Color(0, 127, 0);
      status.repeats = success - 1;
      status.yoyo = false;
      status.tweenTime = 1000;
      status.delayTime = 0;
      status.repeatDelay = 0;
      status.ease = EASE_BLINK;
      status.animComplete = false;
      statusStrip.animateLed(kStatusLEDIndex, status);

      Serial.print("Game over - Succeeded: ");
      Serial.print(success);
      Serial.print(", Failed: ");
      Serial.println(failed);

      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (statusStrip.getAnimation(kStatusLEDIndex).animComplete) {
        stepState = kStepStateEnd;
      }
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      if (failed == 0) {
        gameStep++;
      }
      gameStep++;
    }
  } else if (gameStep == 8) {
    if (stepState == kStepStateStart) {
      Animation status = {};
      status.startColor = Color(0, 0, 0);
      status.endColor = Color(127, 0, 0);
      status.repeats = failed - 1;
      status.yoyo = false;
      status.tweenTime = 1000;
      status.delayTime = 0;
      status.repeatDelay = 0;
      status.ease = EASE_BLINK;
      status.animComplete = false;
      statusStrip.animateLed(kStatusLEDIndex, status);

      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (statusStrip.getAnimation(kStatusLEDIndex).animComplete) {
        stepState = kStepStateEnd;
      }
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 9) {
    if (stepState == kStepStateStart) {
      Animation status = {};
      if (success >= 6) {
        status.startColor = Color(0, 127, 0);
      } else {
        status.startColor = Color(127, 0, 0);
      }
      status.endColor = Color(0, 0, 0);
      status.repeats = 3;
      status.yoyo = false;
      status.tweenTime = 1000;
      status.delayTime = 0;
      status.repeatDelay = 0;
      status.ease = EASE_LINEAR;
      status.animComplete = false;
      statusStrip.animateAll(status, true, 100);

      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      if (statusStrip.getAnimation(kStatusLEDIndex).animComplete) {
        Serial.println("DONE");
        stepState = kStepStateEnd;
      }
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      resetGame();
    }
  }
  /*clockServo.write(clockPos);
  clockPos++;
  if (clockPos > 180){
    clockPos = 0;
  }*/
}

void resetGame() {
  timerId = -1;
  waterTimerId = -1;
  ledTimerId = -1;

  waterHot = 0;
  waterFilled = false;

  stoveTemp = 0;
  useWater = 0;
  oldWater = 0;
  veggieOne = 0;
  veggieTwo = 0;
  veggieThree = 0;
  veggieFour = 0;

  gameStep = 0;
  stepState = 0;

  success = 0;
  failed = 0;
  valL = 0;
  valR = 0;

  oldTouches = 0;
  touches = 0;
  filterOne = 0;
  filterTwo = 0;
  filterThree = 0;
  filterFour = 0;
  timestamp = 0;
  identified = false;

  gameSetup();
}

void succeeded() {
  timer.deleteTimer(timerId);
  Serial.println("Succeeded!");
  success++;

  identified = false;
  stepState = kStepStateWait;
  statusStrip.setColor(kStatusLEDIndex, Color(0, 0, 127));

  ledTimerId = timer.setTimeout(ledTimeout, ledTimedOut);
}

void timedOut() {
  //timer.deleteTimer(timerId);
  Serial.println("Event Timeout");
  timerId = -1;
  failed++;

  identified = false;
  stepState = kStepStateWait;
  statusStrip.setColor(kStatusLEDIndex, Color(127, 0, 0));

  ledTimerId = timer.setTimeout(ledTimeout, ledTimedOut);
}

void ledTimedOut() {
  Serial.print("LED Timeout: ");
  Serial.println(ledTimerId);
  //timer.disable(ledTimerId);
  ledTimerId = -1;
  statusStrip.setColor(kStatusLEDIndex, Color(0, 0, 0));

  stepState = kStepStateEnd;
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

void updateFaucet() {
  oldWater = waterOn;
  waterOn = digitalRead(waterSwitchPin);

  if (waterOn && (waterOn != oldWater)) {
    waterHot = false;
    Serial.println("Water on");
    waterTimerId = timer.setTimeout(waterTimeout, waterTimedOut);
    Animation water = {};
    water.startColor = Color(160, 160, 160);
    water.endColor = Color(0, 0, 255);
    water.repeats = 0;
    water.yoyo = false;
    water.tweenTime = 400;
    water.delayTime = 0;
    water.repeatDelay = 0;
    water.ease = EASE_LINEAR;
    water.repeatForever = true;
    water.animComplete = false;
    waterLEDStrip.animateAll(water, true, 100);
    //theaterChase(waterLEDStrip, Color(0, 0, 127), 0); // Blue
  } else if (!waterOn && (waterOn != oldWater)) {
    if (timer.isEnabled(waterTimerId)) {
      Serial.println("Disable water timer: ");
      Serial.println(waterTimerId);
      timer.deleteTimer(waterTimerId);
      waterTimerId = -1;
    }
    waterLEDStrip.killAllAnims();
    waterLEDStrip.setAllOff();
    waterHot = false;
  }
}

void waterTimedOut() {
  if (waterOn) {
    Serial.println("Water Timeout");
    //timer.deleteTimer(waterTimerId);
    Serial.println("Water now HOT.");
    waterTimerId = -1;
    waterHot = true;
    Animation water = {};
    water.startColor = Color(160, 160, 160);
    water.endColor = Color(255, 0, 0);
    water.repeats = 0;
    water.yoyo = false;
    water.tweenTime = 400;
    water.delayTime = 0;
    water.repeatDelay = 0;
    water.ease = EASE_LINEAR;
    water.repeatForever = true;
    waterLEDStrip.animateAll(water, true, 100);
  }
}

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
      //Serial.println("Need Carrot");
      result = kCarrot;
      break;
    case kEggplantLEDIndex:
      //Serial.println("Need Eggplant");
      result = kEggplant;
      break;
    case kCucumberLEDIndex:
      //Serial.println("Need Cucumber");
      result = kCucumber;
      break;
    case kPotatoLEDIndex:
      //Serial.println("Need Potato");
      result = kPotato;
      break;
    case kPepperLEDIndex:
      //Serial.println("Need Pepper");
      result = kPepper;
      break;
    case kCornLEDIndex:
      //Serial.println("Need Corn");
      result = kCorn;
      break;
  }
  return result;
}

uint8_t checkVeggies(uint8_t aVeggie) {
  //turnOffVeggieLeds();
  uint8_t result = 2;
    if (!identified && (touches == kCarrot)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setColor(kCarrotLEDIndex, Color(0, 0, 127));
        Serial.println("CARROT");
        result = (aVeggie == kCarrot) ? 1 : 0;
    } else if (!identified && (touches == kEggplant)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setColor(kEggplantLEDIndex, Color(0, 0, 127));
        Serial.println("EGGPLANT");
        result = (aVeggie == kEggplant) ? 1 : 0;
    } else if (!identified && (touches == kCucumber)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setColor(kCucumberLEDIndex, Color(0, 0, 127));
        Serial.println("CUCUMBER");
        result = (aVeggie == kCucumber) ? 1 : 0;
    } else if (!identified && (touches == kPotato)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setColor(kPotatoLEDIndex, Color(0, 0, 127));
        Serial.println("POTATO");
        result = (aVeggie == kPotato) ? 1 : 0;
    } else if (!identified && (touches == kPepper)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setColor(kPepperLEDIndex, Color(0, 0, 127));
        Serial.println("PEPPER");
        result = (aVeggie == kPepper) ? 1 : 0;
    } else if (!identified && (touches == kCorn)) {
        identified = true;
        //turnOffVeggieLeds();
        //statusStrip.setColor(kCornLEDIndex, Color(0, 0, 127));
        Serial.println("CORN");
        result = (aVeggie == kCorn) ? 1 : 0;
    }
    //
    return result;
}

void turnOffVeggieLeds() {
  statusStrip.setColor(kCarrotLEDIndex, Color(0, 0, 0));
  statusStrip.setColor(kEggplantLEDIndex, Color(0, 0, 0));
  statusStrip.setColor(kCucumberLEDIndex, Color(0, 0, 0));
  statusStrip.setColor(kPotatoLEDIndex, Color(0, 0, 0));
  statusStrip.setColor(kPepperLEDIndex, Color(0, 0, 0));
  statusStrip.setColor(kCornLEDIndex, Color(0, 0, 0));

}

void printBinary(uint8_t aValue) {
    printf("touch: %d, old touch: %d, 0x%d%d%d%d%d%d%d%d\n", touches, oldTouches,
        IsBitSet(aValue, 7), IsBitSet(aValue, 6),
        IsBitSet(aValue, 5), IsBitSet(aValue, 4),
        IsBitSet(aValue, 3), IsBitSet(aValue, 2),
        IsBitSet(aValue, 1), IsBitSet(aValue, 0));
}