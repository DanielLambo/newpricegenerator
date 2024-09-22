#include <TFT.h>  // Arduino LCD library
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>

// Pin definitions for the Uno
#define cs   10
#define dc   9
#define rst  8
#define LED_PIN_RED 2     // Red LED for high price
#define LED_PIN_YELLOW 3  // Yellow LED for medium price
#define LED_PIN_GREEN 4   // Green LED for low price

TFT TFTscreen = TFT(cs, dc, rst);  // Initialize the TFT object
RTC_DS3231 rtc;

// CSV Data: Electricity Prices based on the hour of the day
const char* csvData = "\
Time,Price\n\
1,3.00\n\
0,2.80\n\
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
23,3.00";

void setup() {
  Serial.begin(9600);

  // Initialize the TFT screen
  TFTscreen.begin();
  TFTscreen.background(0, 0, 0);  // Clear the screen with black

  // Initialize the RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Set LED pins as OUTPUT
  pinMode(LED_PIN_RED, OUTPUT);
  pinMode(LED_PIN_YELLOW, OUTPUT);
  pinMode(LED_PIN_GREEN, OUTPUT);
}

void loop() {
  DateTime now = rtc.now();

  // Display the current time, weather, and check price
  displayTimeWeatherAndCheckPrice(now);

  delay(1000);  // Update every 1 second
}

void displayTimeWeatherAndCheckPrice(DateTime now) {
  TFTscreen.stroke(255, 255, 255);  // Set text color to white

  // Display the current time
  char buffer[10];
  sprintf(buffer, "%02d:%02d", now.hour(), now.minute());
  TFTscreen.setTextSize(2);
  TFTscreen.text("Time: ", 0, 0);
  TFTscreen.text(buffer, 60, 0);

  // Display weather based on the time of day
  TFTscreen.setTextSize(1);
  String weather = getWeather(now.hour());
  TFTscreen.text("Weather: ", 0, 20);
  TFTscreen.text(weather.c_str(), 80, 20);  // Convert String to const char*

  // Check and display electricity price
  checkElectricityPrice(now);
}

String getWeather(int hour) {
  if (hour >= 0 && hour < 7) {
    return "Cold";
  } else if (hour >= 7 && hour < 15) {
    return "Hot";
  } else if (hour >= 15 && hour < 20) {
    return "Rainy";
  } else {
    return "Warm";
  }
}

void checkElectricityPrice(DateTime now) {
  int hour = now.hour();
  float priceInDollars = getPriceFromCSV(hour);

  // Buffer to store the formatted price
  char priceBuf[10];

  // Set LED and message based on price
  if (priceInDollars < 2.50) {
    digitalWrite(LED_PIN_GREEN, HIGH);
    digitalWrite(LED_PIN_YELLOW, LOW);
    digitalWrite(LED_PIN_RED, LOW);
    TFTscreen.stroke(0, 255, 0);  // Green for low price
    TFTscreen.text("Price: ", 0, 40);
    dtostrf(priceInDollars, 4, 2, priceBuf);  // Convert float to char array
    TFTscreen.text(priceBuf, 60, 40);
    TFTscreen.text("Status: Low Price", 0, 60);
    TFTscreen.text("Advice: Use appliances.", 0, 80);
  } else if (priceInDollars < 5.00) {
    digitalWrite(LED_PIN_YELLOW, HIGH);
    digitalWrite(LED_PIN_GREEN, LOW);
    digitalWrite(LED_PIN_RED, LOW);
    TFTscreen.stroke(255, 255, 0);  // Yellow for medium price
    TFTscreen.text("Price: ", 0, 40);
    dtostrf(priceInDollars, 4, 2, priceBuf);
    TFTscreen.text(priceBuf, 60, 40);
    TFTscreen.text("Status: Medium Price", 0, 60);
    TFTscreen.text("Advice: Use priority appliances.", 0, 80);
  } else {
    digitalWrite(LED_PIN_RED, HIGH);
    digitalWrite(LED_PIN_GREEN, LOW);
    digitalWrite(LED_PIN_YELLOW, LOW);
    TFTscreen.stroke(255, 0, 0);  // Red for high price
    TFTscreen.text("Price: ", 0, 40);
    dtostrf(priceInDollars, 4, 2, priceBuf);
    TFTscreen.text(priceBuf, 60, 40);
    TFTscreen.text("Status: High Price", 0, 60);
    TFTscreen.text("Advice: Turn off appliances.", 0, 80);
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
