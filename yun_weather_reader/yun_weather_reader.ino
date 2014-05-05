/*
Author Michael Pechner  mikey@mikey.com

This was writtin for the Argent Data ADS-WS1 weather station base unit.
https://www.argentdata.com/catalog/product_info.php?cPath=29&products_id=146

Support Wiki
http://wiki.argentdata.com/index.php?title=ADS-WS1

You will need a level converter to bring the RS-232 to TTL levels. You will probably need a 10k resistor from the RXD1 terminal to your supply voltage.
The resistor must be disconnected when using the config application.

The SD card setup:
create the directories:
/ardunio/www/all
/arduino/www/wind
From the sketch folder www copy files index.html and zepto.min.js to each of the 2 directories.
So for example /ardunio/www/all/index.hmtl  /ardunio/www/all/zepto.min.js

you will need a rs-232 to ttl level converter to connect to the arduino  since it require ttl serial.
First numbers are from WS1 to DB9 female plug
Pin 6 ground to pin 5 on DB9
Pin 8 serial 1 TX to pin 2
Pin 9 serial 1 RX to pin 3

We use soft serial on pins 8 & 9

Peets Bother Information:
http://www.peetbros.com/shop/custom.aspx?recid=29
The record type is Data Logger

My notes on how the peets brother data looks.
!!000000D801740263275503060020----0006052000360000
!!0000 00BE 0140 0000 27CB 02DD 038E ---- 0024 0544 0000 0000

!! header
0000 wind speed [0.1 kph]
00D8 wind direction [0-255]
0174 current outdoor temp [0.1 deg F]
0263 rain long term total [0.01 inches]
2755 current barometer [0.1 mbar]
0306 current indoor temp [0.1 deg F]
---- current outdoor humidity [0.1%]
---- current indoor humidity [0.1%]
0006 date [day of year, -1]
0520 time [minute of day, 0-1440]
0036 today's rain total [0.01 inches]
0000 1 min wind speed average [0.1 kph]
*/
#include <stdio.h>
#include <SoftwareSerial.h>
#define rxPin 8
#define txPin 9

#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
YunServer server;
String startString;
long hits = 0;

//                           rx tx
SoftwareSerial  weather_port1(rxPin, txPin);
int wind_speed = 0;
int wind_dir = 0;
int raw_wind = 0;
int temp = 0;
float rain = 0;
float bar = 0;
int id_temp = 0;
int hum = 0;
int id_hum = 0;
int day = 0;
int minutes = 0;
float day_rain = 0;
int avg_wind = 0;
char wLine[100];

byte chrconv(byte);
unsigned int getword(char *);
int get_weather();
char * wind_name(int);
char * getLine();

unsigned int getword(char * ptr)
{
  unsigned int ii;
  sscanf(ptr, "%4x", &ii);
  return ii;
  //return( chrconv(*(ptr+0)) << 12 | chrconv(*(ptr+1)) << 8 | chrconv(*(ptr+2)) << 4 | chrconv(*(ptr+3)));
}
/*
byte chrconv(byte chr)
{
 if  (chr  < 48 || chr > 70)
   return -1;

 return  (0x40 & chr) ? (9 + (0x0f & chr) ) : (chr - 48);
}
*/
void setup()
{
  Serial.begin(9600);
 
  delay(2000);

  Serial.println("Before weather");
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  weather_port1.begin(2400);
  weather_port1.listen();
  Serial.println("After weather");
  
  Bridge.begin();

  Serial.println("After bridge begin");

  // Listen for incoming connection only from localhost
  // (no one from the external network could connect)
  server.listenOnLocalhost();
  Serial.println("After  listen");
  server.begin();
  Serial.println("After server begin");
  // get the time that this sketch started:
  Process startTime;
  startTime.runShellCommand("date");
  Serial.println("After date");
  while (startTime.available()) {
    char c = startTime.read();
    startString += c;
  }
  Serial.print(startString);

}
//first read until get get the '!' that signifies the start of a data record.
char * getLine()
{
  Serial.println("getline:");
  char cc = 0;
  int ii = 0;
  while (cc != '!')
  {
    if (weather_port1.available())
    {
     
      cc = weather_port1.read();
      //Serial.print(cc);
    }
  }
  wLine[0] = '!';
  ii = 1;
  while (1)
  {
    if (weather_port1.available())
    {
      cc = weather_port1.read();
      //Serial.print(cc);
      //Valid character range.
      if (ii < 99 and cc != '\n' and cc != '\r' and cc  > 31)
      {
        wLine[ii] = cc;
        ii++;
      }
      else
      {
        //Make sure the line is null terminated.  Valid string in C
        wLine[ii] = '\0';
        //Serial.println(' ');
        return &(wLine[0]);
      }
    }


  }

}
//Process a record from the weather station.
int get_weather()
{

  //char * ptr = "!!030100D8017402632755030600F0----0006052000360201\0";
  //Return a valid weather record
  char * ptr = getLine();
  int num_chars = strlen(ptr);
  Serial.println(ptr);
  Serial.println( num_chars);
  //Makes sure the header is present
  if (!ptr or * ptr != '!' or * (ptr + 1) !=  '!')
  {
    Serial.println("no header");
    return 1;
  }
  //this record must be 50 characters.
  if ( num_chars  != 50 )
  {
    Serial.println("invalid line");
    return 1;
  }
  Serial.print("LINE:");
  Serial.println(ptr);
  //Serial.print("LINE");
  //Serial.println(ptr);
  
  //Serial.println("header");
  //0000 wind speed [0.1 kph]
  wind_speed = (getword(ptr + 2) / 10.0) / 1.609334;
  //00D8 wind direction [0-255]
  //Wind is a 0-255 value and get interpolated to 0-360
  raw_wind = getword(ptr + 6);
  int tWindDir = int((getword(ptr + 6) * 360.0) / 255.0);
  if ( tWindDir >= 0 and tWindDir <= 360)
    wind_dir = tWindDir;

  //0174 current outdoor temp [0.1 deg F]
  temp = (getword(ptr + 10) / 10.0);
  //0263 rain long term total [0.01 inches]
  rain = (getword(ptr + 14) / 100.0);
  //2755 current barometer [0.1 mbar]
  bar = (getword(ptr + 18) / 10.0) * 0.0295301;
  //0306 current indoor temp [0.1 deg F]
  id_temp = (getword(ptr + 22) / 10.0);
  //---- current outdoor humidity [0.1%]
  hum = (getword(ptr + 26) / 10.0);
  //---- current indoor humidity [0.1%]
  id_hum = (getword(ptr + 30) / 10.0) ;
  //0006 date [day of year, -1]
  day = (getword(ptr + 34) + 1);
  //0520 time [minute of day, 0-1440]
  minutes = (getword(ptr + 38));
  //0036 today's rain total [0.01 inches]
  day_rain = (getword(ptr + 42) / 100.0);
  //0000 1 min wind speed average [0.1 kph]
  avg_wind = (getword(ptr + 46) / 10.0);

  return 0;
}
//in value between 0 and 360
// out a string representing the wind direction.
char * wind_name(int windir)
{

  char * wname[17] = {"", "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW",  "NW", "NNW"};
  //Each of the 16 wind directions is 22.5 degrees.
  float rawDir =  float(windir) / 22.5;
  // A litle fudge
  int  offset = int(rawDir + 1.5);
  offset = offset > 16 ? offset - 16 : offset;
  return wname[offset];

}


