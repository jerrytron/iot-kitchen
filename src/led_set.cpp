#include "led_set.h"
#include "math.h"

/* Public Methods */

LedSet::LedSet() {
}

void LedSet::initialize(uint16_t aLedCount, uint8_t aLedPin, uint8_t aLedType) {
	// Setup the neopixel set.
	_ledSet = new Adafruit_NeoPixel(aLedCount, aLedPin, aLedType);
	_ledSet->begin();
	_ledSet->show();
	// Setup persistent storage for LED state information.
	_leds = new Led[aLedCount]();
	for (uint16_t i = 0; i < _ledCount; ++i) {
		Led led = {};
		_leds[i] = led;
	}

	_ledCount = aLedCount;
	this->active = true;
}

void LedSet::updateState() {
	if (this->active) {
		if (_updateTime >= 10) {
			updateLeds();
			_updateTime = 0;
		}
	}
}

uint8_t LedSet::numPixels() {
	return _ledCount;
}

Color LedSet::getColor(uint16_t aLedIndex) {
	Color color = {};
	if (aLedIndex < _ledCount) {
		uint32_t colorVal = _ledSet->getPixelColor(aLedIndex);
		color.red = (uint8_t)(colorVal >> 16);
		color.green = (uint8_t)(colorVal >> 8);
		color.blue = (uint8_t)colorVal;
	}
	return color;
}

void LedSet::setColor(uint16_t aLedIndex, Color aColor) {
	if (aLedIndex < _ledCount) {
		uint32_t val = _ledSet->Color(aColor.red, aColor.green, aColor.blue);
		_ledSet->setPixelColor(aLedIndex, val);
		_ledSet->show();
	}
}

void LedSet::setAllColors(Color aColor) {
	for (uint16_t i = 0; i < _ledCount; ++i) {
		setColor(i, aColor);
	}
}

void LedSet::setOff(uint16_t aLedIndex) {
	_leds[aLedIndex].anim.repeatForever = false;
	setColor(aLedIndex, Color(0, 0, 0));
}

void LedSet::setAllOff() {
    for (uint16_t i = 0; i < _ledCount; ++i) {
       setOff(i);
    }
}

Animation LedSet::getAnimation(uint16_t aLedIndex) {
	if (aLedIndex < _ledCount) {
		return _leds[aLedIndex].anim;
	}
	return Animation();
}

void LedSet::pauseAnim(uint8_t aLedIndex) {
	if (aLedIndex < _ledCount) {
		if (!_leds[aLedIndex].paused) {
			_leds[aLedIndex].paused = true;
			_leds[aLedIndex].pauseTime = _leds[aLedIndex].elapsedTime;
		}
	}
}

void LedSet::pauseAllAnims() {
	for (uint16_t i = 0; i < _ledCount; ++i) {
		pauseAnim(i);
	}
}

void LedSet::resumeAnim(uint8_t aLedIndex) {
	if (aLedIndex < _ledCount) {
		if (_leds[aLedIndex].paused) {
			_leds[aLedIndex].paused = false;
			_leds[aLedIndex].pauseTime = 0;
			_leds[aLedIndex].elapsedTime = _leds[aLedIndex].pauseTime;
		}
	}
}

void LedSet::resumeAllAnims() {
	for (uint16_t i = 0; i < _ledCount; ++i) {
		resumeAnim(i);
	}
}

void LedSet::restartAnim(uint8_t aLedIndex, bool aWithDelay) {
	if (aLedIndex < _ledCount) {
		_leds[aLedIndex].elapsedTime = 0;
		_leds[aLedIndex].anim.currentColor = _leds[aLedIndex].anim.startColor;
		if (_leds[aLedIndex].anim.delayTime && aWithDelay) {
			_leds[aLedIndex].state = LED_DELAY;
		} else {
			_leds[aLedIndex].state = LED_ANIMATING;
		}
		setColor(aLedIndex, _leds[aLedIndex].anim.startColor);
	}
}

void LedSet::killAnim(uint8_t aLedIndex) {
	if (aLedIndex < _ledCount) {
		_leds[aLedIndex].anim = {};
		_leds[aLedIndex].paused = false;
		_leds[aLedIndex].pauseTime = 0;
		_leds[aLedIndex].state = LED_IDLE;
	}
}

void LedSet::killAllAnims() {
	for (uint16_t i = 0; i < _ledCount; ++i) {
		killAnim(i);
	}
}

