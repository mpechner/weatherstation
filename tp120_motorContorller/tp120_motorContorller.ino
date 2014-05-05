
//
//Author Michael Pechner  mikey@mikey.com
//
// http://www.instructables.com/id/Use-Arduino-with-TIP120-transistor-to-control-moto/
// Define which pin to be used to communicate with Base pin of TIP120 transistor
//
// The idea is to use this to control a fan to blow on the anemometer to help have some data for a weathter station.
// Each time period we use a random number generator to pick the time period and wind speed.
//
int TIP120pin = 11; //for this project, I pick Arduino's PMW pin 11
int pinval=0;  // The pwn value for the speed, 0-255
long delaySec; // how long this period will be.
void setup()
{
   pinMode(TIP120pin, OUTPUT); // Set pin for output to control TIP120 Base pin
   analogWrite(TIP120pin, 0); // By changing values from 0 to 255 you can control motor speed
   Serial.begin(9600);
   randomSeed(analogRead(0));
}
int lastpinval=-1;
void loop()
{
  pinval=random(0,4);
  switch (pinval)
  {
  case 0:
      if ( lastpinval == 0) pinval=95;
      Serial.println("No 0 pinvals in a row");
      break;
  case 1:
    pinval=90;
    break;
  case 2:
    pinval=160;
    break;
  case 3:
    pinval=200;
    break;
  case 4: pinval=255;
     break;
  }
  delaySec=random(10, 60);
  lastpinval=pinval;
  Serial.print("Pin Val  ");
  Serial.print(pinval);
  Serial.print(" MSecs ");
  Serial.println(delaySec*1000);
  analogWrite(TIP120pin,pinval );

  delay(delaySec * 1000);
  
}
