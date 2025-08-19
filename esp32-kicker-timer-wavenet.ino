#include <ezButton.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES   4   // 4 modules 8x8 = 32x8 pixels

#define PANEL_DIN 2
#define PANEL_CLK 4
#define PANEL_CS  7

MD_Parola display = MD_Parola(HARDWARE_TYPE, PANEL_DIN, PANEL_CLK, PANEL_CS, MAX_DEVICES);
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, PANEL_DIN, PANEL_CLK, PANEL_CS, MAX_DEVICES);

#define PANEL_BUZZER 5

#define BUTTON_RED 10
#define BUTTON_GREEN 18
#define BUTTON_BLUE 21

ezButton buttonRed(BUTTON_RED);
ezButton buttonGreen(BUTTON_GREEN);
ezButton buttonBlue(BUTTON_BLUE);

unsigned long timerDuration = 300;
unsigned long timerRemaining = timerDuration;
unsigned long timerRemainingDisplay = 0;
unsigned long timerStarted = 0;
bool running = false;

void countdownStart()
{
  //TODO : BUZZ !
  timerRemaining = timerDuration;
  timerStarted = millis();
  running = true;
}

void countdownStop()
{
  //TODO : BUZZ !
  timerStarted = millis();
  running = false;
}

void countdown()
{
  if (running == false)
  {
    return;
  }

  unsigned long ellapsed = timerStarted - millis();
  timerRemaining = timerDuration - (ellapsed / 1000);
}

void countdownDisplay()
{
  if (timerRemaining == timerRemainingDisplay)
  {
    return;
  }

  Serial.println("Timer Remaining : " + timerRemaining);
  timerRemainingDisplay = timerRemaining;
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
}

void loop() {
  if (buttonGreen.isPressed())
  {
    countdownStart();
  }

  if (buttonRed.isPressed())
  {
    countdownStop();
  }

  countdown();
  countdownDisplay();
}
