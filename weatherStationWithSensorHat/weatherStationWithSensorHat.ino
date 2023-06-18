#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include "Adafruit_SGP40.h"
#include "SparkFun_SCD4x_Arduino_Library.h"

int uva = A2;
int uvb = A3;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define sdConnector 1

#define SEALEVELPRESSURE (1013.25)

#define SCREEN_ADDRESS 0x3C
#define BMEADDRESS 0x76
#define BH1750_ADDRESS 0x23

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BME280 bme;
BH1750 lightMeter(BH1750_ADDRESS);
Adafruit_SGP40 sgp;
SCD4x scd;

File myFile;

void setup() {
  Serial.begin(115200);
  Wire.begin();
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

  myFile = SD.open("datalog.csv", FILE_WRITE);
  if(myFile){
    myFile.print("Time (s)");
    myFile.print(",");
    myFile.print("Temp (C)");
    myFile.print(",");
    myFile.print("Pressure (hPa)");
    myFile.print(",");
    myFile.print("Humidity (%)");
    myFile.print(",");
    myFile.print("Altitude (m)");
    myFile.print(",");
    myFile.print("Ambient Light (lux)");
    myFile.print(",");
    myFile.print("VOC Index");
    myFile.print(",");
    myFile.print("CO2 Concentration (ppm)");
    myFile.print(",");
    myFile.println("UVB Index");
    myFile.close();
  }

  if(!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)){
    Serial.println("BH1750 Initialization Failed");
  }
  if(!sgp.begin()){
    Serial.println("SGP40 Initialization Failed");
  }
  if(!scd.begin()){
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
  
  float lux = lightMeter.readLightLevel();
  float sraw = sgp.measureRaw(temperature, humidity);
  float voc_index = sgp.measureVocIndex(temperature, humidity);
  float co2 = scd.getCO2();

  float uva_I = analogRead(uva) * (3300 / 1024); //nA
  float uva_P = uva_I / 113; //mW/cm2
  float uva_index = (uva_I - 83) / 21; //UVI 

  float uvb_I = analogRead(uvb) * (3300 / 1024); //nA
  float uvb_P = uvb_I / 69; //mW/cm2
  float uvb_index = uvb_I / 1.4; //UVI

  char buffer[64];
  sprintf(buffer, "%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f", time, temperature, pressure, humidity, altitude, lux, voc_index, co2, uva_index, uvb_index);
  
  myFile = SD.open("datalog.csv", FILE_WRITE);
  if(myFile){
    myFile.println(buffer);
    myFile.close();
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.print("DS Weather Station");
  
  display.setCursor(0, 10);
  display.print("T: ");
  display.print(temperature, 1);
  display.print("C ");
  display.print("P:");
  display.print(pressure, 0);
  display.print("hPa ");
  
  display.setCursor(0, 20);
  display.print("Hum: ");
  display.print(humidity, 0);
  display.print("% ");
  display.print("Lum: ");
  display.print(lux, 1);
  display.print("lux ");

  display.setCursor(0, 30);
  display.print("VOC: ");
  display.print(voc_index, 0);
  display.print(" ");
  display.print("CO2: ");
  display.print(co2, 0);
  display.print("ppm");
  
  display.setCursor(0, 40);
  display.print("UVA: ");
  display.print(uva_index, 0);
  display.print(" UVB: ");
  display.print(uvb_index, 0);
  display.display();

  sprintf(buffer, "%0.2fC\t%0.2fhPa\t%0.2f\t%0.2fm\t%0.2flux\t%0.0f\t%0.0fppm\t%0.2f\t%0.2f", temperature, pressure, humidity, altitude, lux, voc_index, co2, uva_index, uvb_index);
  Serial.println(buffer);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
}