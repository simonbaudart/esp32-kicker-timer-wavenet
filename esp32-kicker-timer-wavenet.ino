#include <ezButton.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4  // 4 modules 8x8 = 32x8 pixels

#define PANEL_DIN 2
#define PANEL_CLK 4
#define PANEL_CS 7

MD_Parola display = MD_Parola(HARDWARE_TYPE, PANEL_DIN, PANEL_CLK, PANEL_CS, MAX_DEVICES);
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, PANEL_DIN, PANEL_CLK, PANEL_CS, MAX_DEVICES);

#define PANEL_BUZZER 5

#define BUTTON_RED 10
#define BUTTON_GREEN 18
#define BUTTON_BLUE 21

ezButton buttonRed(BUTTON_RED);
ezButton buttonGreen(BUTTON_GREEN);
ezButton buttonBlue(BUTTON_BLUE);

unsigned long timerDuration = 8 * 60;
unsigned long timerRemaining = timerDuration;
unsigned long timerRemainingDisplay = 0;
unsigned long timerStarted = 0;
unsigned long lastBeepSecond = 0;
bool running = false;

bool adjustMode = false;
unsigned long adjustStep = 10;

void countdownStart() {
  timerRemaining = timerDuration;
  timerStarted = millis();
  running = true;

  whistleStart();
  lastBeepSecond = 0;
}

void countdownStop() {
  timerStarted = 0;
  running = false;
}

void countdown() {
  if (running == false) {
    return;
  }

  unsigned long ellapsed = (millis() - timerStarted) / 1000;
  if (ellapsed >= timerDuration) {
    timerRemaining = 0;
    running = false;

    whistleEnd();
  } else {
    timerRemaining = timerDuration - ellapsed;

    if (timerRemaining <= 10 && timerRemaining != lastBeepSecond) {
      whistleNearEnd();
      lastBeepSecond = timerRemaining;
    }
  }
}

void countdownDisplay() {
  if (timerRemaining == timerRemainingDisplay) {
    return;
  }

  Serial.println("Timer Remaining : " + timerRemaining);
  timerRemainingDisplay = timerRemaining;

  display.displayClear();

  if (timerRemaining == 0) {
    display.displayText("FINISHED", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    drawProgressBar(1.0);
  } else {
    int minutes = timerRemaining / 60;
    int seconds = timerRemaining % 60;
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);

    display.displayText(buf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

    float percent = 1.0 - ((float)timerRemaining / timerDuration);
    drawProgressBar(percent);
  }

  display.displayAnimate();
}

void displayAdjustment() {
  display.displayClear();

  int minutes = timerDuration / 60;
  int seconds = timerDuration % 60;
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);

  display.displayText(buf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

  float percent = 0;
  drawProgressBar(percent);

  display.displayAnimate();
}

void drawProgressBar(float percent) {
  int totalCols = MAX_DEVICES * 8;  // 32 columns
  int filledCols = (int)(percent * totalCols);

  for (int col = 0; col < totalCols; col++) {
    mx.setPoint(7, col, col < filledCols);  // bottom row = 7
  }
}

void whistleStart() {
  // Rapid high-pitch pulses like a whistle
  for (int i = 0; i < 3; i++) {
    tone(PANEL_BUZZER, 3000, 150);  // 3kHz, 150ms
    delay(200);                     // short pause
  }
  noTone(PANEL_BUZZER);
}

void whistleEnd() {
  tone(PANEL_BUZZER, 1000, 2000);
}

void whistleNearEnd() {
  tone(PANEL_BUZZER, 2000, 100);
}

void setup() {
  Serial.begin(115200);

  buttonRed.setDebounceTime(50);
  buttonGreen.setDebounceTime(50);
  buttonBlue.setDebounceTime(50);

  mx.begin();

  display.begin();
  display.setIntensity(5);
  display.displayClear();

  pinMode(PANEL_BUZZER, OUTPUT);
}

void loop() {

  if (buttonBlue.isPressed()) {
    adjustMode = true;
  } else {
    adjustMode = false;
  }

  if (adjustMode) {
    if (buttonRed.isPressed()) {
      if (timerDuration >= adjustStep) timerDuration -= adjustStep;
    }
    if (buttonGreen.isPressed()) {
      timerDuration += adjustStep;
    }

    displayAdjustment();
  } else {
    if (buttonGreen.isPressed()) {
      countdownStart();
    }

    if (buttonRed.isPressed()) {
      countdownStop();
    }

    countdown();
    countdownDisplay();
  }
}
