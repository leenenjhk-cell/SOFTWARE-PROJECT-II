// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

// configurable parameters
#define SND_VEL 346.0     // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25       // sampling interval (unit: msec)
#define PULSE_DURATION 10 // ultra-sound Pulse Duration (unit: usec)
#define _DIST_MIN 100.0   // minimum distance to be measured (unit: mm)
#define _DIST_MAX 300.0   // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (unit: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)    // coefficent to convert duration to distance

unsigned long last_sampling_time;   // unit: msec

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  // initialize serial port
  Serial.begin(57600);

  // 처음에는 LED OFF
  analogWrite(PIN_LED, 255);
}
 // 코드 설명 : pwd_value 라는 변수를 통해, 기존 0, 1로 LED를 단순히 제어했던 것에 반면
 // 거리에 따라 밝기가 변화할수있도록 했다.
void loop() {
  float distance;
  int pwm_value;

  // wait until next sampling time
  if (millis() < (last_sampling_time + INTERVAL))
    return;

  // 초음파 센서로 거리 측정
  distance = USS_measure(PIN_TRIG, PIN_ECHO);

  // 측정 실패 또는 측정 범위 밖
  if (distance == 0.0) {
    distance = _DIST_MAX + 10.0;
    pwm_value = 255;    // LED OFF
  }

  // 100mm보다 가까움
  else if (distance < _DIST_MIN) {
    distance = _DIST_MIN - 10.0;
    pwm_value = 255;    // LED OFF
  }

  // 300mm보다 멂
  else if (distance > _DIST_MAX) {
    distance = _DIST_MAX + 10.0;
    pwm_value = 255;    // LED OFF
  }

  // 정상 범위 100~300mm
  else {

    // 100 ~ 200mm
    if (distance <= 200.0) {

      // 100mm -> 255 (OFF)
      // 200mm ->   0 (MAX)
      pwm_value = (int)(255.0 - (distance - 100.0) * 2.55);
    }

    // 200 ~ 300mm
    else {

      // 200mm ->   0 (MAX)
      // 300mm -> 255 (OFF)
      pwm_value = (int)((distance - 200.0) * 2.55);
    }

    // PWM 범위를 0~255로 제한
    pwm_value = constrain(pwm_value, 0, 255);
  }

  // Active Low LED이므로 계산한 PWM 값 적용
  analogWrite(PIN_LED, pwm_value);

  // serial output
  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(", distance:");
  Serial.print(distance);

  Serial.print(", Max:");
  Serial.print(_DIST_MAX);

  Serial.print(", PWM:");
  Serial.print(pwm_value);

  Serial.println("");

  // update last sampling time
  last_sampling_time += INTERVAL;
}


// get a distance reading from USS. return value is in millimeter.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