void LedSet::animateLed(uint8_t aLedIndex, Animation aAnimation, bool aFromCurrentColor) {
	if (aFromCurrentColor) {
		aAnimation.startColor = getColor(aLedIndex);
	}
	if (aAnimation.delayTime) {
		_leds[aLedIndex].state = LED_DELAY;
	} else {
		_leds[aLedIndex].state = LED_ANIMATING;
	}
	_leds[aLedIndex].anim = aAnimation;
	_leds[aLedIndex].elapsedTime = 0;
}

void LedSet::animateSet(int aLedIndexes[], Animation aAnims[], uint8_t aLength, uint32_t aDelayBetween, bool aFromCurrentColor) {
	for (uint16_t i = 0; i < aLength; ++i) {
		animateLed(aLedIndexes[i], aAnims[i], aFromCurrentColor);
		if ((i + 1) < aLength) {
			// Add the between delay to the next animation.
			aAnims[i + 1].delayTime += aDelayBetween;
		}
	}
}

void LedSet::animateSet(int aLedIndexes[], Animation aAnim, uint8_t aLength, uint32_t aDelayBetween, bool aFromCurrentColor) {
	for (uint16_t i = 0; i < aLength; ++i) {
		animateLed(aLedIndexes[i], aAnim, aFromCurrentColor);
		// Add the between delay to the next animation.
		aAnim.delayTime += aDelayBetween;
	}
}

void LedSet::animateRange(uint8_t aLowIndex, uint8_t aHighIndex, Animation aAnim, bool aCountUp, uint32_t aDelayBetween, bool aFromCurrentColor) {
	if (aCountUp) {
		for (uint16_t i = aLowIndex; i < aHighIndex; ++i) {
			animateLed(i, aAnim, aFromCurrentColor);
			aAnim.delayTime += aDelayBetween;
		}
	} else {
		for (uint16_t i = aHighIndex; i > aLowIndex; --i) {
			animateLed(i, aAnim, aFromCurrentColor);
			aAnim.delayTime += aDelayBetween;
		}
	}
}

void LedSet::animateAll(Animation aAnim, bool aCountUp, uint32_t aDelayBetween, bool aFromCurrentColor) {
	animateRange(0, _ledCount, aAnim, aCountUp, aDelayBetween, aFromCurrentColor);
}

/* Private Methods */

void LedSet::updateLeds() {
	for (uint16_t i = 0; i < _ledCount; ++i) {
		if (_leds[i].state == LED_DELAY) {
			if (_leds[i].elapsedTime >= _leds[i].anim.delayTime) {
				_leds[i].elapsedTime = 0;
				_leds[i].state = LED_ANIMATING;
			}
		} else if (_leds[i].state == LED_REPEAT_DELAY) {
			if (_leds[i].elapsedTime >= _leds[i].anim.repeatDelay) {
				_leds[i].elapsedTime = 0;
				_leds[i].state = LED_ANIMATING;
			}
		} else if (_leds[i].state == LED_ANIMATING) {
			if (!_leds[i].paused) {
				_leds[i].anim.currentColor = calculateEase(_leds[i].anim.ease, _leds[i].elapsedTime,
				                                           _leds[i].anim.tweenTime, _leds[i].anim.startColor,
				                                           _leds[i].anim.endColor);

				uint32_t colorVal = 0;
				if (_leds[i].elapsedTime >= _leds[i].anim.tweenTime) {
					colorVal = _ledSet->Color(_leds[i].anim.endColor.red, _leds[i].anim.endColor.green, _leds[i].anim.endColor.blue);
					_ledSet->setPixelColor(i, colorVal);
					_leds[i].elapsedTime = 0;
					if ((_leds[i].anim.repeats > 0) || _leds[i].anim.repeatForever) {
						// Decrement number of repeats.
						if (!_leds[i].anim.repeatForever) {
							_leds[i].anim.repeats--;
						}
						// Yoyo reverses the animation.
						if (_leds[i].anim.yoyo) {
							// Swap start and end colors.
							Color endColor = _leds[i].anim.startColor;
							_leds[i].anim.startColor = _leds[i].anim.endColor;
							_leds[i].anim.endColor = endColor;
							// Reverse the ease.
							if ((_leds[i].anim.ease >= EASE_QUAD_IN) &&
							    (_leds[i].anim.ease <= EASE_CIRCULAR_IN)) {
								_leds[i].anim.ease = (EaseType)(_leds[i].anim.ease + kEaseInOutDiff);
							} else if ((_leds[i].anim.ease >= EASE_QUAD_OUT) &&
							    (_leds[i].anim.ease <= EASE_CIRCULAR_OUT)) {
								_leds[i].anim.ease = (EaseType)(_leds[i].anim.ease - kEaseInOutDiff);
							}
						}
						// Check if there is a delay before the repeat.
						if (_leds[i].anim.repeatDelay) {
							_leds[i].state = LED_REPEAT_DELAY;
						}
					} else {
						_leds[i].anim.startColor = _leds[i].anim.endColor;
						_leds[i].state = LED_IDLE;
					}
				} else {
					colorVal = _ledSet->Color(_leds[i].anim.currentColor.red, _leds[i].anim.currentColor.green, _leds[i].anim.currentColor.blue);
					_ledSet->setPixelColor(i, colorVal);
				}
			}
		} else if (_leds[i].state == LED_IDLE) {
		}
	}
	_ledSet->show();
}

