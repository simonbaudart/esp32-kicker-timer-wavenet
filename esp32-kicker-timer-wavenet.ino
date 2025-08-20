#include <ezButton.h>
#include <MD_Parola.h>
#include "Font5x7Fixed.h"
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Preferences.h>

#define PREF_KEY_TIMER_DURATION "timerDuration"
Preferences prefs;
unsigned long storedTimerDuration = 0;

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4  // 4 modules 8x8 = 32x8 pixels

#define PANEL_DIN 6
#define PANEL_CLK 4
#define PANEL_CS 7

MD_Parola display = MD_Parola(HARDWARE_TYPE, PANEL_DIN, PANEL_CLK, PANEL_CS, MAX_DEVICES);
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, PANEL_DIN, PANEL_CLK, PANEL_CS, MAX_DEVICES);

#define BUZZER 0
#define NOTE_G5 784
#define NOTE_E5 659
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_F5 698

#define BUTTON_RED 1
#define BUTTON_GREEN 3
#define BUTTON_BLUE 10

ezButton buttonRed(BUTTON_RED);
ezButton buttonGreen(BUTTON_GREEN);
ezButton buttonBlue(BUTTON_BLUE);

unsigned long timerDuration = 15;
unsigned long timerRemaining = timerDuration;
unsigned long timerRemainingDisplay = 0;
unsigned long timerStarted = 0;
unsigned long lastBeepSecond = 0;
bool running = false;

bool adjustMode = false;
unsigned long adjustStep = 5;

void countdownStart() {
  Serial.println("Starting Countdown !");
  saveTimerDuration();

  timerRemaining = timerDuration;
  timerStarted = millis();
  running = true;

  whistleStart();
  lastBeepSecond = 0;
}

void countdownStop() {
  Serial.println("Stop Countdown !");
  timerStarted = 0;
  running = false;
  whistleBlurp();
  displayReady();
}

void countdown() {
  if (running == false) {
    return;
  }

  unsigned long ellapsed = (millis() - timerStarted) / 1000;
  if (ellapsed >= timerDuration) {
    timerRemaining = 0;

    countdownDisplay();
    whistleEnd();

    running = false;
  } else {
    timerRemaining = timerDuration - ellapsed;

    if (timerRemaining <= 10 && timerRemaining != lastBeepSecond) {
      whistleNearEnd();
      lastBeepSecond = timerRemaining;
    }
  }
}

void countdownDisplay() {

  if (!running) {
    return;
  }

  if (timerRemaining == timerRemainingDisplay) {
    return;
  }
  timerRemainingDisplay = timerRemaining;

  display.displayClear();

  if (timerRemaining == 0) {
    display.displayText("DONE !!!", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    displayAnimate();
  } else {
    int minutes = timerRemaining / 60;
    int seconds = timerRemaining % 60;
    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);
    Serial.println(buf);

    display.displayText(buf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

    //   float percent = 1.0 - ((float)timerRemaining / timerDuration);
    //   drawProgressBar(percent);

    displayAnimate();
  }
}

void displayAnimate() {
  while (!display.displayAnimate()) {
    display.displayAnimate();
  }
}

void displayAdjustment() {
  display.displayClear();

  int minutes = timerDuration / 60;
  int seconds = timerDuration % 60;
  char buf[6];
  snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);

  display.displayText(buf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

  displayAnimate();
}

void displayReady() {
  display.displayClear();
  display.displayText("READY", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  displayAnimate();
}

void displayWavenet() {
  display.displayClear();
  display.displayText("Wavenet Rules !!!", PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);

  displayAnimate();

  display.displayClear();
}

void drawProgressBar(float percent) {
  int totalCols = MAX_DEVICES * 8;  // 32 columns
  int filledCols = (int)(percent * totalCols);

  for (int col = 0; col < totalCols; col++) {
    mx.setPoint(7, col, col < filledCols);  // bottom row = 7
  }
}

void whistleStart() {
  tone(BUZZER, NOTE_G5, 100);  // "Let's" rapide
  delay(100);

  tone(BUZZER, NOTE_E5, 100);
  delay(100);

  noTone(BUZZER);
  delay(50);  // courte pause pour le rythme

  tone(BUZZER, NOTE_C5, 200);  // "Go!" marqué
  delay(200);

  tone(BUZZER, NOTE_D5, 150);  // montée finale
  delay(150);

  tone(BUZZER, NOTE_F5, 150);  // accent final
  delay(150);

  noTone(BUZZER);
}

void whistleEnd() {
  tone(BUZZER, 1000, 2000);
}

void whistleBlurp() {
  for (int freq = 800; freq >= 200; freq -= 100) {
    tone(BUZZER, freq, 120);  // play each step
    delay(150);
  }
  noTone(BUZZER);
}

void whistleNearEnd() {
  tone(BUZZER, 2000, 100);
}

void saveTimerDuration() {
  if (timerDuration != storedTimerDuration) {
    prefs.begin("timer", false);
    prefs.putUInt(PREF_KEY_TIMER_DURATION, timerDuration);
    prefs.end();
    storedTimerDuration = timerDuration;
    Serial.println("Timer Duration Persisted");
  }
}

void setup() {
  Serial.begin(115200);

  prefs.begin("timer", true);
  if (prefs.isKey(PREF_KEY_TIMER_DURATION)) {
    timerDuration = prefs.getUInt(PREF_KEY_TIMER_DURATION);
    storedTimerDuration = timerDuration;
  }
  prefs.end();

  buttonRed.setDebounceTime(100);
  buttonGreen.setDebounceTime(100);
  buttonBlue.setDebounceTime(100);

  mx.begin();

  display.begin();
  display.setIntensity(5);
  display.displayClear();

  pinMode(BUZZER, OUTPUT);

  whistleStart();
  displayWavenet();
  displayReady();
}

void loop() {
  buttonRed.loop();
  buttonBlue.loop();
  buttonGreen.loop();


  if (buttonBlue.isReleased()) {
    adjustMode = !adjustMode;
    if (adjustMode) {
      timerStarted = 0;
      running = false;
      adjustMode = true;
    }
  }

  if (adjustMode) {
    if (buttonRed.isReleased()) {
      if (timerDuration >= adjustStep) timerDuration -= adjustStep;
    }

    if (buttonGreen.isReleased()) {
      timerDuration += adjustStep;
    }

    displayAdjustment();
  } else {
    if (buttonGreen.isReleased()) {
      Serial.println("Button Green Released");
      countdownStart();
    }

    if (buttonRed.isReleased()) {
      Serial.println("Button Red Released");
      countdownStop();
    }

    countdown();
    countdownDisplay();
  }
}
