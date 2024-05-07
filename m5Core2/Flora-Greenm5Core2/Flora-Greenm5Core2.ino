/**
 * @file Flora-Greenm5Core2.ino
 * @brief MQTT communication and GPIO control for M5Stack device.
 *
 * This Arduino sketch establishes WiFi and MQTT connections, receives messages,
 * and controls GPIO pins based on the received MQTT messages.
 *
 * @author Shreya S.
 * @date 7/05/2024
 */

#include "M5Core2.h"
#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient client(espClient);
//Configure the name and password of the connected wifi and your MQTT Server
// host.
const char* ssid        = "infrastructure";
const char* password    = "PcDvSddLfhA2";
const char* mqtt_server = "csse4011-iot.zones.eait.uq.edu.au";
const char* subtopic = "s4580286";
// const char* pubtopic = "s4580286";

// Wifi Globals 
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int node = 1883; 
void setupWifi();
void callback(char* topic, byte* payload, unsigned int length);
void reConnect();

// Output Pins 
const int petal_flagPin = 19;
const int led_strengthPin = 25; // Analog Pin
int petal;
int b; 
int value; 

/**
 * @brief Extracts values from MQTT payload.
 *
 * This function parses the MQTT payload to extract values for 'petal' and 'b'.
 * It searches for specific characters in the payload and converts them to integers
 * to update global variables 'petal' and 'b'.
 *
 * @param payload Pointer to the MQTT payload data.
 * @param length Length of the payload data.
 */
void extractValues(unsigned char* payload, int length) {

    // Loop through the payload to find the values
    for (int i = 0; i < length; i++) {
        // Check if the current character is 'petal'
        if (payload[i] == 'p' && payload[i + 1] == 'e' && payload[i + 2] == 't' && payload[i + 3] == 'a' && payload[i + 4] == 'l') {
            // Skip to the value part
            i += 8;  // Skip ':1,' to get to the value 1
            // Convert the character representing the value to an integer
            petal = payload[i] - '0';
        }
        // Check if the current character is 'b'
        else if (payload[i] == 'b') {
            // Skip to the value part
            i += 3;  // Skip ':5}' to get to the value 5
            // Convert the character representing the value to an integer
            b = payload[i] - '0';
            break;  // No need to continue parsing once both values are found
        }
    }

    // Print the extracted values
    Serial.print("Petal: ");
    Serial.println(petal);
    Serial.print("B: ");
    Serial.println(b);
}

void setup() {
    M5.begin();
    setupWifi();
    client.setServer(mqtt_server,
                    1883);  // Sets the server details. 
    client.setCallback(
        callback);  // Sets the message callback function.
        
    // Output
    pinMode(petal_flagPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(led_strengthPin, OUTPUT); // Sets the echoPin as an Input
    Serial.begin(9600); // Starts the serial communication
}

/**
 * @brief Initializes the program setup.
 *
 * This function initializes the M5Core2, sets up WiFi connection, MQTT server details,
 * message callback function, and configures GPIO pins for output and serial communication.
 */
void loop() {
  //MQTT Loop 
  if (!client.connected()) {
      reConnect();
  }
  client.loop();  // This function is called periodically to allow clients to
                  // process incoming messages and maintain connections to the
                  // server.

  if (value % 12 == 0) {
      M5.Lcd.clear();
      M5.Lcd.setCursor(0, 0);
  }

}

/**
 * @brief Sets up WiFi connection.
 *
 * This function initializes the WiFi connection with the provided SSID and password,
 * sets the WiFi mode to station mode, and waits for a successful connection.
 */
void setupWifi() {
    delay(10);
    M5.Lcd.printf("Connecting to %s", ssid);
    WiFi.mode(
        WIFI_STA);  // Set the mode to WiFi station mode.  
    WiFi.begin(ssid, password);  // Start Wifi connection.  

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");
    }
    M5.Lcd.printf("\nSuccess\n");
}

/**
 * @brief Callback function for MQTT messages.
 *
 * This function handles incoming MQTT messages, prints the topic and payload to the LCD,
 * and performs actions based on the received payload, such as updating GPIO pins.
 *
 * @param topic Pointer to the topic string of the received message.
 * @param payload Pointer to the payload data of the received message.
 * @param length Length of the payload data.
 */
void callback(char* topic, byte* payload, unsigned int length) {
    M5.Lcd.print("Message arrived [");
    M5.Lcd.print(topic);
    M5.Lcd.print("] ");
    for (int i = 0; i < length; i++) {
        M5.Lcd.print((char)payload[i]);
    }
    if (length > 10){ 
      value++;
          //Activate GPIOS only if its valid
      extractValues(payload, length); 
      
      // Check if petal is 1 and set petal_flagPin accordingly
      if (petal == 1) {
          digitalWrite(petal_flagPin, HIGH);
      } else {
          digitalWrite(petal_flagPin, LOW);
      }
      
      // Map the range of B (1-10) to analog values (0-1023 for example)
      int analogValue = map(b, 0, 10, 0, 255);

      // Write the analog value to led_strengthPin
      analogWrite(led_strengthPin, analogValue);
      M5.Lcd.println();
    }   
    
}

/**
 * @brief Attempts to reconnect to the MQTT broker.
 *
 * This function continuously attempts to connect to the MQTT broker until a successful
 * connection is established. It generates a random client ID, connects to the broker,
 * subscribes to the specified topic, and handles connection failures.
 */
void reConnect() {
    while (!client.connected()) {
        M5.Lcd.print("Attempting MQTT connection...");
        // Create a random client ID. 
        String clientId = "M5Stack-s86";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect.
        if (client.connect(clientId.c_str())) {
            M5.Lcd.printf("\nSuccess\n");
            // Once connected, publish an announcement to the topic.
            //client.publish(pubtopic, "reconnecting");
            // ... and resubscribe. 
            client.subscribe(subtopic);
        } else {
            M5.Lcd.print("failed, rc=");
            M5.Lcd.print(client.state());
            M5.Lcd.println("try again in 1 seconds");
            delay(1000);
        }
    }
}
