#include <Servo.h>

// Arduino pin assignment
#define PIN_IR    0
#define PIN_LED   9
#define PIN_SERVO 10

#define _DUTY_MIN 400
#define _DUTY_NEU 1400
#define _DUTY_MAX 2400

#define _DIST_MIN 100.0
#define _DIST_MAX 250.0

#define EMA_ALPHA 0.2
#define LOOP_INTERVAL 20

Servo myservo;
unsigned long last_loop_time;

float dist_prev = _DIST_MIN;
float dist_ema = _DIST_MIN;

void setup()
{
  pinMode(PIN_LED, OUTPUT);

  // 서보 펄스 범위를 명시적으로 설정
  myservo.attach(PIN_SERVO, _DUTY_MIN, _DUTY_MAX);
  myservo.writeMicroseconds(_DUTY_NEU);

  Serial.begin(1000000);
}

void loop()
{
  unsigned long time_curr = millis();

  int duty;
  float a_value, dist_raw;

  // Loop interval: 20ms
  if (time_curr < last_loop_time + LOOP_INTERVAL)
    return;

  last_loop_time += LOOP_INTERVAL;

  // IR sensor reading
  a_value = analogRead(PIN_IR);

  // ADC 값이 9 이하이면 계산하지 않음
  if (a_value > 9.0)
  {
    dist_raw = ((6762.0 / (a_value - 9.0)) - 4.0) * 10.0;
  }
  else
  {
    dist_raw = -1.0;
  }

  // Range filter: 100mm ~ 250mm
  if (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX)
  {
    dist_prev = dist_raw;
    digitalWrite(PIN_LED, HIGH);
  }
  else
  {
    dist_raw = dist_prev;
    digitalWrite(PIN_LED, LOW);
  }

  // EMA filter
  dist_ema =
      EMA_ALPHA * dist_raw
      + (1.0 - EMA_ALPHA) * dist_ema;

  // map() equivalent: distance -> servo duty
  duty = _DUTY_MIN
       + (dist_ema - _DIST_MIN)
       * (_DUTY_MAX - _DUTY_MIN)
       / (_DIST_MAX - _DIST_MIN);

  // Servo control
  myservo.writeMicroseconds(duty);

  // Serial Plotter output
  Serial.print("_DUTY_MIN:");
  Serial.print(_DUTY_MIN);

  Serial.print(",_DIST_MIN:");
  Serial.print(_DIST_MIN);

  Serial.print(",IR:");
  Serial.print(a_value);

  Serial.print(",dist_raw:");
  Serial.print(dist_raw);

  Serial.print(",ema:");
  Serial.print(dist_ema);

  Serial.print(",servo:");
  Serial.print(duty);

  Serial.print(",_DIST_MAX:");
  Serial.print(_DIST_MAX);

  Serial.print(",_DUTY_MAX:");
  Serial.print(_DUTY_MAX);

  Serial.println("");
}
