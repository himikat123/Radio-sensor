/* 
 *  BIM32 Radio-sensor v4.2
 *  Copyright © himikat123@gmail.com, Nürnberg, Deutschland, 2022-2024
 *
 *  Arduino Nano - ATmega328P
 *  Arduino IDE v1.8.19
 */


#include <Adafruit_Sensor.h> // v1.1.3 https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_BME280.h> // v2.0.2 https://github.com/adafruit/Adafruit_BME280_Library
#include "SHT21.h"           // v1.0.0 https://github.com/markbeee/SHT21
#include <microDS18B20.h>    // v3.5.0 https://github.com/GyverLibs/microDS18B20
#include <MAX44009.h>        // v1.2.3 https://github.com/dantudose/MAX44009
#include <BH1750.h>          // v1.1.4 https://github.com/claws/BH1750
#include <PZEM004Tv30.h>     // v1.1.2 https://github.com/mandulaj/PZEM-004T-v30
#include <s8_uart.h>         // v1.0.1 https://github.com/jcomas/S8_UART

#include <SoftwareSerial.h>
#include "config.h"
#include "BIM32_Radio-sensor.h"
#include "tm1637.h"

Adafruit_BME280 bme280;
SHT21 SHT21;
MicroDS18B20<ONE_WIRE_1> ds18b20_1;
MicroDS18B20<ONE_WIRE_2> ds18b20_2;
MicroDS18B20<ONE_WIRE_3> ds18b20_3;
MicroDS18B20<ONE_WIRE_4> ds18b20_4;
MAX44009 max44009;
BH1750 bh1750(0x23);
SoftwareSerial pzemSWSerial(PZEM_RX, PZEM_TX);
PZEM004Tv30 pzem(pzemSWSerial);
SoftwareSerial S8_serial(SENSEAIR_RX, SENSEAIR_TX);
S8_UART *sensor_S8;
S8_sensor sensor; 
TM1637 tm1637;

void setup(){
  // Initialization pins to output
  uint8_t pins[] = {12, 13, A0, A1, A3, SET, DONE};
  for(uint8_t i = 0; i < sizeof(pins); i++) pinMode(pins[i], OUTPUT);
  
  // Determining the logical state of pins
  digitalWrite(DONE, LOW);
  digitalWrite(SET, HIGH);
  
  // ADC reference voltage setting
  analogReference(INTERNAL);

  // Start serial interface
  Serial.begin(9600);
  while(!Serial);
  
  // Start each software serial port
  pzemSWSerial.begin(9600);
  S8_serial.begin(S8_BAUDRATE);
  sensor_S8 = new S8_UART(S8_serial);
  
  // Sensors initialization
  sensorsInit();

  // Data send
  dataSend();
  delay(30);

  // Power off
  powerOff();
}

void loop(){
  tm1637.print(datas.temp_1); /* Displaying sensor data on display TM1637 */
  
  /* 
   * If the power does not turn off, send data and try to 
   * turn off the power again every 5 seconds
   */
  delay(5000);
  dataSend();
  powerOff();

  if(detected.ds18b20_1) ds18b20_1.requestTemp();
  if(detected.ds18b20_2) ds18b20_2.requestTemp();
  if(detected.ds18b20_3) ds18b20_3.requestTemp();
  if(detected.ds18b20_4) ds18b20_4.requestTemp();
}


/**
 * Power off
 */
void powerOff(void){
  digitalWrite(DONE, HIGH);
  delay(5);
  digitalWrite(DONE, LOW);
  delay(5);
  digitalWrite(DONE, HIGH);
  delay(10);
}

/**
 * Send data via UART
 */
void dataSend(void){
  getData();
  uint8_t attempts = 0;

  // wait if another wireless sensor is transmitting data now, 
  // but no more than 5 seconds
  while(Serial.available()){
    if(attempts++ > 25) break;
    Serial.read();
    delay(200);
  }
  
  // send data
  Serial.print("{");
  Serial.print("\"b\":"); Serial.print(datas.adc);
  Serial.print(",\"num\":\""); Serial.print(SENSOR_NUMBER); Serial.print("\"");
  if(detected.bme280){
    Serial.print(",\"t\":"); Serial.print(datas.temp);
    Serial.print(",\"h\":"); Serial.print(datas.hum);
    Serial.print(",\"p\":"); Serial.print(datas.pres);
    Serial.print(",\"s\":"); Serial.print("\"BME280\"");
  }
  else if(detected.sht21){
    Serial.print(",\"t\":"); Serial.print(datas.temp);
    Serial.print(",\"h\":"); Serial.print(datas.hum);
    Serial.print(",\"s\":"); Serial.print("\"SHT21\"");
  }
  if(detected.max44009){
    Serial.print(",\"l\":"); Serial.print(datas.light);
    Serial.print(",\"a\":"); Serial.print("\"MAX44009\"");
  }
  if(detected.bh1750){
    Serial.print(",\"l\":"); Serial.print(datas.light);
    Serial.print(",\"a\":"); Serial.print("\"BH1750\"");
  }
  #ifdef USE_DS18B20_1
    Serial.print(",\"ds\":["); Serial.print(datas.temp_1); 
    Serial.print(","); Serial.print(datas.temp_2);
    Serial.print(","); Serial.print(datas.temp_3);
    Serial.print(","); Serial.print(datas.temp_4);
    Serial.print("]");
  #endif
  #ifdef USE_PZEM_004T
    Serial.print(",\"pzem\":[");
    Serial.print(datas.voltage, 1); Serial.print(",");
    Serial.print(datas.current, 3); Serial.print(",");
    Serial.print(datas.power, 1); Serial.print(",");
    Serial.print(datas.energy, 2); Serial.print(",");
    Serial.print(datas.frequency, 1); Serial.print(",");
    Serial.print(datas.pf, 2);
    Serial.print("]");
  #endif
  if(detected.s8){
    Serial.print(",\"s8\":"); Serial.print(datas.co2);
  }
  Serial.println("}");
  Serial.flush();
}
