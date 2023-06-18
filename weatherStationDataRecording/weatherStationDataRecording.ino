#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET -1
#define sdConnector 1
#define BMEADDRESS 0x76
#define SEALEVELPRESSURE (1013.25)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BME280 bme;

File myFile;

void setup() {
  Serial.begin(57600);
  pinMode(LED_BUILTIN, OUTPUT);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println("OLED Display Initialization Failed");
    while(1) {
      digitalWrite(LED_BUILTIN, LOW);
      delay(250);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(250);
    }
  }

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.display();
  delay(2000);
  display.clearDisplay();

  if(!bme.begin(BMEADDRESS, &Wire)){
    Serial.println("BME280 Initialization Failed");
    while(1);
  }

  if(!SD.begin(sdConnector)){
    Serial.println("microSD Initialization Failed");
    display.print("SD Failure");
    display.setCursor(0, 10);
    display.print("Check if SD card is inserted");
    display.display();
    while(1);
  }

  char buffer[64];
  sprintf(buffer, "Time(s),Temp (C),Pressure (hPa),Humidity (%),Altitude (m)");
  myFile = SD.open("datalog.csv", FILE_WRITE);
  if(myFile){
    myFile.println(buffer);
    myFile.close();
  }

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF   );
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  float time = millis() / 1000.00;

  bme.takeForcedMeasurement();
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure() / 100.00F;
  float humidity = bme.readHumidity();
  float altitude = bme.readAltitude(SEALEVELPRESSURE);
  
  char buffer[64];
  sprintf(buffer, "%0.2f, %0.2f, %0.2f, %0.2f", temperature, pressure, humidity, altitude);
  myFile = SD.open("datalog.csv", FILE_WRITE);
  if(myFile){
    myFile.println(buffer);
    myFile.close();
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.print("DS Weather Station");
  
  display.setCursor(0, 10);
  display.print("Temperature: ");
  display.print(temperature, 2);
  display.print(" C");
  
  display.setCursor(0, 20);
  display.print("Pressure: ");
  display.print(pressure, 2);
  display.print(" hPa");
  
  display.setCursor(0, 30);
  display.print("Humidity: ");
  display.print(humidity, 2);
  display.print(" %");

  display.setCursor(0, 40);
  display.print("Alt: ");
  display.print(altitude, 2);
  display.print(" m");
  display.display();

  sprintf(buffer, "%0.2f C\t%0.2f hPa\t%0.2f\t%0.2f m", temperature, pressure, humidity, altitude);
  Serial.println(buffer);
  
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
}