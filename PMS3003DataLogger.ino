// reference: https://github.com/brucetsao/eMiniParticle

#include <DHT.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>

// T&RH sensor digital pin
#define DHTPIN 4     // what pin we're connected to
#define DHTTYPE DHT22   // DHT11 or DHT22  (AM2302)
// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

// MicroSD card adaptor digital pin
 // SD card attached to SPI bus as follows:
 // DI - pin 51
 // DO - pin 50
 // CLK - pin 52
 // CS - pin SD_chipSelect
const int SD_chipSelect = 5;

// OLED display
// You can use any (4 or) 5 pins 
#define OLED_MOSI   48
#define OLED_CLK   49
#define OLED_DC    45
#define OLED_CS    46
#define OLED_RESET 47
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS); 

// Plantower PMS3003
#define pmsDataLen 24
unsigned char buf[pmsDataLen];
int idx = 0;
int pm1 = 0;
int pm2_5 = 0;
int pm10 = 0;
bool hasPm25Value = false;


// RTC
// GND connected to pin 53
// 5V connected to pin 22
// SDA connected to pin 20
// SCL connected to pin 21
RTC_DS1307 rtc;

void setup()
{  
  // Open serial communications and wait for port to open:
   Serial.begin(9600); 
  
  // Start DHT sensor
  dht.begin();
  
// Display test
  Serial.print("Display testing...");
  display.begin(SSD1306_SWITCHCAPVCC);
  // initiate done
  
  // display the splashscreen.
  display.display();
  delay(1000);
  // Clear the buffer.
  display.clearDisplay();
  Serial.println("Done");
  delay(1000);  
  
    // Setup microSD card adaptor
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(SD_chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    // return;
  }
  else {Serial.println("Card initialized.");} 
  
     // Setup RTC
  #ifdef AVR
    Wire.begin();
  #else
    Wire1.begin(); // Shield I2C pins connect to alt I2C bus on Arduino Due
  #endif
    rtc.begin();
    
    if (! rtc.isrunning()) {
      Serial.println("RTC is NOT running!");
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2016, 9, 26, 12, 8, 0));
    }
}


