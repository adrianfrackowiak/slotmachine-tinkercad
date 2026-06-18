#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const int DS    = 10;
const int SHCP  = 6;
const int STCP  = 9;

const int BTN_SPIN = 7;
const int BUZZER   = 8;
const int LED_WIN  = 13;

const byte DIGITS[6] = {
  0b00000110,
  0b01011011,
  0b01001111,
  0b01100110,
  0b01101101,
  0b01111101
};

int reels[3] = {1, 1, 1};
int credits  = 10;

enum State { IDLE, SPINNING, WIN, GAMEOVER };
State state = IDLE;

void setup() {
  lcd.begin(16, 2);

  pinMode(DS, OUTPUT);
  pinMode(SHCP, OUTPUT);
  pinMode(STCP, OUTPUT);

  pinMode(BTN_SPIN, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_WIN, OUTPUT);

  randomSeed(analogRead(A0));
  showDisplay();
  updateLCD();
}

void shiftOutAll(byte r1, byte r2, byte r3) {
  digitalWrite(STCP, LOW);
  shiftOut(DS, SHCP, MSBFIRST, r3);
  shiftOut(DS, SHCP, MSBFIRST, r2);
  shiftOut(DS, SHCP, MSBFIRST, r1);
  digitalWrite(STCP, HIGH);
}

void showDisplay() {
  shiftOutAll(DIGITS[reels[0]-1], DIGITS[reels[1]-1], DIGITS[reels[2]-1]);
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (state == IDLE)     lcd.print("Wcisnij SPIN");
  if (state == SPINNING) lcd.print("Kreci sie...");
  if (state == WIN)      lcd.print("*** WYGRANA! ***");
  if (state == GAMEOVER) lcd.print("  GAME OVER");
  lcd.setCursor(0, 1);
  lcd.print("Kredyty: ");
  lcd.print(credits);
}

void animateSpin() {
  for (int i = 0; i < 20; i++) {
    byte r1 = DIGITS[random(6)];
    byte r2 = DIGITS[random(6)];
    byte r3 = DIGITS[random(6)];
    shiftOutAll(r1, r2, r3);
    delay(60 + i * 8);
  }
}

int checkWin() {
  if (reels[0] == reels[1] && reels[1] == reels[2]) {
    return (reels[0] == 6) ? 50 : 20;
  }
  if (reels[0]==reels[1] || reels[1]==reels[2] || reels[0]==reels[2])
    return 5;
  return 0;
}

void winEffect() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_WIN, HIGH);
    tone(BUZZER, 880, 100);
    delay(150);
    digitalWrite(LED_WIN, LOW);
    delay(150);
  }
}

void loop() {
  bool spinPressed = (digitalRead(BTN_SPIN) == HIGH);

  if (state == GAMEOVER) {
    if (spinPressed) {
      credits = 10;
      state = IDLE;
      updateLCD();
      delay(300);
    }
    return;
  }

  if (state == IDLE && spinPressed && credits > 0) {
    credits--;
    state = SPINNING;
    updateLCD();

    animateSpin();

    for (int i = 0; i < 3; i++) reels[i] = random(1, 7);
    showDisplay();

    int prize = checkWin();
    credits += prize;

    if (prize > 0) {
      state = WIN;
      winEffect();
    } else {
      state = IDLE;
    }

    if (credits <= 0) state = GAMEOVER;
    updateLCD();
    delay(300);

  } else if (state == WIN && spinPressed) {
    state = IDLE;
    digitalWrite(LED_WIN, LOW);
    updateLCD();
    delay(200);
  }
}
