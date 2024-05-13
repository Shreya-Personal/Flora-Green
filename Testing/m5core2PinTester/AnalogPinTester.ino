/**
 * @file AnalogPinTester.ino
 * @brief Reads input pins and calculates LED brightness and petal status.
 * 
 * This Arduino sketch defines constants for sample duration and pin numbers, includes the necessary libraries, 
 * and implements functions to sample PWM signals from specified pins. It calculates the duty cycle of the PWM signal, 
 * maps it to a 0-10 range for LED brightness, and determines the petal status based on another pin's PWM signal.
 * 
 * @author Shreya S
 * @date 7/05/2024
 */

#define SAMPLE_DURATION_MS 200
#define LED_PIN A0  // Change this to your PWM pin number
#define PETAL_PIN A1 

#include <math.h>
unsigned long startTime;
unsigned long totalHighTime;
int totalCount;

int samplePin(int PWM_PIN){ 
  startTime = millis();  // Get the current time
  totalHighTime = 0;
  totalCount = 0;
  int totalval = 0; 
  // Sample the PWM signal for the specified duration
  while (millis() - startTime < SAMPLE_DURATION_MS) {
    int pwmValue = digitalRead(PWM_PIN);
    if (pwmValue == HIGH) {
      totalHighTime += 1;  // Increment high time counter
    }
    totalCount += 1;  // Increment total count
    
    delayMicroseconds(10);  // Optional small delay for stability
  }

  // Calculate duty cycle as percentage (0-100)
  float dutyCycle = (float)totalHighTime / totalCount * 100.0;
  return dutyCycle;
}

int GetPWM(int pin)
{
  unsigned long highTime = pulseIn(pin, HIGH, 50000UL);  // 50 millisecond timeout
  unsigned long lowTime = pulseIn(pin, LOW, 50000UL);  // 50 millisecond timeout

  // pulseIn() returns zero on timeout
  if (highTime == 0 || lowTime == 0)
    return digitalRead(pin) ? 100 : 0;  // HIGH == 100%,  LOW = 0%

  return (100 * highTime) / (highTime + lowTime);  // highTime as percentage of total cycle time
}

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  pinMode(PETAL_PIN, INPUT);
  pinMode(LED_PIN, INPUT);  // Set the PWM pin as input
}

void loop() {
  int PWM = GetPWM(LED_PIN);
  // Map the duty cycle to a 0-10 range
  int led_brightness = map(PWM, 0, 100, 0, 10);
  int dutyCycle = samplePin(PETAL_PIN);

  int petal = map(dutyCycle, 0, 30, 0, 1); 
  if (petal > 1) { 
    petal = 1; 
  }
  Serial.print("%, led_brightness: ");
  Serial.println(led_brightness);
  Serial.print("%, Petal: ");
  Serial.println(petal);

  delay(1000);  // Delay before next sampling
}