Color LedSet::calculateEase(EaseType aEase, float aCurrentFrame, float aEndFrame, Color aStart, Color aEnd) {
	Color color = Color(0, 0, 0);
	float redChange = aEnd.red - aStart.red;
	float greenChange = aEnd.green - aStart.green;
	float blueChange = aEnd.blue - aStart.blue;

	switch(aEase) {
		default:
			Serial.print("Ease not recognized: ");
			Serial.println(aEase);
		case EASE_LINEAR:
			color.red = easeLinear(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeLinear(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeLinear(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_BLINK:
			if ((aCurrentFrame / aEndFrame) < 0.5f) {
				color = aStart;
			} else {
				color = aEnd;
			}
			break;
		case EASE_QUAD_IN:
			color.red = easeQuadraticIn(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeQuadraticIn(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeQuadraticIn(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_QUAD_OUT:
			color.red = easeQuadraticOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeQuadraticOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeQuadraticOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_QUAD_IN_OUT:
			color.red = easeQuadraticInOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeQuadraticInOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeQuadraticInOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_CUBIC_IN:
			color.red = easeCubicIn(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeCubicIn(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeCubicIn(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_CUBIC_OUT:
			color.red = easeCubicOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeCubicOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeCubicOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_CUBIC_IN_OUT:
			color.red = easeCubicInOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeCubicInOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeCubicInOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_QUARTIC_IN:
			color.red = easeQuarticIn(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeQuarticIn(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeQuarticIn(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_QUARTIC_OUT:
			color.red = easeQuarticOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeQuarticOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeQuarticOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_QUARTIC_IN_OUT:
			color.red = easeQuarticInOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeQuarticInOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeQuarticInOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_QUINTIC_IN:
			color.red = easeQuinticIn(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeQuinticIn(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeQuinticIn(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_QUINTIC_OUT:
			color.red = easeQuinticOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeQuinticOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeQuinticOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_QUINTIC_IN_OUT:
			color.red = easeQuinticInOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeQuinticInOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeQuinticInOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_SIN_IN:
			color.red = easeSinIn(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeSinIn(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeSinIn(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_SIN_OUT:
			color.red = easeSinOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeSinOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeSinOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_SIN_IN_OUT:
			color.red = easeSinInOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeSinInOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeSinInOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_EXP_IN:
			color.red = easeExponentialIn(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeExponentialIn(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeExponentialIn(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_EXP_OUT:
			color.red = easeExponentialOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeExponentialOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeExponentialOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_EXP_IN_OUT:
			color.red = easeExponentialInOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeExponentialInOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeExponentialInOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_CIRCULAR_IN:
			color.red = easeCircularIn(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeCircularIn(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeCircularIn(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_CIRCULAR_OUT:
			color.red = easeCircularOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeCircularOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeCircularOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
		case EASE_CIRCULAR_IN_OUT:
			color.red = easeCircularInOut(aCurrentFrame, aEndFrame, aStart.red, redChange);
			color.green = easeCircularInOut(aCurrentFrame, aEndFrame, aStart.green, greenChange);
			color.blue = easeCircularInOut(aCurrentFrame, aEndFrame, aStart.blue, blueChange);
			break;
	}

	/*Serial.print("R: ");
	Serial.print(color.red);
	Serial.print(", G: ");
	Serial.print(color.green);
	Serial.print(", B: ");
	Serial.println(color.blue);*/
	return color;
}

/* Easing Functions */

float LedSet::easeLinear(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	return aChangeVal * aCurrentFrame / aEndFrame + aStartVal;
}

float LedSet::easeQuadraticIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame;
	return aChangeVal * aCurrentFrame * aCurrentFrame + aStartVal;
}

float LedSet::easeQuadraticOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame;
	return -aChangeVal * aCurrentFrame * (aCurrentFrame - 2) + aStartVal;
}

float LedSet::easeQuadraticInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame/2;
	if (aCurrentFrame < 1) {
		return aChangeVal / 2 * aCurrentFrame*aCurrentFrame + aStartVal;
	}
	aCurrentFrame--;
	return -aChangeVal / 2 * (aCurrentFrame * (aCurrentFrame - 2) - 1) + aStartVal;
}

float LedSet::easeCubicIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame;
	return aChangeVal * aCurrentFrame * aCurrentFrame * aCurrentFrame + aStartVal;
}

float LedSet::easeCubicOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame;
	aCurrentFrame--;
	return aChangeVal * (aCurrentFrame * aCurrentFrame * aCurrentFrame + 1) + aStartVal;
}

float LedSet::easeCubicInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame/2;
	if (aCurrentFrame < 1) {
		return aChangeVal / 2 * aCurrentFrame * aCurrentFrame * aCurrentFrame + aStartVal;
	}
	aCurrentFrame -= 2;
	return aChangeVal / 2 * (aCurrentFrame * aCurrentFrame * aCurrentFrame + 2) + aStartVal;
}

float LedSet::easeQuarticIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame;
	return aChangeVal * aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame + aStartVal;
}

float LedSet::easeQuarticOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame;
	aCurrentFrame--;
	return -aChangeVal * (aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame - 1) + aStartVal;
}

float LedSet::easeQuarticInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame/2;
	if (aCurrentFrame < 1) {
		return aChangeVal / 2 * aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame + aStartVal;
	}
	aCurrentFrame -= 2;
	return -aChangeVal / 2 * (aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame - 2) + aStartVal;
}

float LedSet::easeQuinticIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame;
	return aChangeVal * aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame + aStartVal;
}

float LedSet::easeQuinticOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame;
	aCurrentFrame--;
	return aChangeVal * (aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame + 1) + aStartVal;
}

float LedSet::easeQuinticInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame/2;
	if (aCurrentFrame < 1) {
		return aChangeVal / 2 * aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame + aStartVal;
	}
	aCurrentFrame -= 2;
	return aChangeVal / 2 * (aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame * aCurrentFrame + 2) + aStartVal;
}


float LedSet::easeSinIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	return -aChangeVal * cosf(aCurrentFrame / aEndFrame * (M_PI / 2)) + aChangeVal + aStartVal;
}

float LedSet::easeSinOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	return aChangeVal * sinf(aCurrentFrame / aEndFrame * (M_PI / 2)) + aStartVal;
}

float LedSet::easeSinInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	return -aChangeVal / 2 * (cosf(M_PI * aCurrentFrame / aEndFrame) - 1) + aStartVal;
}

float LedSet::easeExponentialIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	return aChangeVal * powf(2, 10 * (aCurrentFrame / aEndFrame - 1) ) + aStartVal;
}

float LedSet::easeExponentialOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	return aChangeVal * (-powf(2, -10 * aCurrentFrame / aEndFrame ) + 1 ) + aStartVal;
}

float LedSet::easeExponentialInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame / 2;
	if (aCurrentFrame < 1) {
		return aChangeVal / 2 * powf(2, 10 * (aCurrentFrame - 1)) + aStartVal;
	}
	aCurrentFrame--;
	return aChangeVal / 2 * (-powf(2, -10 * aCurrentFrame) + 2) + aStartVal;
}

float LedSet::easeCircularIn(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame;
	return -aChangeVal * (sqrtf(1 - aCurrentFrame * aCurrentFrame) - 1) + aStartVal;
}

float LedSet::easeCircularOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame;
	aCurrentFrame--;
	return aChangeVal * sqrtf(1 - aCurrentFrame * aCurrentFrame) + aStartVal;
}

float LedSet::easeCircularInOut(float aCurrentFrame, float aEndFrame, float aStartVal, float aChangeVal) {
	aCurrentFrame /= aEndFrame / 2;
	if (aCurrentFrame < 1) {
		return -aChangeVal / 2 * (sqrtf(1 - aCurrentFrame * aCurrentFrame) - 1) + aStartVal;
	}
	aCurrentFrame -= 2;
	return aChangeVal / 2 * (sqrtf(1 - aCurrentFrame * aCurrentFrame) + 1) + aStartVal;
}