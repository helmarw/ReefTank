//doseing pump with stepeer motor and LCD-keypad shield an rotary encoder
#include <LiquidCrystal.h>
#include <Chrono.h>
#include <LightChrono.h>
#include <math.h>

/*******************************************************

This Program is for controlling of a syringe dosage pump using s NEMA stepper driver
e.g https://www.ebay.de/itm/60MM-Z-Achse-Lineareinheit-Slide-Motion-Kugelumlaufspindel-Graviermaschine-CNC/292705104364
and a luer lock syring 20ml
apart from some 3D prited or machied parts you will nee also
Arduino Uno
http://www.watterott.com/de/Arduino-FabScan-Shield
https://www.watterott.com/de/Pololu-A4988-Schrittmotortreiber
https://www.dfrobot.com/wiki/index.php/Arduino_LCD_KeyPad_Shield_(SKU:_DFR0009) or simpilar
and a sufficient Power supply e.g.
https://www.watterott.com/de/Netzteil-12V-2A
https://www.dfrobot.com/wiki/index.php/EC11_Rotary_Encoder_Module_SKU:_SEN0235

H.Waiczies 2018-09-30

********************************************************/

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//set this for correct dosage, needs to be calibrated for each syringe
int nActive     = 48;    // initial intervall in hours, every two days
int steps_int   = 57;  // initial amount of liquid in steps 50steps = 1/4 revolution = 1mm
float steps_ul    = 5.614; //ul per step 57*5.614 = 320ul (with 20ml luer lock syringe) about 15ml usable volume, about 48 dosages (96days)
// some more things to setup
int steps_done  = 0; //steps per intervall done, will be reset after each loop
int steps       = 0; // total steps count

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// stepper driver definitions
            //driver: 1 | 2 | 3  | 4 
#define EN_PIN   11  //2 | 5 | 11 | A0
#define STEP_PIN 12  //3 | 6 | 12 | A1
#define DIR_PIN  13  //4 | 7 | 13 | A2
#define MS_PIN   A5 //HIGH for 16 microstepping, LOW for no microstepping
// use driver 3 only since pin 4,5,6,7 are beeing used by LCD !!!!

// rotary encoder definitions
#define clkPin 1
#define dtPin 2
#define swPin 3 //the number of the button
int encoderVal = steps_int;

// time constants
Chrono myChrono; 
long day = 86400000; // 86400000 milliseconds in a day
long hour = 3600000; // 3600000 milliseconds in an hour
long minute = 60000; // 60000 milliseconds in a minute
long second =  1000; // 1000 milliseconds in a second
int lastRunTime = 0;

// funktions here

// read the buttons funktion
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
/* 
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 250)  return btnUP; 
 if (adc_key_in < 450)  return btnDOWN; 
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 850)  return btnSELECT;  

 // For V1.0 comment the other threshold and use the one below:
/*
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;   
*/

 // For V1.1 with motor shield in between, or maybe just a glitch/error in the resistors of the LCD shield
 
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 270)  return btnUP; 
 if (adc_key_in < 530)  return btnDOWN; 
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 850)  return btnSELECT;  

 return btnNONE;  // when all others fail, return this...
}


// rotary endoder funktion
int getEncoderTurn(void)
{
  static int oldA = HIGH;
  static int oldB = HIGH;
  int result = 0;
  int newA = digitalRead(clkPin);
  int newB = digitalRead(dtPin);
  if (newA != oldA || newB != oldB)
  {
    // something has changed
    if (oldA == HIGH && newA == LOW)
    {
      result = (oldB * 2 - 1);
    }
  }
  oldA = newA;
  oldB = newB;
  return result;
}

// setup

