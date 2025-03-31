#include <Arduino.h>
#include <app.h>

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  delay(100);
  Serial.println("Serial.begin() called.");

}

void loop(void)
{
  // Serial.println("loop...");
}
