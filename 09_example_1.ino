// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10
#define _DIST_MIN 100
#define _DIST_MAX 300

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

// 중위수 필터의 샘플 개수
// 3, 10, 30 등으로 변경하여 실험 가능
#define N 30

// global variables
unsigned long last_sampling_time = 0;

float dist_list[N];
int sample_count = 0;

float dist_med;


// setup
void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_TRIG, LOW);

  Serial.begin(57600);
}


// loop
void loop() {
  float dist_raw;

  // 다음 샘플링 시간까지 대기
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // 초음파 센서로 거리 측정
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);


  // 최근 N개의 샘플 유지
  if (sample_count < N) {
    dist_list[sample_count] = dist_raw;
    sample_count++;
  }
  else {
    // 가장 오래된 값 제거
    for (int i = 0; i < N - 1; i++) {
      dist_list[i] = dist_list[i + 1];
    }

    // 가장 최근 측정값 추가
    dist_list[N - 1] = dist_raw;
  }


  // 현재까지 모인 샘플을 복사
  float temp[N];

  for (int i = 0; i < sample_count; i++) {
    temp[i] = dist_list[i];
  }


  // 오름차순 정렬
  for (int i = 0; i < sample_count - 1; i++) {
    for (int j = 0; j < sample_count - 1 - i; j++) {
      if (temp[j] > temp[j + 1]) {
        float swap = temp[j];
        temp[j] = temp[j + 1];
        temp[j + 1] = swap;
      }
    }
  }


  // 중위수 계산
  if (sample_count % 2 == 1) {
    // 홀수 개: 가운데 값
    dist_med = temp[sample_count / 2];
  }
  else {
    // 짝수 개: 가운데 두 값의 평균
    dist_med =
      (temp[sample_count / 2 - 1] +
       temp[sample_count / 2]) / 2.0;
  }


  // 출력
  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(",raw:");
  Serial.print(dist_raw);

  Serial.print(",median:");
  Serial.print(dist_med);

  Serial.print(",Max:");
  Serial.print(_DIST_MAX);

  Serial.println("");


  // 중위수 필터 결과를 기준으로 LED 제어
  if ((dist_med < _DIST_MIN) || (dist_med > _DIST_MAX))
    digitalWrite(PIN_LED, 1);
  else
    digitalWrite(PIN_LED, 0);


  // 다음 샘플링 시간
  last_sampling_time += INTERVAL;
}


// 초음파 거리 측정
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
