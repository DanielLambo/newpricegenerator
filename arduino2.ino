#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>  // For Bluetooth communication

// Pin definitions for the Uno
#define LED_PIN_LOW 2     // LED for low price
#define LED_PIN_MEDIUM 3  // LED for medium price
#define BT_TX_PIN 5       // HC-05 Bluetooth TX Pin
#define BT_RX_PIN 6       // HC-05 Bluetooth RX Pin

RTC_DS3231 rtc;

// Initialize SoftwareSerial for HC-05 communication
SoftwareSerial btSerial(BT_RX_PIN, BT_TX_PIN);  // RX, TX

// CSV Data: Electricity Prices based on the hour of the day
const char* csvData = "\
Time,Price\n\
0,2.80\n\
1,3.00\n\
2,2.60\n\
3,2.40\n\
4,2.20\n\
5,2.00\n\
6,2.50\n\
7,3.00\n\
8,4.00\n\
9,5.00\n\
10,6.00\n\
11,5.50\n\
12,2.50\n\
13,4.50\n\
14,2.00\n\
15,3.50\n\
16,3.00\n\
17,4.50\n\
18,5.00\n\
19,6.00\n\
20,5.00\n\
21,4.00\n\
22,5.50\n\
23,1.00";

void setup() {
  Serial.begin(9600);     // Default Serial Monitor
  btSerial.begin(9600);   // Bluetooth Serial Communication

  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Set the RTC clock time to 11:32:00

  // Set LED pins as OUTPUT
  pinMode(LED_PIN_LOW, OUTPUT);
  pinMode(LED_PIN_MEDIUM, OUTPUT);
}

void loop() {
  DateTime now = rtc.now();

  // Display the current time and check price
  displayCurrentTime(now);
  checkElectricityPrice(now);

  // Read data from Bluetooth
  if (btSerial.available()) {
    char incomingByte = btSerial.read();
    Serial.print("Received from BT: ");
    Serial.println(incomingByte);
    // Add handling code for received data here if needed
  }

  delay(1000);  // Update every 1 second
}

void displayCurrentTime(DateTime now) {
  Serial.print("Time: ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());
}

void checkElectricityPrice(DateTime now) {
  int hour = now.hour();
  float priceInDollars = getPriceFromCSV(hour);

  // Set LED and message based on price
  if (priceInDollars < 2.50) {
    digitalWrite(LED_PIN_LOW, HIGH);
    digitalWrite(LED_PIN_MEDIUM, HIGH);
    sendBluetoothData(0);  // Send "0" for low price
    Serial.println("Status: Low Price");
    Serial.println("Advice: Use appliances.");

  } else if (priceInDollars < 5.00) {
    digitalWrite(LED_PIN_LOW, LOW);
    digitalWrite(LED_PIN_MEDIUM, HIGH);
    sendBluetoothData(1);  // Send "1" for medium price
    Serial.println("Status: Medium Price");
    Serial.println("Advice: Use priority appliances.");

  } else {
    digitalWrite(LED_PIN_LOW, LOW);
    digitalWrite(LED_PIN_MEDIUM, LOW);
    sendBluetoothData(2);  // Send "2" for high price
    Serial.println("Status: High Price");
    Serial.println("Advice: Turn off appliances.");
  }
}

float getPriceFromCSV(int hour) {
  char searchStr[4];
  sprintf(searchStr, "%02d", hour);

  char* priceStr = strstr(csvData, searchStr);
  if (priceStr != NULL) {
    int priceInCents = atoi(strchr(priceStr, ',') + 1);
    return static_cast<float>(priceInCents);
  }
  return 0.0;  // Default to 0 if no price is found for that hour
}

// Function to send data over Bluetooth
void sendBluetoothData(int priceLevel) {
  if (priceLevel == 0) {
    btSerial.write('0');  // Low price
  } else if (priceLevel == 1) {
    btSerial.write('1');  // Medium price
  } else {
    btSerial.write('2');  // High price
  }
}