void loop()
{  
// Timing
    unsigned long time1;
    time1 = millis();
// RTC time
  DateTime now = rtc.now();
  
// display date, time
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print(now.hour(), DEC); display.print(':');
  display.print(now.minute(), DEC); display.print(':');
  display.print(now.second(), DEC); display.println(' ');
  
    // print time to the serial port:
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.print("\t");
    
//-------T&RH Data----------------------------------------------- 
  // Read temperature as Celsius
  float temp = dht.readTemperature();
  // Read relative humidity 
  float humi = dht.readHumidity();

  if(isnan(humi) || isnan(temp) )
  {
    Serial.println();
    Serial.print("Failed to read from DHT sensor!");
    Serial.println(); 
   
  }
  else
  {  
  }
    // Serial.print("T ");
    Serial.print(temp);
    Serial.print("\t");
    // Serial.print(" degree C, RH ");
    Serial.print(humi);
    Serial.println("\t");

  //---------- Plantower PMS3003 sensor------------------
  hasPm25Value = false;
  int count = 0;
  int ckecksum = 0 ;
  int exptsum = 0 ;
  memset(buf, 0, pmsDataLen);

  while ( Serial.available() && count < pmsDataLen )
  {
    buf[count] = Serial.read();
    
    if (buf[0] == 0x42 && count == 0 )
    {
      count = 1 ;
      continue ;

    }
    if (buf[1] == 0x4d && count == 1 )
    {
      count = 2 ;
      continue ;

    }
    if (count >= 2 )
    {
      count++ ;
      if (count >= (pmsDataLen) )
      {
        hasPm25Value = true ;
        break ;
      }
      continue ;
    }

  }

  if (hasPm25Value )
  {
    for (int i = 0 ; i < (pmsDataLen - 2) ; i++)
      ckecksum = ckecksum + buf[i] ;
    exptsum = ((unsigned int)buf[22] << 8 ) + ((unsigned int)buf[23]) ;
    if (ckecksum == exptsum)
    {
      hasPm25Value = true ;
    }
    else
    {
      hasPm25Value = false ;
      Serial.print("\n \n ERROR Check sum");
      Serial.print("\n Sensor Check sum is : ");
      Serial.print(exptsum);
      Serial.print(", \n And Data Check sum is :");
      Serial.print(ckecksum);
      Serial.println("");
    }
    pm1 = ( buf[10] << 8 ) | buf[11];
    pm2_5 = ( buf[12] << 8 ) | buf[13];
    pm10 = ( buf[14] << 8 ) | buf[15];

    Serial.print("pm1.0: ");
    Serial.print(pm1);
    Serial.print(" ug/m3\t");
    Serial.print("pm2.5: ");
    Serial.print(pm2_5);
    Serial.print(" ug/m3\t");
    Serial.print("pm10: ");
    Serial.print(pm10);
    Serial.print(" ug/m3");
    Serial.println("");
  }

    
  //---------------Make Date String------------------------------------
   char str1[12]; // year
   char str1t[12] = "0"; // year
   char str2[12]; // month
   char str2t[12] = "0"; // month
   char str3[12]; // day
   char str3t[12] = "0"; // day
   
   char strP[12] = "P";
   char strT[12] = ".txt";
   
   int rtc_year = now.year();
   int rtc_month = now.month();
   int rtc_day = now.day();
   rtc_year = rtc_year-2000;
   
   sprintf(str1,"%d",rtc_year);
   int len1 = strlen(str1);
   if (len1 == 1){
   strcat(str1t, str1);
   strcpy(str1, str1t);
   }
   else {}

   sprintf(str2,"%d",rtc_month);
   int len2 = strlen(str2);
   if (len2 == 1){
   strcat(str2t, str2);
   strcpy(str2, str2t);
   }
   else {}

   sprintf(str3,"%d",rtc_day);
   int len3 = strlen(str3);
   if (len3 == 1){
   strcat(str3t, str3);
   strcpy(str3, str3t);
   }
   else {}   
   
   // make PM data file name, format: Pyymmdd.txt
   strcat(str1, str2);
   strcat(str1, str3);
   strcat(strP, str1);
   strcat(strP, strT);
    
  //-----------Logging Data to MicroSD Card--------------------------------  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another. 
  // save PM data

        File dataFile = SD.open(strP, FILE_WRITE);
        // if the file is available, write to it:
        if (dataFile) {
          dataFile.print(now.year(), DEC); dataFile.print("\t");
          dataFile.print(now.month(), DEC); dataFile.print("\t");   
          dataFile.print(now.day(), DEC); dataFile.print("\t");
          dataFile.print(now.hour(), DEC); dataFile.print("\t");
          dataFile.print(now.minute(), DEC); dataFile.print("\t");
          dataFile.print(now.second(), DEC); dataFile.print("\t");
      
          dataFile.print(temp); dataFile.print("\t");
          dataFile.print(humi); dataFile.print("\t");
          
          dataFile.print(pm1); dataFile.print("\t");
          dataFile.print(pm2_5); dataFile.print("\t");
          dataFile.println(pm10);
          
          display.println("  PM: OK");                                       
          dataFile.close();
          }
          else {
              // Serial.println("error opening pmdata.txt");
              display.println("PM: Fail");
          }   
 
//-------------OLED Display--------------------------------------------
        display.print("T ");
        display.print(temp);
        display.print(" C, RH ");
        display.print(humi);
        display.println("%");
        display.println("");

        display.print("PM1: ");
        display.println(pm1);
        display.print("PM2.5: ");
        display.println(pm2_5);
        display.print("PM10: ");
        display.println(pm10);
        
        display.display();
        display.clearDisplay(); 
    
//---------Time Interval (1s)----------------------------------------------  
    unsigned long time2;
    time2 = millis();
    unsigned long time_int;
    time_int = 1000-(time2-time1);
    delay(time_int); //delay for reread 
}