void loop()
{
  String wData;
  YunClient client = server.accept();
  if (!client) {
    delay(1000);
  }
   String command = client.readString();
   command.trim();
  Serial.println("command:"+command);
  int ret = get_weather();
  if ( ret == 1 )
  {
    Serial.println("fail");
    return;
  }
      // get the time from the server:
      Process time;
      time.runShellCommand("date");
      String timeString = "";
      while (time.available()) {
        char c = time.read();
        timeString += c;
      }
      //Serial.println(timeString);
  //Write the string so it can be grabbed using rest interface.    
  Bridge.put("time", timeString);
  Bridge.put("WSp", String(wind_speed));
  Bridge.put("WDir", wind_name(wind_dir));
  Bridge.put("WDeg", String(wind_dir));
  Bridge.put("AvSp", String(avg_wind));
  Bridge.put("Temp", String(id_temp));
  char barCh[10];
  sprintf(barCh, "%2d.%2d", int(bar),  int(bar * 100) - int(bar) * 100);
  Bridge.put("fBar", String(barCh));
  Bridge.put("Hum", String(hum));
  Bridge.put("rawWind", String(raw_wind)) ;
 
  //Serial.println(wData);
 if (command == "wind")
 {
   wData = "<h1>Time " + timeString  + "</h1>";
   wData += "<H1>Wind Direction "+String(wind_name(wind_dir)) +"</H1>";
   wData += "<H1>Avg Wind Speed "+String(avg_wind) +"</H1>";
   client.print(wData);
 }
 else if ( command == "all" )
 {
   wData = "<P>";
   wData += "<br>TM:&nbsp;" + timeString;
  wData += "<br>WSp:&nbsp;" + String(wind_speed) ;
  wData += "&nbsp;&nbsp;WDir:&nbsp;" + String(wind_name(wind_dir));
  wData += "<br>WDeg:&nbsp;" + String(wind_dir) ;
  wData += "&nbsp;&nbsp;AvSp:&nbsp;" + String(avg_wind) ;
  wData += "<br>Temp:&nbsp;" + String(id_temp);
  wData += "&nbsp;&nbsp;fBar:&nbsp;" + String(barCh) ;
  wData += "<br>Hum:&nbsp;" + String(hum);
  wData += "&nbsp;rawWind:&nbsp;" + String(raw_wind) ;
  wData += "</P>";
  client.print(wData);
   
 }else
 {
   client.print("<h1>bad request</h1>");
 }
 

client.stop();
  /*
  String response = String("Weather is ");
  Serial.println(response);
  char buff[100];
  response += String("Julian:") + day + String(" Minutes:") + minutes;
  response += String(" Wind Speed:") + wind_speed + String("Avg:") + avg_wind + String(" Direction:") + wind_dir + String("\n") ;
  sprintf(buff,"%d.%02d",  int(day_rain),  int(day_rain*100)- int(day_rain)*100);

  response += String("Rain Today:") + String(buff);
   sprintf(buff,"%d.%02d",  int(rain),  int(rain*100)- int(rain)*100);
  response +=  String(" LT Total:") + String(buff) +  String("\n");
  sprintf(buff,"%2d.%2d", int(bar),  int(bar*100)- int(bar)*100);
  Serial.println(bar);  Serial.println(bar/100);  Serial.println(bar);
  response +=  String(" OD Temp:") + temp+ String(" Hum:") + hum + String(" Bar:") + String(buff) + String("\n") + String(" ID Temp:") + id_temp + String(" Hum:") + id_hum + String("\n");
   Serial.println(response);
  */
  delay(500);
}
