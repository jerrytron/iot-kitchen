#ifndef MWAM_LED_RING_H
#define MWAM_LED_RING_H

#include "ElapsedTime.h"
#include "Adafruit_NeoPixel.h"

// Led States
typedef enum LedState_t {
	LED_IDLE = 0,
	LED_DELAY,
	LED_REPEAT_DELAY,
	LED_ANIMATING
} LedState;

typedef enum EaseType_t {
	EASE_LINEAR = 0,
	EASE_BLINK,
	EASE_QUAD_IN,
	EASE_CUBIC_IN,
	EASE_QUARTIC_IN,
	EASE_QUINTIC_IN,
	EASE_SIN_IN,
	EASE_EXP_IN,
	EASE_CIRCULAR_IN,
	EASE_QUAD_OUT,
	EASE_CUBIC_OUT,
	EASE_QUARTIC_OUT,
	EASE_QUINTIC_OUT,
	EASE_SIN_OUT,
	EASE_EXP_OUT,
	EASE_CIRCULAR_OUT,
	EASE_QUAD_IN_OUT,
	EASE_CUBIC_IN_OUT,
	EASE_QUARTIC_IN_OUT,
	EASE_QUINTIC_IN_OUT,
	EASE_SIN_IN_OUT,
	EASE_EXP_IN_OUT,
	EASE_CIRCULAR_IN_OUT
} EaseType;

// Can switch between an ease in and out by adding / subtracting diff.
const uint8_t kEaseInOutDiff = EASE_QUAD_OUT - EASE_QUAD_IN;

typedef struct Color_t {
	uint8_t red;
	uint8_t green;
	uint8_t blue;

	// Default Constructor
	Color_t(uint8_t aRed=0, uint8_t aGreen=0, uint8_t aBlue=0)
		: red(aRed), green(aGreen), blue(aBlue) {}

	// Assignment Operator
	Color_t& operator=(const Color_t& aColor) {
		red = aColor.red;
		green = aColor.green;
		blue = aColor.blue;
		return *this;
	}

	// Add Operation
	Color_t operator+(const Color_t& aColor) const {
		return Color_t(aColor.red + red, aColor.green + green, aColor.blue + blue);
	}

	// Subtract Operation
	Color_t operator-(const Color_t& aColor) const {
		return Color_t(aColor.red - red, aColor.green - green, aColor.blue - blue);
	}

	// Equality Operation
	bool operator==(const Color_t& aColor) const {
		return (aColor.red == red && aColor.green == green && aColor.blue == blue);
	}
} Color;

typedef struct Animation_t {
	uint32_t tweenTime = 0; // In millis.
	uint32_t delayTime = 0; // In millis.
	uint32_t repeatDelay = 0; // In millis;
	Color startColor = Color(0, 0, 0);
	Color currentColor = Color(0, 0, 0);
	Color endColor = Color(0, 0, 0);
	EaseType ease = EASE_LINEAR;
	uint16_t repeats = 0;
	bool repeatForever = false;
	bool yoyo = false;
} Animation;

typedef struct Led_t {
	Animation anim;
	ElapsedMillis elapsedTime = 0;
	LedState state = LED_IDLE;
	bool paused = false;
	unsigned long pauseTime = 0;
} Led;

class LedSet
{
	public:
		/* Public Methods */
		LedSet();
		void initialize(uint16_t aLedCount, uint8_t aLedPin, uint8_t aLedType);
		void updateState();
		uint8_t numPixels();
		Color getColor(uint16_t aLedIndex);
		void setColor(uint16_t aLedIndex, Color aColor);
		void setAllColors(Color aColor);
		void setOff(uint16_t aLedIndex);
		void setAllOff();
		Animation getAnimation(uint16_t aLedIndex);
		void pauseAnim(uint8_t aLedIndex);
		void pauseAllAnims();
		void resumeAnim(uint8_t aLedIndex);
		void resumeAllAnims();
		void restartAnim(uint8_t aLedIndex, bool aWithDelay = false);
		void killAnim(uint8_t aLedIndex);
		void killAllAnims();
		void animateLed(uint8_t aLedIndex, Animation aAnimation, bool aFromCurrentColor = false);
		void animateSet(int aLedIndexes[], Animation aAnims[], uint8_t aLength, uint32_t aDelayBetween, bool aFromCurrentColor = false);
		void animateSet(int aLedIndexes[], Animation aAnim, uint8_t aLength, uint32_t aDelayBetween, bool aFromCurrentColor = false);
		void animateRange(uint8_t aLowIndex, uint8_t aHighIndex, Animation aAnim, bool aCountUp, uint32_t aDelayBetween, bool aFromCurrentColor = false);
		void animateAll(Animation aAnim, bool aCountUp, uint32_t aDelayBetween, bool aFromCurrentColor = false);

		/* Public Variables */
		bool active;

	private:
		/* Private Methods */
		void updateLeds();

		Color calculateEase(EaseType aEase, float aCurrentFrame, float aEndFrame, Color aStart, Color aEnd);
		float easeLinear(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeQuadraticIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeQuadraticOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeQuadraticInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeCubicIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeCubicOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeCubicInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeQuarticIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeQuarticOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeQuarticInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeQuinticIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeQuinticOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeQuinticInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeSinIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeSinOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeSinInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeExponentialIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeExponentialOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeExponentialInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeCircularIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeCircularOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);
		float easeCircularInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal);

		//Color easeLinear(Led aLed);

		/* Private Variables */
		Adafruit_NeoPixel* _ledSet;
		Led* _leds;
		uint16_t _ledCount;

};

#endif