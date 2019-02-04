/*
  Level Sensor  v2 for new Board done in Fritzing
  Turns on a relais when low water level is detected and check if refill Tank is empty, to protect pump from running dry...

  using Non-contact Liquid Level Sensor XKC-Y25-T12V SKU: SEN0204
  https://www.dfrobot.com/wiki/index.php/Non-contact_Liquid_Level_Sensor_XKC-Y25-T12V_SKU:_SEN0204
  for upper level detektion and
  Liquid Level Sensor-FS-IR02 SKU: SEN0205
  https://www.dfrobot.com/wiki/index.php/Liquid_Level_Sensor-FS-IR02_SKU:_SEN0205
  for lower level detection
  
  modified 14 January 2019
  by Helmar Waiczies
*/

int Liquid_level=0;
int Liquid_level1=0;
int Liquid_level2=0;
int Liquid_levelTank=0;
int LED=LED_BUILTIN;
int Relais=2;
int SensLowTank=A1;
int SensLow=A2;
int SensHigh=A3;
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(Relais, OUTPUT);
  digitalWrite(Relais, LOW);
  Serial.begin(9600);
  pinMode(SensLowTank,INPUT); //opt. or capacitive Sensot for refill tank
  pinMode(SensLow,INPUT);  //opt. Sensor
  pinMode(SensHigh,INPUT);  //cap. or opt. sensor (overflow protection)
}

// the loop function runs over and over again forever
void loop() {
  Liquid_level1=digitalRead(SensLow);
  Liquid_level2=digitalRead(SensHigh);
  Liquid_levelTank=digitalRead(SensLowTank);
  Liquid_level=Liquid_level1+Liquid_level2;
  if (Liquid_level == 0) {
    if (Liquid_levelTank == 1) {
    digitalWrite(LED, HIGH);    // turn the LED on by making the voltage HIGH
    digitalWrite(Relais, HIGH);  //pumpe an
    Serial.println("Water_Level Low, refilling");
    delay(2500); // pump has to run at least 2.5sec to fill the pipe; needs adjustments depending on pipe length, diameter and pump
    }
  }
  if (Liquid_level == 1) {
    digitalWrite(LED, LOW);   // turn the LED OFF 
    digitalWrite(Relais, LOW);   //pumpe aus
    Serial.println("Water_Level ok");
  }
  if (Liquid_level == 2) {
    digitalWrite(LED, LOW);   // turn the LED OFF  
    digitalWrite(Relais, LOW);   //pumpe aus
    Serial.println("Water_Level to high, remove some water, possible overflow!!!");
  }
  if (Liquid_levelTank == 0) {
    digitalWrite(LED, LOW);   // turn the LED OFF  
    digitalWrite(Relais, LOW);   //pumpe aus
    Serial.println("Water_Tank_Level to low, refill Water Tank!");
  }

  delay(500);                     // wait for a second

}
