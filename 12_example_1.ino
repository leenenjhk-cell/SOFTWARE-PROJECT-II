#include <Servo.h>
#include <math.h>

#define PIN_TRIG 12
#define PIN_ECHO 13
#define PIN_SERVO 10

// 기존에 정상 작동한 서보 설정 유지
#define _DUTY_MIN 400
#define _DUTY_MAX 2400

#define _POS_START (_DUTY_MIN + 100)
#define _POS_END   (_DUTY_MAX - 100)

// 1 = Sigmoid, 2 = Smoothstep
#define EASING_MODE 2

// 차량 감지 설정
const float DETECT_DISTANCE = 35.0;

const unsigned long CLOSE_DELAY = 2000;
const unsigned long MOVE_DURATION = 1500;

Servo myservo;

// 차단기 시작 위치와 목표 위치
const int CLOSED_DUTY = _POS_START;
const int OPEN_DUTY = _POS_END;

float dutyCurr = CLOSED_DUTY;
float dutyStart = CLOSED_DUTY;
float dutyTarget = CLOSED_DUTY;

bool isMoving = false;

unsigned long moveStartTime = 0;
unsigned long lastVehicleSeen = 0;
unsigned long lastPrintTime = 0;


// 초음파 거리 측정
float readDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration =
      pulseIn(PIN_ECHO, HIGH, 12000UL);

  if (duration == 0) {
    return -1.0;
  }

  float distance = duration * 0.0343 / 2.0;

  // 유효 범위 밖의 값은 무시
  if (distance < 2.0 || distance > 200.0) {
    return -1.0;
  }

  return distance;
}


// Sigmoid 함수
float sigmoidEase(float t) {
  const float k = 10.0;

  float low = 1.0 / (1.0 + expf(k * 0.5));
  float high = 1.0 / (1.0 + expf(-k * 0.5));

  float y = 1.0 / (1.0 + expf(-k * (t - 0.5)));

  return (y - low) / (high - low);
}


// Smoothstep 함수
float smoothstepEase(float t) {
  return t * t * (3.0 - 2.0 * t);
}


// 사용할 함수 선택
float easingFunction(float t) {
#if EASING_MODE == 1
  return sigmoidEase(t);
#else
  return smoothstepEase(t);
#endif
}


// 서보 이동 시작
void startMove(int target) {
  if (target == dutyTarget) {
    return;
  }

  dutyStart = dutyCurr;
  dutyTarget = target;

  moveStartTime = millis();
  isMoving = true;
}


// 부드럽게 서보 제어
void updateServo() {
  if (!isMoving) {
    return;
  }

  float t = (millis() - moveStartTime)
            / (float)MOVE_DURATION;

  if (t >= 1.0) {
    dutyCurr = dutyTarget;
    myservo.writeMicroseconds((int)roundf(dutyCurr));

    isMoving = false;
    return;
  }

  float eased = easingFunction(t);

  dutyCurr = dutyStart
             + (dutyTarget - dutyStart) * eased;

  // 기존에 정상 작동한 제어 방식 사용
  myservo.writeMicroseconds((int)roundf(dutyCurr));
}


void setup() {
  Serial.begin(57600);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_TRIG, LOW);

  // 기존 서보 코드와 동일하게 연결
  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(CLOSED_DUTY);

  Serial.println("Parking barrier ready");
}


void loop() {
  unsigned long now = millis();

  float distance = readDistance();

  // 차량이 35cm 이내에 있는지 확인
  bool vehicleDetected =
      distance >= 2.0 &&
      distance <= DETECT_DISTANCE;

  if (vehicleDetected) {
    lastVehicleSeen = now;

    // 차량 감지: 차단기 올리기
    if (dutyTarget != OPEN_DUTY) {
      startMove(OPEN_DUTY);
      Serial.println("Vehicle detected -> OPEN");
    }
  } else {
    // 차량이 사라진 뒤 2초 후 차단기 내리기
    if (dutyTarget == OPEN_DUTY &&
        now - lastVehicleSeen >= CLOSE_DELAY) {
      startMove(CLOSED_DUTY);
      Serial.println("Vehicle passed -> CLOSE");
    }
  }

  // 매 루프에서 서보 위치 갱신
  updateServo();

  // 거리 출력
  if (now - lastPrintTime >= 250) {
    lastPrintTime = now;

    if (distance < 0) {
      Serial.println("Distance: NO ECHO");
    } else {
      Serial.print("Distance: ");
      Serial.print(distance, 2);
      Serial.println(" cm");
    }
  }

  delay(20);
}
