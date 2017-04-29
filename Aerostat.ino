#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <XMLWriter.h>
#include <Ticker.h>
#include <Servo.h>
#include <SPI.h>
#include <SD.h>
#include "DHT.h"

//#define LOGIN "TP-LINK_BAB676" // WIFI settings
//#define PASSWORD "14083815"
#define LOGIN "Terminal 42 Secure 2G" // WIFI settings
#define PASSWORD "Multipass"
#define TOKEN "545ed1cd246d40e9b124ce09cfc2b377"// этот токен есть у тебя на почте его НЕ ТРОГАТЬ

#define TIMER 10 // сейчас опрос датчик происходит каждые 10 СЕКУНДЫ. Но можно менять 

//##################################################
#define SERVOPIN 4 // servo motor for driving main motor
//##################################################

//##################################################
#define MOTORSPEED 5 // control speed of motor
#define MOTORDIRECTION 0 // control rotation direction of motor
//##################################################

//##################################################
#define DHTPIN 2 // pin for DHT11 sensor
#define DHTTYPE DHT11   // DHT11 sensor 
//##################################################

//##################################################
#define TEMPERATURE 1 // temperature index
#define HUMIDITY    2 // hudimity index
//##################################################

//##################################################
#define CHIPSELECT 15 // SPI SD card
//##################################################

String LogFile = "Log.xml";          // file, that log measures

Servo MotorServo;

Ticker TimerForSensors;

boolean tickerFired;

DHT dht(DHTPIN, DHTTYPE, 15);

File dataFile;

void flagSensorScan( void )
{
  tickerFired = true;
}

void MeasureXML(String file, uint8_t id_measure, uint8_t readings)
{      
    dataFile.close();
    dataFile = SD.open(file, FILE_WRITE);
    
    if (dataFile)
    {
      XMLWriter XML(&dataFile);
  
      XML.tagOpen("measure");
      XML.writeNode("id", id_measure);
      XML.writeNode("readings", readings);
      XML.tagClose();
      dataFile.flush();
    }
    dataFile.close();   
}

void Sensorloop()
{
  float hum = 0;
  float temp = 0;

  do
  {
    delay(100);
        
    hum = dht.readHumidity();
     // Read temperature as Celsius (the default)
    temp = dht.readTemperature(); 

    yield();

    Serial.println("read");
  }
  while(isnan(hum) || isnan(temp));

   MeasureXML(LogFile, TEMPERATURE, temp);
   MeasureXML(LogFile, HUMIDITY, hum);    

   Blynk.virtualWrite(V5, temp);//  отправка данных на приложение
   Blynk.virtualWrite(V6, hum);//  отправка данных на приложение

   Serial.println("Humidity:");
   Serial.println(hum);
   Serial.println("Temperature:");
   Serial.println(temp);
      
   Serial.println("Done");
}

BLYNK_WRITE(V0)
{
  int servodegree = param.asInt();
  if(servodegree >= 0 &&  servodegree <= 180)
  {
      MotorServo.write(servodegree);
  }
}

BLYNK_WRITE(V1)
{

  
  int motor = param.asInt();

  Serial.println("Motor speed:");
  Serial.println(motor);
  
  if(motor >= 0 &&  motor <= 1023)
  {
      analogWrite(MOTORSPEED, motor);
  }
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  Serial.println();

  dht.begin(); //init DHT sensor

  Blynk.begin(TOKEN, LOGIN, PASSWORD);

  if (!SD.begin(CHIPSELECT)) // init SD card
  {
    Serial.println("Card failed, or not present");
  }

  pinMode(MOTORSPEED, OUTPUT); // motor A speed
  pinMode(MOTORDIRECTION, OUTPUT); //  motor A rotation direction

  analogWrite(MOTORSPEED, 1023);
  digitalWrite(MOTORDIRECTION, HIGH);
  
  MotorServo.attach(SERVOPIN);

  TimerForSensors.attach(TIMER, flagSensorScan); // init interrupt for scan sensors
  tickerFired = true; //flag for scan sensors interrupt
}

void loop()
{
  Blynk.run();
  
  if(tickerFired) 
  {
    tickerFired = false;

    Sensorloop();
  }   

  yield();// wait some time for executing background functions
}