void setup()
{
  myChrono.restart();
  lcd.begin(16, 2);              // start the library
  lcd.setCursor(0,0);
  lcd.print("Dosingpump v3 ul"); // print a simple message 
  lcd.setCursor(0,1);
  lcd.print("HW 2018-09-30");
  delay(2000);
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");

  // stepper setup
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH); //deactivate driver (LOW active) 
  // leave it off until you use it, dont forget to switch it off when ready, otherwise the stepper might get really hot
  pinMode(DIR_PIN, OUTPUT);
  digitalWrite(DIR_PIN, LOW); //LOW or HIGH, CW or CCW
  pinMode(STEP_PIN, OUTPUT);
  digitalWrite(STEP_PIN, LOW);
  pinMode(MS_PIN, OUTPUT);
  digitalWrite(MS_PIN, LOW); // microstepping off

  // rotary encoder setup
  pinMode(clkPin, INPUT);
  pinMode(dtPin, INPUT);
  pinMode(swPin, INPUT);
  digitalWrite(swPin, HIGH);
}

// main program loop
 
void loop()
{
 digitalWrite(EN_PIN, HIGH); //deactivate driver (LOW active)
//----------- time elapsed-------------
 long timeNow = myChrono.elapsed();
 //int days = timeNow / day ;                                //number of days 
 //int hours = (timeNow % day) / hour;                       //the remainder from days division (in milliseconds) divided by hours, this gives the full hours
 //int minutes = ((timeNow % day) % hour) / minute ;         //and so on...
 //int seconds = (((timeNow % day) % hour) % minute) / second;
 //char buf[21];
 //sprintf(buf,"%02d:%02d:%02d:%02d",days,hours,minutes,seconds);
 //sprintf(buf,"%02d:%02d:%02d",hours,minutes,seconds);
 //
 int runTime = timeNow/hour;  //change hour to minute for short term testing
 lcd.setCursor(0,0);
 lcd.print("D:");
 lcd.print(round(steps_int*steps_ul)); // Dosage
 lcd.print("ul "); 
 lcd.setCursor(11,0);
 lcd.print("#D:");              
 lcd.print(steps/steps_int);    // number of Dosages
 lcd.setCursor(0,1);
 lcd.print("Int:");
 lcd.print(nActive);
 lcd.print("h");
 lcd.print(" ");

 // rotary encoder
 int change = getEncoderTurn();
 encoderVal = encoderVal - change;
 steps_int = encoderVal;
  if(digitalRead(swPin) == LOW)
  {
   lcd.begin(16, 2);
   steps_int = encoderVal;
   steps = 0;
  }


 // main part, checking button status
 
 lcd.setCursor(12,1);            
 lcd_key = read_LCD_buttons();  // read the buttons

 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
   case btnRIGHT:
     {
     lcd.print("INC ");
     digitalWrite(EN_PIN, LOW); //activate driver
     digitalWrite(DIR_PIN, LOW);
     digitalWrite(STEP_PIN, LOW);
     delay(2);
     digitalWrite(STEP_PIN, HIGH);
     delay(2);
     steps++;
     break;
     }
   case btnLEFT:
     {
     lcd.print("DEC ");
     digitalWrite(EN_PIN, LOW); //activate driver
     digitalWrite(DIR_PIN, HIGH);
     digitalWrite(STEP_PIN, LOW);
     delay(2);
     digitalWrite(STEP_PIN, HIGH);
     delay(2);
     steps--;
     break;
     }
   case btnUP:
     {
     lcd.print("UP  ");
     //digitalWrite(MS_PIN, HIGH);
     nActive = nActive + 1;
     delay(500);
     break;
     }
   case btnDOWN:
     {
     lcd.print("DOWN");
     //digitalWrite(MS_PIN, LOW);
     nActive = nActive - 1;
     if (nActive <= 1)
      {
        nActive = 1;
      }
     delay(500);
     break;
     }
   case btnSELECT:
     {
     lcd.print("ZERO");
     steps=0;
     lcd.begin(16, 2); 
     break;
     }
   case btnNONE:
     {
     lcd.print("IDLE");
     if (runTime >= (lastRunTime+nActive)) 
     {
      steps_done = 0;
      while(steps_int-1 >= steps_done)
       {
        lcd.setCursor(12,1);
        lcd.print(" ACT");
        digitalWrite(EN_PIN, LOW); //activate driver
        digitalWrite(DIR_PIN, LOW);
        digitalWrite(STEP_PIN, LOW);
        delay(10);
        digitalWrite(STEP_PIN, HIGH);
        delay(10);
        steps_done++;
        steps++;
        lastRunTime = runTime;
       }
     } 
     break;
     }
 }

}
