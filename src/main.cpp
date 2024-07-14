#include <M5Stack.h>

hw_timer_t* timer = NULL;

const int IN1_pin = 16;
const int IN2_pin = 17;
const int ENA_pin = 2;
const int IR_pin = 21;

boolean state = 0;
volatile long encoderValue = 0;
double setpoint = 700.0;  // 目標速度 (RPM)
double input, output;
double Kp = 0.5, Ki = 0.05, Kd = 0.05;
double integral = 0, previous_error = 0;
unsigned long speed = 0;

// 600msのカウント数を1分間当たりに変換
void Counter() {
  if (encoderValue) {
    speed = (encoderValue * 100) / 20;
  } else {
    speed = 0;
  }
  input = speed;

  // M5.Lcd.fillScreen(BLACK);

  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(30, 10);
  M5.Lcd.drawRightString(String(speed), 225, 40, 7);

  M5.Lcd.setCursor(190, 120);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextFont(0);
  M5.Lcd.println("(r/min)");

  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("target:%d r/min", int(setpoint));

  encoderValue = 0;
}

void setup() {
  pinMode(IR_pin, INPUT_PULLUP);
  pinMode(IN1_pin, OUTPUT);
  pinMode(IN2_pin, OUTPUT);
  pinMode(ENA_pin, OUTPUT);

  Serial.begin(155200);

  M5.begin();
  M5.Lcd.fillScreen(BLACK);            // 画面をクリア
  M5.Lcd.setTextColor(YELLOW, BLACK);  // 文字色と背景色

  timer = timerBegin(0, 80, true);  // タイマ作成
  timerAttachInterrupt(timer, &Counter,
                       true);  // タイマ割り込みサービス・ルーチンを登録
  timerAlarmWrite(timer, 600000, true);  // 割り込みタイミング(600ms)の設定
  timerAlarmEnable(timer);               // タイマ有効化
}

void loop() {
  static long lastTime = 0;
  long currentTime = millis();
  int data = 0;

  digitalWrite(IN1_pin, HIGH);
  digitalWrite(IN2_pin, LOW);

  data = digitalRead(IR_pin);
  if (data == 0 && state == 1) {
    state = 0;
    encoderValue++;
  } else if (data == 1 && state == 0) {
    state = 1;
  }

  if (currentTime - lastTime >= 100) {  // 100ミリ秒ごとに更新
    // PID制御
    double error = setpoint - input;
    integral += error * (currentTime - lastTime) / 1000.0;
    double derivative =
        (error - previous_error) / ((currentTime - lastTime) / 1000.0);
    output = Kp * error + Ki * integral + Kd * derivative;

    previous_error = error;

    // モーター制御
    output -= 255;
    output = output * -1;
    // if (output > 255) output = 255;
    if (output < 0) output = 0;

    if (output >= 0 && output <= 255) {
      analogWrite(ENA_pin, output);
    }

    // シリアルモニタに出力
    Serial.print("Speed: ");
    Serial.print(speed);
    Serial.print(" RPM, Output: ");
    Serial.print(output);
    Serial.print(" , Integral: ");
    Serial.print(integral);
    Serial.print(" , Derivative: ");
    Serial.println(derivative);

    lastTime = currentTime;
  }
}
