#include <EEPROM.h>
#include <GravityTDS.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TdsSensorPin A1
GravityTDS gravityTds;

float temperature = 25, tdsValue = 0;

// Distance sensor pins
const unsigned int TRIG_PIN=11;
const unsigned int ECHO_PIN=12;



// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <RTClib.h>

// Include the Servo library 
#include <Servo.h> 
// Declare the Servo pin 
int servoPin = 2; 
// Create a servo object 
Servo Servo1; 


//Temp sensor pin
#define ONE_WIRE_BUS 13
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


// Feeding time
uint8_t DAILY_EVENT_START_HH = 8; // event start time: hour
uint8_t DAILY_EVENT_START_MM = 45; // event start time: minute
uint8_t DAILY_EVENT_END_HH   = 8; // event end time: hour
uint8_t DAILY_EVENT_END_MM   = 46; // event end time: minute

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

void setup () {
  Serial.begin(9600);
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();  //initialization


  Servo1.attach(servoPin); 

  sensors.begin();

  // SETUP RTC MODULE
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // sets the RTC to the date & time on PC this sketch was compiled
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // sets the RTC with an explicit date & time, for example to set
  // January 21, 2021 at 3am you would call:
  // rtc.adjust(DateTime(2021, 1, 21, 3, 0, 0));

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);


}

void loop () {

  DateTime now = rtc.now();
  if (now.hour()   == DAILY_EVENT_START_HH && now.minute() == DAILY_EVENT_START_MM && now.minute() <  DAILY_EVENT_END_MM )
  {
   // Feeding sequence
   Servo1.write(0); 
   delay(500); 
   // Make servo go to 90 degrees 
   Servo1.write(90); 
   delay(500); 
   // Make servo go to 180 degrees 
   Servo1.write(180); 
   delay(500); 
  } 

    digitalWrite(TRIG_PIN,LOW);
    delayMicroseconds(5);
    
    //Start Measurement
    digitalWrite(TRIG_PIN,HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN,LOW);

    // Acquire and convert to cm
    int distance=pulseIn(ECHO_PIN,HIGH);
    distance=distance*0.0001657*39.37*2.54;


  printTime(now);

  sensors.requestTemperatures(); 
  
  Serial.print("Temperature: ");
  // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  Serial.print(sensors.getTempCByIndex(0)); 
  char degree[]="\u00b0";
  Serial.print(degree);
  Serial.println("C");
  
  temperature = sensors.getTempCByIndex(0);  //add your temperature sensor and read it

  gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
  gravityTds.update();  //sample and calculate
  tdsValue = gravityTds.getTdsValue();  // then get the value
  Serial.print("TDS: ");
  Serial.print(tdsValue,0);
  Serial.println("ppm");


  Serial.print("Tank Filling: "); 
  float x = (100*(29-distance))/29;
  Serial.print(x,2);
  Serial.println("%");
  Serial.println("");
  


  delay(10000);

}

void printTime(DateTime time) {
  Serial.print("TIME: ");
  Serial.print(time.day(), DEC);
  Serial.print('/');
  Serial.print(time.month(), DEC);
  Serial.print('/');
  Serial.print(time.year(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[time.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(time.hour(), DEC);
  Serial.print(':');
  Serial.print(time.minute(), DEC);
  Serial.print(':');
  Serial.println(time.second(), DEC);
}