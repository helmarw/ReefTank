/*
  peristaltic pump control for Balling Solutions
  peristaltic pump examlpe:
  https://www.amazon.de/dp/B01LYBW0GH/ref=cm_sw_em_r_mt_dp_U_3juyCbT278HG1

  the pump_holder.stl was desinged for those kind of bottles:
  https://www.amazon.de/dp/B01M5FQFGY/ref=cm_sw_em_r_mt_dp_U_MEuyCbYEA5Y0E

  for electronics you can use my AutoRefillSystem Board very few modifications
  
  
  IMPORTANT:  -check comment onder function Dosage()
              - set the correct nActive time intervall in hours for your application
  
  v1 February 2019
  by Helmar Waiczies
*/

#include <Chrono.h>
#include <LightChrono.h>
#include <math.h>

int motor=2; //LED connected to digital pin 2
int button=A1; //manual control button for pump
int buttonStatus;
int nActive=24; // initial intervall in hours, e.g. 48 for every two days

// time constants
Chrono myChrono; 
long day = 86400000; // 86400000 milliseconds in a day
long hour = 3600000; // 3600000 milliseconds in an hour
long minute = 60000; // 60000 milliseconds in a minute
long second =  1000; // 1000 milliseconds in a second
int lastRunTime = 0;

void setup()
{
myChrono.restart();
pinMode(motor, OUTPUT); //sets the digital pin as output
pinMode(LED_BUILTIN, OUTPUT); 
pinMode(button, INPUT_PULLUP);
delay(5000); //wait a bit until everything is settled, then start one dosage
Dosage();
//end of one time dosage. you can turn device off now, manual control will be enabled after 1sec
//if you keep device on, every nActive hours one dosage will be delivered
delay(1000);
//Serial.begin(9600);
}

int Dosage(){
//the automatic dosage control needs to be adjusted manually for each Balling solution and pump
//first you need to calibrate ml/sec for each pump and then enter the correct amount of ml of Balling solution you want to dose
//you can do it in steps with some delays for slow injection of the Balling solution
float ml=2.5; // amount of Balling Solution you want to dose
float ml_per_sec=0.22; // calibrated amount of ml/sec for the pump you are using (each pump may be different!!!)
float steps=10;  // total dosage deliverd in n steps
float steps_delay = 5000; // delay between steps in usec
float pump_on=(ml/ml_per_sec)*1000; //pump on in usec
float i=i+pump_on/steps;
while (i<=pump_on) {
  digitalWrite(motor,HIGH); //turns pump on
  digitalWrite(LED_BUILTIN, HIGH); //turns the LED on
  i=i+pump_on/steps;
  delay(pump_on/steps);
  digitalWrite(motor,LOW); //turns pump off
  digitalWrite(LED_BUILTIN, LOW); //tuffns the LED off
  delay(steps_delay); //wait a bit until balling solution is desolved
  }
}

void loop()
{
buttonStatus=digitalRead(button);
long timeNow = myChrono.elapsed();
int runTime = timeNow/hour;  //change hour to minute for short term testing
//Serial.println(buttonStatus);
if(buttonStatus == LOW)
  {
  digitalWrite(motor,HIGH); //turns pump on
  digitalWrite(LED_BUILTIN, HIGH); //turns the LED on
  } 
  else
  {
  digitalWrite(motor,LOW); //turns pump off
  digitalWrite(LED_BUILTIN, LOW); //tuffns the LED off
  }
if (runTime >= (lastRunTime+nActive)) {
  lastRunTime = runTime;
  Dosage();
  }
}
