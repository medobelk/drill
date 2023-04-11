#include "Display.h"

/*
  HINT 
  "dutyCycle" is pwm duty cycle value 0 - 255 range
  "frequency" is calculated from pulse widthes from optical reader value 0 - maxFrequency range 
  "speed" potentiometer value to change dutyCycle value 0 - 1023 range
*/

const int motorStartBias = 15;
const int minFrequency = 450;
const int loadLedBlinkInterval = 300;
const int displayRenderInterval = 600;
const int overloadMaxTries = 4;
const int notRunningMaxCount = 3;
const float gearRatio = 1.77;

unsigned long previousLoadLedBlinkTime = 0;
unsigned long previousDisplayRenderTime = 0;
int previousDerrivativeError = 0;
int previousIntegralError = 0;
int overloadsCount = 0;
int notRunningCount = 0;
int speedDutyCycle;
int prevTachoHighPulse = 0;
int prevTachoLowPulse = 0;
int prevSpeedValue = 0;


float speedAdjust = 0.0;
float detectedMaxFrequency = 1820;

bool isEnable = false;
bool isReversed = false;
bool reverse = false;
bool isOnBrakes = false;
bool isRunning = false;
bool isOverloaded = false;
bool noReservePower = false;

#define GET_SPEED_PIN    A0
#define OVERLOAD_LED_PIN A1
#define BRAKE_PIN        2
#define REVERSE_PIN      3
#define ENABLE_PIN       4
#define SET_SPEED_PIN    5
#define GET_REVERSE_PIN  6
#define BRAKE_PIN        7
#define REVERSE_PIN      8
#define GET_REVERSE_PIN  9
#define ENABLE_PIN       10
#define SET_SPEED_PIN    11
>>>>>>> c59d59fc86c2405730bb159cc4d573be16f2765a
#define TACHO_PIN        12

void setup() {
  Serial.begin(9600);
  pinMode(TACHO_PIN, INPUT);
  pinMode(SET_SPEED_PIN, OUTPUT);
  digitalWrite(SET_SPEED_PIN, LOW);
  pinMode(ENABLE_PIN, INPUT_PULLUP);
  pinMode(BRAKE_PIN, OUTPUT);
  digitalWrite(BRAKE_PIN, LOW);
  pinMode(GET_REVERSE_PIN, INPUT_PULLUP);
  pinMode(REVERSE_PIN, OUTPUT);
  digitalWrite(REVERSE_PIN, LOW);

  displayInit();
}

void overloadLedSetBehavior(int behavior) {
  unsigned long currentMillis = millis();

  /*
    behavior:
      0 = "off"
      1 = "on"
      2 = "blink"
  */

  switch (behavior) {
    case 0:
      digitalWrite(OVERLOAD_LED_PIN, LOW);
    break;

    case 1:
      digitalWrite(OVERLOAD_LED_PIN, HIGH);
    break;

    case 2:
      if (currentMillis - previousLoadLedBlinkTime >= loadLedBlinkInterval) {
        previousLoadLedBlinkTime = millis();

        digitalWrite(OVERLOAD_LED_PIN, HIGH);
      } else {
        digitalWrite(OVERLOAD_LED_PIN, LOW);
      }
    break;
  } 
}

void setMotorToReverse(bool reverseSwitch) {
  if (reverseSwitch && !isReversed) {
    digitalWrite(REVERSE_PIN, HIGH);

    isReversed = true;
  }
  
  if (!reverseSwitch && isReversed) {
    digitalWrite(REVERSE_PIN, LOW);

    isReversed = false;
  }
}

void motorBrakes(bool brakeSwitch, String dbgMsg = "") {
  if (brakeSwitch && !isOnBrakes) {
    speedDutyCycle = 0;

    digitalWrite(BRAKE_PIN, HIGH);
    setSpeed(0);

    isOnBrakes = true;
  } 

  if (!brakeSwitch && isOnBrakes) {
    digitalWrite(BRAKE_PIN, LOW);

    isOnBrakes = false;
  }

  if (dbgMsg.length() > 0) {
    // Serial.println("brakes func msg: " + dbgMsg);
  }
}

