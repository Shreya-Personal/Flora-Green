#include <Adafruit_CircuitPlayground.h>
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
#define LED_PIN A6  // Change this to your PWM pin number
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
    int pwmValue = analogRead(PWM_PIN);
    if (pwmValue > 250) {
      totalHighTime += 1;  // Increment high time counter
    }
    totalCount += 1;  // Increment total count
    
    delayMicroseconds(2);  // Optional small delay for stability
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
    return analogRead(pin) ? 100 : 0;  // HIGH == 100%,  LOW = 0%

  return (100 * highTime) / (highTime + lowTime);  // highTime as percentage of total cycle time
}

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  pinMode(PETAL_PIN, INPUT);
  pinMode(LED_PIN, INPUT);  // Set the PWM pin as input
  CircuitPlayground.begin();
  CircuitPlayground.clearPixels();
  delay(500);
  CircuitPlayground.setPixelColor(0, 255,   0,   0);
  CircuitPlayground.setPixelColor(1, 128, 128,   0);
  CircuitPlayground.setPixelColor(2,   0, 255,   0);
  CircuitPlayground.setPixelColor(3,   0, 128, 128);
  CircuitPlayground.setPixelColor(4,   0,   0, 255);
  
  CircuitPlayground.setPixelColor(5, 0xFF0000);
  CircuitPlayground.setPixelColor(6, 0x808000);
  CircuitPlayground.setPixelColor(7, 0x00FF00);
  CircuitPlayground.setPixelColor(8, 0x008080);
  CircuitPlayground.setPixelColor(9, 0x0000FF);

}

void setpixel(){ 
  CircuitPlayground.setPixelColor(0, 255,   0,   0);
  CircuitPlayground.setPixelColor(1, 128, 128,   0);
  CircuitPlayground.setPixelColor(2,   0, 255,   0);
  CircuitPlayground.setPixelColor(3,   0, 128, 128);
  CircuitPlayground.setPixelColor(4,   0,   0, 255);
  
  CircuitPlayground.setPixelColor(5, 0xFF0000);
  CircuitPlayground.setPixelColor(6, 0x808000);
  CircuitPlayground.setPixelColor(7, 0x00FF00);
  CircuitPlayground.setPixelColor(8, 0x008080);
  CircuitPlayground.setPixelColor(9, 0x0000FF);
}
void loop() {
  
  int temp_du = samplePin(LED_PIN); 
  Serial.println(temp_du); 
  // Map the duty cycle to a 0-10 range
  float led_brightness = map(temp_du, 62, 86, 0, 255);
  if (led_brightness < 0){ 
    led_brightness = 5; 
  }else if (led_brightness > 255) {
    led_brightness = 255;  
  }
  CircuitPlayground.setBrightness(led_brightness);
  setpixel(); 
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


