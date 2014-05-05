/*
 Modifications to display the data by Michael Pechner mikey@mikey.com

 This example code is in the public domain.
 http://arduino.cc/en/Tutorial/HttpClient
 created by Tom igoe
 May 2013

Also cost taken from Adafruit adafruit_ST7735 graphicstext sample sketch
Written by Limor Fried/Ladyada for Adafruit Industries.
MIT license, all text above must be included in any redistribution

JSON uses Javascript/EMCA Script like syntax 
http://www.json.org/

Arduino JSON Library
https://github.com/bblanchon/ArduinoJsonParser

Using Adafruit Shield and associated libraries for the display
http://www.adafruit.com/products/802

 */
#define sclk 13
#define mosi 11
#define cs   10
#define dc   8
#define rst  0  // you can also connect this to the Arduino reset

#include <Bridge.h>
#include <HttpClient.h>
#include <JsonParser.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>


#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

// Option 1: use any pins but a little slower
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, mosi, sclk, rst);

char wjsonBuff[200];
JsonHashTable valueTab;

void setup() {
  // Bridge takes about two seconds to start up
  // it can be helpful to use the on-board LED
  // as an indicator for when it has initialized
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);

  Serial.begin(9600);

  //while (!Serial); // wait for a serial connection
  
    // If your TFT's plastic wrap has a Black Tab, use the following:
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  // If your TFT's plastic wrap has a Red Tab, use the following:
  //tft.initR(INITR_REDTAB);   // initialize a ST7735R chip, red tab
  // If your TFT's plastic wrap has a Green Tab, use the following:
  //tft.initR(INITR_GREENTAB); // initialize a ST7735R chip, green tab
  delay(50);
  tft.fillScreen(ST7735_BLACK);
  delay(50);
  
  
}
//Draws the text
void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextSize(6);
  tft.setTextWrap(true);
  tft.print(text);
}

//resets scripot then draws the text.
// Nothing fancy, wrote the simplist print that displays the speed and direction on the TFT.
void DisplayData()
{
  //char * name = valueTab.getString("time");
    //Serial.print("name=");
    //Serial.println(name);
     tft.fillScreen(ST7735_BLACK);
     char buff[10];
     sprintf(buff, "%-3s   %3s", 
        valueTab.getString("WSp"), 
       valueTab.getString("WDir"));
  testdrawtext(buff, ST7735_WHITE);
  delay(1000);
  
}

//Parses the JSON string returned fron the weatehr station
void parseWeather()
{
  JsonParser<32> parser;
  JsonHashTable hashTable = parser.parseHashTable(wjsonBuff);
  valueTab = hashTable.getHashTable("value");
  
  
  
}

void readWeather()
{
   HttpClient client;

  // Make a HTTP request:
  // Yes I hardcoded the IP address of the weather station.
  // It is the maker faire must mkae the demo work.
  // Excecise for the reader, make this find the weather station, or put the IP address of the weatehr station on the SD card.
  // Or add code to use the joy stick to enter the ip address.
  client.get("http://192.168.241.2/data/get");

  // if there are incoming bytes available
  // from the server, read them and print them:
  int ii=0;
  while (client.available()) {
    wjsonBuff[ii] = client.read();
    Serial.print(wjsonBuff[ii]);
    ii++;
  }
  wjsonBuff[ii]='\0';
  
  Serial.println("");
  Serial.flush();
}
void loop() {
 
  readWeather();
  parseWeather();
  DisplayData();
  delay(3000);
}