int calculateFrequencyOnSpindle(float frequency) {
  return ceil(frequency / gearRatio);
}

float calculateFrequency(int period) {
  int frequency;

  frequency = (period == 0) ? 0 : round(1000000.0 / period);

  return frequency;
}

int normalizeSpeedToFrequency(int speed) { //converts range 0 - 1023 to range of 0 - max frequency
  return round(speed * (float) detectedMaxFrequency / 1023);
}

int normalizeDutyCycleToFrequency(int dutyCycle) {
  return round(dutyCycle * (float) detectedMaxFrequency / 255);
}

int normalizeFrequencyToDutyCycle(float frequency) {
  return round(frequency * (255.0 / detectedMaxFrequency));
}

int normalizeSpeedToDutyCycle(int speed) {
  return map(speed, 0, 1023, 0, 255);
}

void setSpeed(int speedValue) {
  analogWrite(SET_SPEED_PIN, speedValue);
}

bool start(int getSpeedValue) {
  int getSpeedValueNormalized;
  int minStartValue = motorStartBias * 3;

  getSpeedValueNormalized = normalizeSpeedToDutyCycle(getSpeedValue); 

  Serial.println("start func");

  if (getSpeedValueNormalized < minStartValue) {
    getSpeedValueNormalized = minStartValue;
  }

  if (speedDutyCycle < getSpeedValueNormalized && getSpeedValueNormalized - speedDutyCycle > motorStartBias) {
    if (speedDutyCycle == 0) {
      speedDutyCycle = minStartValue;
    }

    speedDutyCycle = speedDutyCycle + motorStartBias;
    setSpeed(speedDutyCycle);

    return false;
  } else {
    speedDutyCycle = getSpeedValueNormalized;

    setSpeed(speedDutyCycle);

    return true;
  }
}

float pid(int actualPoint, int setPoint, float p, float i, float d, int iterationTime) {
  float error, integral, derrivative, output;

  error = setPoint - actualPoint;
  // Serial.println("er");
  // Serial.println(setPoint);
  // Serial.println(actualPoint);
  // Serial.println(error);
  integral = previousIntegralError + error * iterationTime;
  derrivative = (error - previousDerrivativeError) / iterationTime;
  output = p * error + i * integral + d * derrivative;
  previousDerrivativeError = derrivative;
  previousIntegralError = integral;

  return output;
}

int calculateSpeed(int actualFrequency, int desiredFrequency, int previousDutyCycle, int measurmentInterval) {
  int dutyCycle, dutyCycleAdjustNormalized;
  float frequencyAdjust;

  // frequencyAdjust = pid(actualFrequency, desiredFrequency, 0.18, 0.0005, 0.0005, measurmentInterval);
  frequencyAdjust = pid(actualFrequency, desiredFrequency, 0.18, 0, 0, measurmentInterval);
  dutyCycleAdjustNormalized = normalizeFrequencyToDutyCycle(frequencyAdjust);

  // Serial.println("out");
  // Serial.println(frequencyAdjust);
  // Serial.println(dutyCycleAdjustNormalized);

  dutyCycle = previousDutyCycle + dutyCycleAdjustNormalized;

  if (dutyCycleAdjustNormalized >= 0) {
    dutyCycle = (int) ceil(dutyCycle);
  } else {
    dutyCycle = (int) floor(dutyCycle);
  }

  // Serial.println("sp");
  // Serial.println(dutyCycle);

  dutyCycle = constrain(dutyCycle, 0, 255);

  return dutyCycle;
}

