
// DHT22 : Humitity & Temperature
#include <Wire.h>

// Try this from ADAFRUIT instead, current libs seems causing problem with new IDE :
// https://github.com/adafruit/DHT-sensor-library/blob/master/examples/DHTtester/DHTtester.ino
#include <DHT.h>

#include <SoftwareSerial.h>

//BMP180 : Pressure & Temperature & Altitude
// https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_Sensor.h>
// https://github.com/adafruit/Adafruit_BMP085_Unified
#include <Adafruit_BMP085_U.h>

// https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_GFX.h>    // Core graphics library
// https://github.com/adafruit/Adafruit-ST7735-Library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

// Official SD.h seem's OK, but an unofficial version maybe needed
#include <SD.h>              // SD Card

// http://wiki.seeedstudio.com/wiki/File:RTC_Library.zip
#include <DS1307.h>         // Realtime clock

// Data wire is plugged into port 7 on the Arduino
// Connect a 4.7K resistor between VCC and the data pin (strong pullup)
#define DHTPIN 7
#define DHTTYPE DHT22
// Setup a DHT22 instance
DHT dht(DHTPIN, DHTTYPE);

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);
SoftwareSerial mySerial(12, 13); // RX, TX

#define SD_CS      11
#define TFT_CS     10
#define TFT_RST    9  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_DC     8
// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

DS1307 clock;//define a object of DS1307 class

struct mydata_t
{
  uint8_t second;
	uint8_t minute;
	uint8_t hour; 
	uint8_t dayOfWeek;// day of week, 1 = Monday
	uint8_t dayOfMonth;
	uint8_t month;
	uint16_t year;
    
  float pressure;
  float temp;
  float humidity;
} mydata;

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  mySerial.begin(9600);
  // Serial.begin(57600);
  
  dht.begin();
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  
  clock.begin();
  bmp.begin();
  // if (!bmp.begin()) Serial.println("Ooops, no BMP085 detected ...");
  // displaySensorDetails();
  
  // Serial.print("\nInitializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin 
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output 
  // or the SD library functions will not work. 
  pinMode(SS, OUTPUT);     // change this to 53 on a mega
  
  if (!SD.begin(11)) {
    Serial.println("Something is wrong!");
    return;
  }
  readFile();
  
  tft.setTextWrap(false);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
}

void loop(void)
{ 
  dataHandle();
  delay(2);
  btHandle();
  delay(2);
}

void btHandle(void)
{
    char *buf;
    String odate;
    byte c=0;
    
    if(mySerial.available()) c = mySerial.read ();
    
    switch (c)
    {
       case 'm': 
          buf=getDataLine();
          mySerial.println(buf);
       break;
       
       case 'd':  // get date from external source
          // delay(10);
          // Serial.println("recu : ");
          btGetDate();
       break;
    }   
    
}

/* Set Clock from incoming data */

void btGetDate()
{
    char r=0;
    
    r = myRead(); //Blank
    clock.dayOfWeek = readInt(1);       r = myRead(); //Blank
    clock.year = readInt(4);            r = myRead(); //-
    clock.month = readInt(2);           r = myRead(); //-
    clock.dayOfMonth = readInt(2);      r = myRead(); // Blank
    clock.hour = readInt(2);            r = myRead(); //:    
    clock.minute = readInt(2);          r = myRead(); //:    
    clock.second = readInt(2);
   
    clock.fillByYMD(clock.year,clock.month,clock.dayOfMonth); // Needed since setting year directly seem's to fail
    clock.fillByHMS(clock.hour,clock.minute,clock.second);
    clock.fillDayOfWeek(clock.dayOfWeek);
    
    clock.setTime();//write time to the RTC chip 
}

char myRead()
{
    char c=0;
      
    if(mySerial.available())    c=mySerial.read();
    //Serial.print(c);
    delay(5);
    return c;
}

uint16_t readInt(uint8_t x)
{
    uint8_t z;
    char buf[x+1];
    
    for(z=0;z<x;z++) buf[z]=myRead();
    buf[z]='\0';
    
    return atoi(buf);   
}


/* Prepare data to send or store */
char *getDataLine(void)
{
    static char buf[100];
    static char fdat1[20];
    static char fdat2[20];
    static char fdat3[20];
    
    dtostrf(mydata.temp,1,2, fdat1);
    dtostrf(mydata.humidity,1,2, fdat2);
    dtostrf(mydata.pressure,1,2, fdat3);
    
    sprintf(buf, "%04hi-%02hi-%02hi %02hi:%02hi:%02hi;%s;%s;%s", 
                   mydata.year+2000,mydata.month, mydata.dayOfMonth, 
                   mydata.hour,mydata.minute, mydata.second, 
                   fdat1, fdat2, fdat3);         
    return buf;  
}

