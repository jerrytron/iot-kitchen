#include <Arduino.h>
//#include "iotHackDayKitchen.h"
#include "Adafruit_NeoPixel.h"
#include "LPD8806.h"
#include <Servo.h>
#include "SimpleTimer.h"
#include "SPI.h"

SimpleTimer ledTimer;
int ledTimerId = 0;
uint8_t ledIndex = 0;

SimpleTimer timer;
int timerId = 0;
uint16_t timeoutTime = 5000;
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

int waterSwitchPin = 7;
int waterOn = 0;

const uint8_t kStatusLed = 9;

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

const uint8_t veggies[] = {6, 13, 5, 10, 9, 11};

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

void succeeded();
void timedOut();
void ledTimedOut();
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

void touchLoop();
void checkVeggies();
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
  veggieOne = random(6);
  veggieTwo = random(6);
  veggieThree = random(6);
  veggieFour = random(6);
  
  clockServo.attach(9);
  
  pinMode(potPinL, INPUT);
  pinMode(potPinR, INPUT);
  
  pinMode(waterSwitchPin, OUTPUT);
  
  pinMode(touchOnePin, INPUT);
  pinMode(touchTwoPin, INPUT);
  pinMode(touchThreePin, INPUT);
  pinMode(touchFourPin, INPUT);
  
  timerId = timer.setTimeout(timeoutTime, timedOut);
  timer.disable(timerId);
  
  ledTimerId = ledTimer.setTimeout(1000, ledTimedOut);
  ledTimer.disable(ledTimerId);
  
  waterLEDStrip.begin();
  waterLEDStrip.show(); 
  colorWipe(waterLEDStrip, waterLEDStrip.Color(0, 0, 255));
  
  statusStrip.begin();
  statusStrip.show();
  //statusColorWipe(statusStrip, waterLEDStrip.Color(0, 0, 255));
}

void loop() {
  timer.run();
  ledTimer.run();
  if (gameStep == 0) {
    if (stepState == kStepStateStart) {
      timer.enable(timerId);
      timer.restartTimer(timerId);
      if (stoveTemp == kStoveMedium) {
        statusStrip.setPixelColor(gameStep, statusStrip.Color(127, 0, 127));
      } else if (stoveTemp == kStoveHot) {
        statusStrip.setPixelColor(gameStep, statusStrip.Color(127, 0, 0));
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
      stepState = kStepStateStart;
      gameStep++;
    }
    
  } else if (gameStep == 1) {
    if (stepState == kStepStateStart) {
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      
      succeeded();
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 2) {
    if (stepState == kStepStateStart) {
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      
      succeeded();
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 3) {
    if (stepState == kStepStateStart) {
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      
      succeeded();
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 4) {
    if (stepState == kStepStateStart) {
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      
      succeeded();
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 5) {
    if (stepState == kStepStateStart) {
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      
      succeeded();
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 6) {
    if (stepState == kStepStateStart) {
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      
      succeeded();
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 7) {
    if (stepState == kStepStateStart) {
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      
      succeeded();
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 8) {
    if (stepState == kStepStateStart) {
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      
      succeeded();
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 9) {
    if (stepState == kStepStateStart) {
      stepState = kStepStateLoop;
    } else if (stepState == kStepStateLoop) {
      
      succeeded();
    } else if (stepState == kStepStateEnd) {
      stepState = kStepStateStart;
      gameStep++;
    }
  } else if (gameStep == 10) {

  }
  
  
  touchLoop();
  valL = analogRead(potPinL); 
  valR = analogRead(potPinR); 
  //Serial.println("Reading: " + String(valL) + " " + String(valR));
  
  clockServo.write(clockPos);
  clockPos++;
  if (clockPos > 180){
    clockPos = 0;
  }
  
  waterOn = digitalRead(waterSwitchPin);
  //Serial.println("Water on: " + String(waterOn));
  
 // theaterChase(waterLEDStrip, waterLEDStrip.Color(0, 0, 127), 50); // Blue
 // theaterChase(waterLEDStrip, waterLEDStrip.Color(127, 0, 0), 50); // Red
  
  delay(15);
}

void succeeded() {
  timer.disable(timerId);
  success++;
  statusStrip.setPixelColor(kStatusLed, statusStrip.Color(0, 0, 127));
  statusStrip.show();
  ledIndex = gameStep;
  ledTimer.enable(ledTimerId);
  ledTimer.restartTimer(ledTimerId);
  stepState = kStepStateEnd;
}

void timedOut() {
    failed++;
    stepState = kStepStateEnd;
    statusStrip.setPixelColor(kStatusLed, statusStrip.Color(127, 0, 0));
    statusStrip.show();
    timer.disable(timerId);
}

void ledTimedOut() {
    statusStrip.setPixelColor(kStatusLed, statusStrip.Color(0, 0, 0));
    statusStrip.show();
    ledTimer.disable(ledTimerId);
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

void theaterChase(Adafruit_NeoPixel strip, uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < waterLEDStrip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
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
}