#include <ezButton.h>

#define PANEL_DIN 6
#define PANEL_CLK 4
#define PANEL_CS 7

#define PANEL_BUZZER 0

#define BUTTON_RED 1
#define BUTTON_GREEN 2
#define BUTTON_BLUE 3

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