void sdDataPrintBinary(void)
{
    char *buf;
    File myFile;
  
    myFile = SD.open("test_binary.txt", FILE_WRITE);
    
    Serial.println("Try to print file");
    
    if (myFile) 
    {
      buf=getDataLine();        
                   
      Serial.println(buf);
      myFile.println(buf);
      myFile.close();
    } 
  
  // delay(50);  
    
 /*   
      // re-open the file for reading:
  myFile = SD.open("test.txt", FILE_READ);
  if (myFile) 
  {
    Serial.println("test.txt:");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
    	Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  */
}

void sdDataPrint(void)
{
    char *buf;
    File myFile;
  
    myFile = SD.open("test.txt", FILE_WRITE);
    
    Serial.println("Try to print file");
    
    if (myFile) 
    {
      buf=getDataLine();        
                   
      Serial.println(buf);
      myFile.println(buf);
      myFile.close();
    } 
  
  // delay(50);  
    
 /*   
      // re-open the file for reading:
  myFile = SD.open("test.txt", FILE_READ);
  if (myFile) 
  {
    Serial.println("test.txt:");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
    	Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  */
}


uint32_t refreshSensors = millis();
uint32_t refreshGraph = millis();
uint32_t refreshClock = millis();

int firstInit = 1;

void dataHandle(void)
{
   if ( refreshSensors > millis() ) refreshSensors = millis();
   if ( refreshGraph > millis() ) refreshGraph = millis();
   if ( refreshClock > millis() ) refreshClock = millis();
    
    if ( millis() - refreshClock >= 1000 || firstInit == 1) 
    {
        refreshClock = millis();
      
        clock.getTime();
        mydata.second=clock.second;
	      mydata.minute=clock.minute;
	      mydata.hour=clock.hour;
	      mydata.dayOfWeek=clock.dayOfWeek;
	      mydata.dayOfMonth=clock.dayOfMonth;
	      mydata.month=clock.month;
	      mydata.year=clock.year;

        dataPrintTime();
    }

    // Each minutes
    if ( millis() - refreshSensors >= 60000 || firstInit == 1) 
    { 
        refreshSensors = millis();
        
        getTempHumidity();
        getBmp180();
        
        printTempHumidity();
        printBmp180();
        
        sdDataPrint();
    }

    // Each 6 minutes
    if ( millis() - refreshGraph >= 60000 * 6 || firstInit == 1) 
    { 
        refreshGraph = millis();
      
        storeTemp();
        cleanGraph();
        drawAxisGraph();
        drawTempGraph();
        drawHumGraph();
        drawPresGraph();
        // sdDataPrint();
        
        firstInit=0;
    }
}

void dataPrintTime(void)
{
        static char buf[15];
        static const char day[][4]={"Lun", "Mar", "Mer", "Jeu", "Ven", "Sam", "Dim"};
        
        sprintf(buf, "%02hi:%02hi",
                   mydata.hour,mydata.minute);
        myTftPrint(0,5, 3, buf);
 
        sprintf(buf, ":%02hi",
                   clock.second);

        myTftPrint(94,5, 2, buf);

        sprintf(buf, "%s %02hi/%02hi/%04hi",                   
                   day[clock.dayOfWeek - 1],clock.dayOfMonth,clock.month,clock.year+2000);      
        myTftPrint(44,30, 1, buf);
        tft.drawFastHLine( 0, 42, 128, ST7735_WHITE );
        // tft.println("----------------------");
}

void myTftFloatPrint(uint8_t xpos, uint8_t ypos, uint8_t mysize, float value, char *buf)
{
      tft.setCursor(xpos, ypos);
      tft.setTextSize(mysize);
      tft.print(value);
      tft.println(buf);
}

void myTftPrint(uint8_t xpos, uint8_t ypos, uint8_t mysize, char *buf)
{
      tft.setCursor(xpos, ypos);
      tft.setTextSize(mysize);
      tft.println(buf);
}

void printTempHumidity(void)
{
      myTftFloatPrint(0,48, 2, mydata.temp, (char *)" C");
      myTftFloatPrint(0,66, 2, mydata.humidity, (char *)" %");
}

void getTempHumidity(void)
{ 
    mydata.temp=dht.readTemperature();
    mydata.humidity=dht.readHumidity();    
    
    /*  
    if (isnan(mydata.temp) || isnan(mydata.humidity) ) 
    {
      Serial.println("Failed to read from DHT sensor!");
    }*/
}


void printBmp180(void)
{
    myTftFloatPrint(0,84, 2, mydata.pressure, (char *)" hPa");
}

void getBmp180(void) 
{
  /* Get a new sensor event */ 
  sensors_event_t event;
  bmp.getEvent(&event);
 
  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  { 
    mydata.pressure=event.pressure;
  }  
  
}


