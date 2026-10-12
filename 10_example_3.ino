#include <Servo.h>
#include <math.h>

// 핀 설정
#define PIN_SERVO 10
#define PIN_TRIG  12
#define PIN_ECHO  13

// 거리 설정(cm)
#define DETECT_DISTANCE 20   // 20cm 이내에 차량이 들어오면 감지
#define PASS_DISTANCE   30   // 30cm보다 멀어지면 차량이 지나간 것으로 판단

// 차단기 각도
#define BAR_DOWN 30
#define BAR_UP   90

// 움직이는 시간
#define MOVING_TIME 2000     // 2초 동안 부드럽게 이동

Servo myServo;

int currentAngle = BAR_DOWN;
bool carDetected = false;

// --------------------------------------------------
// 초음파 센서 거리 측정
// --------------------------------------------------
float getDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long duration = pulseIn(PIN_ECHO, HIGH, 30000);

  if (duration == 0)
    return 999;

  return duration * 0.0343 / 2.0;
}

// --------------------------------------------------
// sigmoid 함수
// --------------------------------------------------
float sigmoid(float t) {
  float k = 10.0;

  float s = 1.0 / (1.0 + exp(-k * (t - 0.5)));

  // 0~1 범위로 다시 정규화
  float start = 1.0 / (1.0 + exp(-k * (0.0 - 0.5)));
  float end   = 1.0 / (1.0 + exp(-k * (1.0 - 0.5)));

  return (s - start) / (end - start);
}

// --------------------------------------------------
// 비교용 smoothstep 함수
// --------------------------------------------------
float smoothStep(float t) {
  return t * t * (3.0 - 2.0 * t);
}

// --------------------------------------------------
// 서보를 부드럽게 이동
// useSigmoid = true  -> sigmoid
// useSigmoid = false -> smoothstep
// --------------------------------------------------
void moveBar(int startAngle, int targetAngle, bool useSigmoid) {

  unsigned long startTime = millis();

  while (millis() - startTime <= MOVING_TIME) {

    float t = (float)(millis() - startTime) / MOVING_TIME;

    if (t > 1.0)
      t = 1.0;

    float progress;

    if (useSigmoid)
      progress = sigmoid(t);
    else
      progress = smoothStep(t);

    int angle = startAngle +
                (targetAngle - startAngle) * progress;

    myServo.write(angle);

    delay(15);
  }

  myServo.write(targetAngle);
  currentAngle = targetAngle;
}

// --------------------------------------------------
// 초기 설정
// --------------------------------------------------
void setup() {

  Serial.begin(9600);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  myServo.attach(PIN_SERVO);

  myServo.write(BAR_DOWN);
  currentAngle = BAR_DOWN;
}

// --------------------------------------------------
// 반복 실행
// --------------------------------------------------
void loop() {

  float distance = getDistance();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // 차량이 접근함
  if (!carDetected && distance <= DETECT_DISTANCE) {

    carDetected = true;

    // sigmoid 함수 사용
    moveBar(BAR_DOWN, BAR_UP, true);
  }

  // 차량이 지나감
  else if (carDetected && distance >= PASS_DISTANCE) {

    carDetected = false;

    // sigmoid 함수 사용
    moveBar(BAR_UP, BAR_DOWN, true);
  }

  delay(100);
}