void loop() {
  // Serial.println("START");
  unsigned long currentMillis = millis();
  unsigned int tachoPulseHighDuration, tachoPulseLowDuration;
  int getEnableValue, getReverseValue, getSpeedValue, desiredFrequency, speedCalculationInterval, frequencyOnSpindle;

  float frequency;

  getEnableValue = digitalRead(ENABLE_PIN);
  getReverseValue = digitalRead(GET_REVERSE_PIN);
  getSpeedValue = analogRead(GET_SPEED_PIN);

  tachoPulseHighDuration = pulseIn(TACHO_PIN, HIGH, 6000);
  tachoPulseLowDuration = speedCalculationInterval = pulseIn(TACHO_PIN, LOW, 6000);

  // Serial.println("pulses");
  // Serial.println(tachoPulseHighDuration);
  // Serial.println(tachoPulseLowDuration);

  frequency = calculateFrequency(tachoPulseHighDuration + tachoPulseLowDuration);
  frequencyOnSpindle = calculateFrequencyOnSpindle(frequency);

  if (detectedMaxFrequency < frequency) {
    detectedMaxFrequency = frequency;
  }

  if (getEnableValue == 1) {
    isEnable = true;
  } else {
    isEnable = false;

    if (isOverloaded) {
      isOverloaded = false;
    }

    motorBrakes(true);
  }

  // Serial.print("is overloaded val: ");
  // Serial.println(isOverloaded);

  if (getSpeedValue <= 15) {
    isEnable = false;

    if (isOverloaded) {
      isOverloaded = false;
    }

    motorBrakes(true);
  }

  if (getReverseValue == 0) { //0 is true because of pullup input
    reverse = true;
  } else {
    reverse = false;
  }

  if (frequency == 0) {
    if (notRunningCount == notRunningMaxCount && isRunning) {
      Serial.println("freq 0 setting isRunning false");
      isRunning = false;
      notRunningCount = 0;
    }
    
    notRunningCount++;
  } else {
    if (notRunningCount > 0) {
      notRunningCount = 0;
    }
  }

  if (tachoPulseHighDuration == 0 && tachoPulseLowDuration == 0) {
    if (!isOverloaded && isRunning) {
      if (overloadsCount >= overloadMaxTries) {
        isOverloaded = true;
        overloadsCount = 0;

        motorBrakes(true, "on overloaded");
      }

      if (isEnable) {
        overloadsCount++;
      }
    }
  } else {
    if (overloadsCount > 1) {
      overloadsCount = 0;
    }
  }

  if (tachoPulseHighDuration == 0) {
    tachoPulseHighDuration = prevTachoHighPulse;
  } else {
    prevTachoHighPulse = tachoPulseHighDuration;
  }

  if (tachoPulseLowDuration == 0) {
    tachoPulseLowDuration = speedCalculationInterval = prevTachoLowPulse;
  } else {
    prevTachoLowPulse = tachoPulseLowDuration;
  }

  if (!isOverloaded) {
    if (isEnable) {
      // Serial.println("enable not overloaded");

      if (!isRunning) {
        motorBrakes(false, "off enable not running");

        if (reverse) {
          setMotorToReverse(true);
        } else {
          setMotorToReverse(false);
        }

        isRunning = start(getSpeedValue);
      } else {
        // Serial.print("is running true");

        if (reverse) {
          if (!isReversed) {
            motorBrakes(true, "on enable running reverse not reversed");
          }
        } else {
          if (isReversed) {
            motorBrakes(true, "on enable running no reverse reversed");
          }
        }

        if (reverse == isReversed) {
          Serial.println("workingLogic");
          desiredFrequency = normalizeSpeedToFrequency(getSpeedValue);

          if (desiredFrequency < minFrequency) {
            desiredFrequency = minFrequency;
          }

          speedDutyCycle = calculateSpeed(frequency, desiredFrequency, speedDutyCycle, speedCalculationInterval);

          setSpeed(speedDutyCycle);
        }
      }
    } else {
      motorBrakes(true, "on not enabled not running");
    }
  } else {
    if (reverse) {
      motorBrakes(false, "off overloaded reverse unblock");
      setSpeed(125);      
    }

    if (frequency > 0) {
      isOverloaded = false;
    }
  }

  if (isOverloaded) {
    overloadLedSetBehavior(1);
  } else if (noReservePower) {
    overloadLedSetBehavior(2);
  } else {
    overloadLedSetBehavior(0);
  }

  if (prevSpeedValue != getSpeedValue) {
    prevSpeedValue = getSpeedValue;

    displayRender(frequencyOnSpindle);
  }
}