const int MAX = 120;
int tempArray[ MAX ];
int humArray[ MAX ];
int presArray[ MAX ];
//********************************************************************
void storeTemp(void)
{
  // int temp = dht.readTemperature();
  // int hum = dht.readHumidity(); 
  
  int temp = (int) 10 * mydata.temp;
  int hum = (int) mydata.humidity;
  int pres = (int) mydata.pressure;
  
  static int i = 0;
  if ( isnan( ( int ) temp ) )
    if ( i < MAX )
    {
      tempArray[ i ] = 0;
      humArray[ i ] = 0;
      presArray[ i ] = 0;
      i++;
    }
    else
    {
      for ( int j = 0; j < MAX - 1; j++ )
      {
        tempArray[ j ] = tempArray[ j + 1 ];
        tempArray[ MAX - 1 ] = 0;
        humArray[ j ] = humArray[ j + 1 ];
        humArray[ MAX - 1 ] = 0; 
        presArray[ j ] = presArray[ j + 1 ];
        presArray[ MAX - 1 ] = 0; 
      }
    }
  else
  {
    if ( i < MAX )
    {
      tempArray[ i ] = temp;
      humArray[ i ] = hum;
      presArray[ i ] = pres;
      i++;
    }
    else
    {
      for ( int j = 0; j < MAX - 1; j++ )
      {
        tempArray[ j ] = tempArray[ j + 1 ];
        tempArray[ MAX - 1 ] = temp;
        humArray[ j ] = humArray[ j + 1 ];
        humArray[ MAX - 1 ] = hum; 
        presArray[ j ] = presArray[ j + 1 ];
        presArray[ MAX - 1 ] = pres; 
      }
    }
  }
}
//********************************************************************

#define XG 1
void drawAxisGraph(void)
{
  tft.drawFastVLine( XG, 100, 65, ST7735_WHITE );
  tft.drawFastHLine( XG, 159, 120, ST7735_WHITE );
}

void cleanGraph(void)
{
  tft.fillRect (XG+1, 100, 128, 60, ST7735_BLACK);
}

// TempÃ©rature *10 - 20.8Â° --> 208 
// Hauteur Graph 60 Pixel
// Temp Minimale  15Â° - Maxi 35Â° - Delta 20Â° --> 1 Â° = 3 Pixels
// position : 164 - ((Temp -  (hum Max * 10)) / 3)

#define TEMP_COLOR ST7735_RED
void drawTempGraph(void)
{
  for (int i = 0; i < MAX; i++ )
    tft.drawFastHLine( 120+XG+1 - MAX * 1 + i * 1, 164 - ((tempArray[ i ] - 150) / 3), 1, TEMP_COLOR ); 
}
//********************************************************************

// Hauteur Graph 60 Pixel
// HumiditÃ© minimale  30% - Maxi 80% - delta 50% --> 1 Pixel = 1% humiditÃ©
// position : 164 - hum - hum Max

#define HUM_COLOR ST7735_BLUE
void drawHumGraph()
{
  for (int i = 0; i < MAX; i++ )
     tft.drawFastHLine( 120+XG+1 - MAX * 1 + i * 1, 164 - (humArray[ i ] - 30), 1, HUM_COLOR ); 
}  

// Hauteur Graph 60 Pixel
// Pression minimale  950 hPa - Maxi 1040 hPa - delta 90 --> 1 Pixel = 1,33 humiditÃ©
// position : 164 - hum - hum Max

#define PRES_COLOR ST7735_GREEN
void drawPresGraph()
{

  for (int i = 0; i < MAX; i++ )
     tft.drawFastHLine( 120+XG+1 - MAX * 1 + i * 1, 164 - ((presArray[ i ] - 950)/ 1.33) , 1, PRES_COLOR ); 
}  


// *************************************************************
// *    Gestion de la reprise de donnÃ©es
// *************************************************************

void readFile()
{
  File myFile;
  
  myFile = SD.open("test.txt", FILE_READ);
  if (myFile) 
  {
    Serial.println("test.txt:");

    handleLines(myFile);
    
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void handleLines(File f)
{
   uint32_t pos=0;
//   float temp, humidity, pressure;
   char buf;
   String date;
   pos=getLinesPos(f, 120);
   
   while (f.available())
   {
     // buf=readlines(f);
      date = f.readStringUntil(';');
      mydata.temp = f.parseFloat();
      buf = f.read();
      mydata.humidity = f.parseFloat();
      buf = f.read();
      mydata.pressure = f.parseFloat();
      
      storeTemp();
   }
}


uint32_t getLinesPos(File f, uint8_t nb)
{
  uint32_t absPos=0;
  uint8_t CRcount=0;
  absPos=f.size();
  f.seek(absPos--);
  
  while (absPos > 0 && CRcount < nb)
  {
    if(f.read() == '\n') CRcount++;
    f.seek(absPos);
    absPos = absPos -2;
  }
  
  if (CRcount == nb) return f.position();
  else return -1;
}


