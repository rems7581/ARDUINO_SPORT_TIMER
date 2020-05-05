/*

Wiring:

LCD:        Rotary:     Buzzer:
SCL > A5    CLK > D2    +  > D8
SDA > A4    DT  > D3    -  > 100 ohm > GND
            SW  > D4



*/





#include <Wire.h>
#include <RotaryEncoder.h>
#include <LiquidCrystal_I2C.h>

#define X1 1
#define Y1 0
#define X2 1
#define Y2 1
#define XSEC 8
#define XGO 13
#define YGO 0

#define MENUMIN 0
#define MENUMAX 2
#define T1MIN 10
#define T1MAX 300
#define T2MIN 10
#define T2MAX 300

LiquidCrystal_I2C lcd(0x27,20,4);
RotaryEncoder encoder(3, 2);

int swCt = 0; // Switch counter

int lastPosMenu = 0;
int lastPosT1 = T1MIN;
int lastPosT2 = T2MIN;

bool doneFlag = false; // To execute some code once

int c = 0; // timer
const int period = 250; // timer
unsigned long time_now = 0; // timer
unsigned long time_now2 = 0; // timer 2









void setup() {
  Serial.begin(115200);
  lcd.init();
  pinMode(4, INPUT);
  lcd.backlight();
  lcd.clear();
  // Print home page
  lcdPrint(X1, Y1, "Time1:");
  lcdPrint(X2, Y2, "Time2:");
  lcdPrint(XGO, YGO, "Go!");
  // Initial Arrow
  printArrow(0);
}


void loop() {
  if (digitalRead(4) == LOW && swCt == 0) { // switch press to go in sub-menu
    swCt = 1;
    doneFlag = false;
    delay(200);
  }
  if (digitalRead(4) == LOW && swCt == 1) { // switch press to go back in menu
    swCt = 0;
    doneFlag = false;
    delay(200);
  }
  if (swCt == 0) {
    if (doneFlag == false) {
      encoder.setPosition(lastPosMenu); // Reset rotary to lastPosMenu (if not, arrow jump to last variable set)
      printArrow(lastPosMenu); // Drawing Arrow at the first run
      secPrint(0, lastPosT1); // Avoid stopping blinking on blank
      secPrint(1, lastPosT2); // Same
      doneFlag = true;
    }
    // Menu Arrow
    lastPosMenu = changeVal(255, lastPosMenu, MENUMIN, MENUMAX);
  }
  if (swCt == 1 && lastPosMenu == 0) {
    // Change speed
    lastPosT1 = changeVal(0, lastPosT1, T1MIN, T1MAX);
  }
  if (swCt == 1 && lastPosMenu == 1) {
    // Change step
    lastPosT2 = changeVal(1, lastPosT2, T2MIN, T2MAX);
  }
  if (swCt == 1 && lastPosMenu == 2) {
    // Running
    if (doneFlag == false) {
      Serial.println("Running");
      runTimer(lastPosT1, lastPosT2);
      doneFlag = true;
      swCt = 0;
    }
  }
}



void lcdPrint(int x, int y, String str) {
  lcd.setCursor(x, y);
  lcd.print(str);
}

void printArrow(int y) {
  switch (y) {
    case 0:
      lcdPrint(0, 0, ">");
      lcdPrint(0, 1, " ");
      lcdPrint(12, 0, " ");
      break;
    case 1:
      lcdPrint(0, 0, " ");
      lcdPrint(0, 1, ">");
      lcdPrint(12, 0, " ");
      break;
    case 2:
      lcdPrint(0, 0, " ");
      lcdPrint(0, 1, " ");
      lcdPrint(12, 0, ">");
      break;
  }
}

void secPrint (int item, int sec) {
  String sec_str = String(sec);
  int len = sec_str.length();
  for (int i = 0; i < 5 - len; i++) {
    sec_str += " ";
    i++;
  }
  lcdPrint(XSEC, item, sec_str);
}


void runTimer (int t1, int t2) {
  tone(8, 1000, 500);
  delay(1000);
  tone(8, 1000, 500);
  delay(1000);
  tone(8, 1500, 500);
  while (digitalRead(4) != LOW) {
    time_now = millis();
    time_now2 = millis();
    int sec1 = lastPosT1;
    while (millis()/1000 <= time_now/1000 + t1) { // Timer
      if (digitalRead(4) == LOW) {
        break;
      }
      if (millis() >= time_now2 + 1000) {
        time_now2 = millis();
        sec1--;
        secPrint(0, sec1);
      }
    }
    secPrint(0, lastPosT1);
    tone(8, 1500, 200);
    delay(400);
    tone(8, 1500, 200);
    time_now = millis();
    time_now2 = millis();
    int sec2 = lastPosT2;
    while (millis()/1000 <= time_now/1000 + t2) { // Timer
      if (digitalRead(4) == LOW) {
        break;
      }
      if (millis() >= time_now2 + 1000) {
        time_now2 = millis();
        sec2--;
        secPrint(1, sec2);
      }
    }
    secPrint(1, lastPosT2);
    tone(8, 1000, 200);
    delay(400);
  }
  lastPosMenu = 0;
  swCt = 0;
  doneFlag = false;
  delay(1000);

}

int changeVal(int item, int lastPos, int MIN, int MAX) {
  if (doneFlag == false) {
    encoder.setPosition(lastPos);
    doneFlag = true;
    c = 0;
    time_now = millis();
  }
  encoder.tick();
  // Blinking value
  if (item != 255) {
    if (millis() >= time_now + period) { // Timer
      time_now += period;
      if ((c % 2) == 0) {
        secPrint(item, lastPos);
      }
      else {
        lcdPrint(XSEC, item, "     ");
      }
      c++;
    }
  }
  int newPos = encoder.getPosition();
  if (newPos < MIN) {
    encoder.setPosition(MAX);
    newPos = MAX;
  }
  else if (newPos > MAX) {
    encoder.setPosition(MIN);
    newPos = MIN;
  }
  if (lastPos != newPos) {
    lastPos = newPos;
    if (item == 255) {
      printArrow(lastPos);
    }
    else {
      secPrint(item, lastPos);
    }
  }
  return lastPos;
}
