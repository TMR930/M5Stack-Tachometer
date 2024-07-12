#include <M5Stack.h>

hw_timer_t* timer = NULL;

const int IR_pin = 21;

unsigned long cnt = 0;
boolean state = 0;

// 600msのカウント数を1分間当たりに変換
void Counter() {
  if (cnt) {
    cnt = (cnt * 100) / 20;
  }

  M5.Lcd.fillScreen(BLACK);

  M5.Lcd.setTextFont(0);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(30, 10);
  M5.Lcd.drawRightString(String(cnt), 290, 40, 7);

  M5.Lcd.setCursor(190, 200);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setTextFont(0);
  M5.Lcd.println("(r/min)");

  cnt = 0;
}

void setup() {
  pinMode(IR_pin, INPUT_PULLUP);

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
  int data = 0;

  data = digitalRead(IR_pin);
  if (data == 0 && state == 1) {
    state = 0;
    cnt++;
  } else if (data == 1 && state == 0) {
    state = 1;
  }
}
