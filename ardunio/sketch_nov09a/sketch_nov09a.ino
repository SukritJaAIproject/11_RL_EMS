#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Daikin.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESP32Ping.h>

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include "Adafruit_CCS811.h"

const char *ssid =     "mummai_owner_2.4G";         
const char *password = "mummai0972470067";              
char auth[] = "iFjAMTrHNiaFa9o5C6IYn58WCx-jjq4t";                    
#define BLYNK_PRINT Serial

IPAddress staticIP(192, 168, 1, 242);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 1);

const uint16_t kIrLed = 23;  
int temp_ac = 25; 

Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_CCS811 ccs;

typedef enum {
  fMIN=0,
  fMAX,
  fAUTO
}fan_ac_en;

fan_ac_en  fan_ac  = fAUTO;   // 0=min, 1=max, 2=auto

typedef enum{
  REQ_NONE =0,
  REQ_ON_AC,
  REQ_OFF_AC,
  REQ_TEMP_AC,
  REQ_FAN_AC
}req_en;

req_en req;
int retry = 0;
unsigned long last_retry=0;
unsigned long last_report=0;

//ปุ่ม ON
BLYNK_WRITE(V1)
{
  if(param.asInt()) {
    Serial.println("Req on recv");
    req = REQ_ON_AC;
  }
}

//ปุ่ม OFF
BLYNK_WRITE(V2)
{
  if(param.asInt()) {
    Serial.println("Req off recv");
    req = REQ_OFF_AC;
  }
}

//slider ปรับอุณหภูมิ
BLYNK_WRITE(V0)
{
  temp_ac = param.asInt();
  Serial.println("Req temp = " + String(temp_ac));
  req = REQ_TEMP_AC;
}

// slider ปรับความแรงพัดลมตามระดับ
BLYNK_WRITE(V3)
{
  req = REQ_FAN_AC;
  fan_ac = (fan_ac_en)param.asInt();
}

//ปุ่มความแรงพัดลมอัตโนมัต
BLYNK_WRITE(V4)
{
  Serial.println("Set fan auto");
  req = REQ_FAN_AC;
  fan_ac = fAUTO;
}

BlynkTimer timer;  //เรียกใช้การตั้งเวลาของ Blynk

IRDaikinESP ac(kIrLed);  // Set the GPIO used for sending messages.

//ฟังก์ชันโชว์ค่าที่ส่งออกไปเป็น HEX
void printState() {
  // Display the settings.
  Serial.println("Panasonic A/C remote is in the following state:");
  Serial.printf("  %s\n", ac.toString().c_str());
}
 
void setup() {
  
  Serial.begin(115200);
  delay(200);
  
//######################### Sensors ##################################
  Serial.println("SHT31 test"); 
  if (! sht31.begin(0x44)) { // Set to 0x45 for alternate i2c addr
  Serial.println("Couldn't find SHT31");
  while (1) delay(1);
  }
  
  Serial.println("CCS811 test");
  if(!ccs.begin()){
  Serial.println("Failed to start sensor! Please check your wiring.");
  while(1);
  }
  
  //calibrate temperature sensor
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);

//######################### WIFI ##################################
  if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
    Serial.println("Configuration failed.");
  }
  
  WiFi.mode(WIFI_STA);
  
  WiFi.disconnect();
  delay(100);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
    
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  IPAddress ip = WiFi.localIP();
  Serial.println(F("WiFi connected"));
  Serial.println("");
  Serial.println(ip);
  
  Serial.println(F("Trying to connect the blynk server.."));
  Blynk.config(auth);
  
  timer.setInterval(30000L, reconnecting);  //Function reconnect

  //ตั้งเวลาส่งข้อมูลให้ Blynk Server ทุกๆ 30 วินาที
  reconnecting();
    
  Serial.println(F("Blynk connected"));

  ac.begin();

  // Set up what we want to send. See ir_Panasonic.cpp for all the options.
  Serial.println("Default state of the remote.");
  printState();
  Serial.println("Setting desired state for A/C.");
  ac.on();
  ac.setFan(1);
  ac.setMode(kDaikinCool);
  ac.setTemp(temp_ac);
  ac.setSwingVertical(false);
  ac.setSwingHorizontal(false);  
}
 
 
void loop() {
  
      Serial.println("############SHT312###########################");
      Serial.print("Temp *C = "); Serial.println(sht31.readTemperature());
      Serial.print("Hum. % = "); Serial.println(sht31.readHumidity());
      delay(500);
    
      Serial.println("############CJMCU-8111###########################");
      if(ccs.available()){
      float temp = ccs.calculateTemperature();
      
      if(!ccs.readData()){
      Serial.print("CO2: = "); Serial.println(ccs.geteCO2());
      Serial.print("ppm, TVOC: "); Serial.println(ccs.getTVOC());
      Serial.print("ppb Temp:"); Serial.println(temp);
      delay(500);
      }
     
      else{
      Serial.println("ERROR!");
      while(1);}
      }

      switch(req) {
        case REQ_NONE: break;
        case REQ_ON_AC:   ac.on();   break;
        case REQ_OFF_AC:  ac.off();  break;
        case REQ_TEMP_AC: ac.setTemp(temp_ac); ac.on(); break;
        case REQ_FAN_AC: {
          switch(fan_ac) {
            case fMIN :  ac.setFan(kDaikinFanMin); break;
            case fMAX :  ac.setFan(kDaikinFanMax); break;
            case fAUTO : ac.setFan(kDaikinFanAuto);break;
          }
          break;
        }        
      }


      if(millis() - last_retry > 500 && req!=REQ_NONE ) {
        last_retry = millis();
        Serial.println("Sending IR command to A/C ...");
        ac.send();
        printState();
        retry ++;
        if(retry >= 2) {
          retry = 0;
          req = REQ_NONE;
        }
      }


      if(millis() - last_report > 2000) {
        last_report = millis();

        static bool heartbeat = false;
        Blynk.virtualWrite(0, temp_ac);  
        Blynk.virtualWrite(1, ac.getPower());  
        Blynk.virtualWrite(2, !ac.getPower());  
        if(fan_ac != fAUTO)  Blynk.virtualWrite(3, (int)fan_ac);
        Blynk.virtualWrite(4, fan_ac == fAUTO);
        Blynk.virtualWrite(5, temp_ac);
      }


      timer.run();


      if (Blynk.connected())
      {
        Blynk.run();
      } else {
        delay(200);
        Serial.println("Offlne");
      }

// delay(1000);
}


void reconnecting()
{
  static int blynkIsDownCount = 0;
  if (!Blynk.connected())
  {
    blynkIsDownCount++;
    BLYNK_LOG("blynk server is down! %d  times", blynkIsDownCount);
    Blynk.connect(5000);
  }
}
