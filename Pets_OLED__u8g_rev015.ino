//RGB Output Pins
int RPin = 10;
int GPin = 11;
int BPin = 9;

//Initial RGB Values: 0 Off, 255 On
int red = 0;
int green = 0;
int blue = 0;

//Cycle Color Values
int SRR = 225;
int SRG = 10;
int SRB = 25;
int dayR = 240;
int dayG = 240;
int dayB = 240;
int SSR = 225;
int SSG = 25;
int SSB = 10;
int nightR = 1;
int nightG = 1;
int nightB = 3;

//Cycle Schedule
int night = 2; // 0 for sunrise, 1 for day, 2 for sunset, 3 for Night
int sunrise = 6;
int sunset = 20;
int hourtemp = 0;// temp value from RTC
int mintemp = 0;

//RTC
#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68
//int tod;

char * tod[8];

//OLED
#include "U8glib.h"
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI

//Wtemp
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float Celcius = 0;
float Fahrenheit = 0;
float wtemp;

//Temp & Humid
#include <dht.h>
dht DHT;
#define DHT11_PIN 7

//PWM Fan
#define pwm 3
float humid;
float temp;
float fanspd;

//Sensor Delays
int period = 5500;
unsigned long time_now = 0;

//Clock 
int period2 = 0;
unsigned long time_now2 = 0;

//Cycle Pacing
int period3 = 0;
unsigned long time_now3 = 0;

//Uptime
#include "uptime.h"

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val / 10 * 16) + (val % 10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val / 16 * 10) + (val % 16) );
}

void setup() {
  //Serial.begin(9600);
  Wire.begin();
  sensors.begin();
  //sensors.setWaitForConversion(true);
  analogWrite(pwm, 255); //pwm fan

  // SET TIME HERE:
  //DS3231  seconds, minutes, hours, day, date, month, year
  //setDS3231time(17,     4,    11,   6,   27,    3,    20);

  //RGB output pins
  pinMode(RPin, OUTPUT); // red
  pinMode(GPin, OUTPUT); // green
  pinMode(BPin, OUTPUT); // blue
}
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
                   dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

void readDS3231time(byte *second,
                    byte *minute,
                    byte *hour,
                    byte *dayOfWeek,
                    byte *dayOfMonth,
                    byte *month,
                    byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

void displayTime()
{
  if (millis() >= time_now2)
  {
    time_now2 += period2;
    byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
    readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
                   &year);
    u8g.setFont(u8g_font_blipfest_07r);
    u8g.setPrintPos(0, 5);
    if (hour < 10)
    {
      u8g.print(F("0"));
    }
    u8g.print (hour, DEC);
    hourtemp = hour;
    mintemp = minute;

    if ( hourtemp > sunrise) {
      if (hourtemp < sunset) {
        if (night == 0 && red == SRR && green == SRG && blue == SRB) {
          night = 1;
        }
        else if (night == 3 && red == nightR && green == nightG && blue == nightB) {
          night = 0;
        }
        else if (night == 2 && red == SSR && green == SSG && blue == SSB) {
          night = 3;
        }
      }
      else if (hourtemp == sunrise) {
        if (night == 3 && red == nightR && green == nightG && blue == nightB) {
          night = 0;
        }
        else if (night == 2 && red == SSR && green == SSG && blue == SSB) {
          night = 3;
        }
        else if (night == 1 && red == dayR && green == dayG && blue == dayB) {
          night = 2;
        }
      }
      else if (hourtemp == sunset) {
        if (night == 1 && red == dayR && green == dayG && blue == dayB) {
          night = 2;
        }
        else if (night == 0 && red == SRR && green == SRG && blue == SRB) {
          night = 1;
        }
        else if (night == 3 && red == nightR && green == nightG && blue == nightB) {
          night = 0;
        }
      }
      else if ( hourtemp < sunrise) {
        if (night == 0 && red == SRR && green == SRG && blue == SRB) {
          night = 1;
        }
        else if (night == 1 && red == dayR && green == dayG && blue == dayB) {
          night = 2;
        }
        else if (night == 2 && red == SSR && green == SSG && blue == SSB) {
          night = 3;
        }
      }
      else if ( hourtemp > sunset) {
        if (night == 0 && red == SRR && green == SRG && blue == SRB) {
          night = 1;
        }
        else if (night == 1 && red == dayR && green == dayG && blue == dayB) {
          night = 2;
        }
        else if (night == 2 && red == SSR && green == SSG && blue == SSB) {
          night = 3;
        }
      }
    }
    else if (hourtemp == sunrise) {
      if (night == 1 && red == dayR && green == dayG && blue == dayB) {
        night = 2;
      }
      else if (night == 2 && red == SSR && green == SSG && blue == SSB) {
        night = 3;
      }
      else if (night == 3 && red == nightR && green == nightG && blue == nightB) {
        night = 0;
      }
    }
    else if (hourtemp == sunset) {
      if (night == 3 && red == nightR && green == nightG && blue == nightB) {
        night = 0;
      }
      else if (night == 0 && red == SRR && green == SRG && blue == SRB) {
        night = 1;
      }
      else if (night == 1 && red == dayR && green == dayG && blue == dayB) {
        night = 2;
      }
    }
    else if ( hourtemp < sunrise) {
      if (night == 0 && red == SRR && green == SRG && blue == SRB) {
        night = 1;
      }
      else if (night == 1 && red == dayR && green == dayG && blue == dayB) {
        night = 2;
      }
      else if (night == 2 && red == SSR && green == SSG && blue == SSB) {
        night = 3;
      }
    }
    else if ( hourtemp > sunset) {
      if (night == 0 && red == SRR && green == SRG && blue == SRB) {
        night = 1;
      }
      else if (night == 1 && red == dayR && green == dayG && blue == dayB) {
        night = 2;
      }
      else if (night == 2 && red == SSR && green == SSG && blue == SSB) {
        night = 3;
      }
    }
    u8g.print(F(":"));
    if (minute < 10)
    {
      u8g.print(F("0"));
    }
    u8g.print(minute, DEC);
    u8g.print(F(":"));
    if (second < 10)
    {
      u8g.print(F("0"));
    }
    u8g.print(second, DEC);

    u8g.setPrintPos(101, 5);
    if (month < 10)
    {
      u8g.print(F("0"));
    }
    u8g.print(month, DEC);
    u8g.print(F("."));
    if (dayOfMonth < 10)
    {
      u8g.print(F("0"));
    }
    u8g.print(dayOfMonth, DEC);
    u8g.print(F("."));
    u8g.print(year, DEC);
    u8g.setPrintPos(93, 11);
    switch (dayOfWeek) {
      case 1:
        u8g.print(F("            SUNDAY"));
        break;
      case 2:
        u8g.print(F("            MONDAY"));
        break;
      case 3:
        u8g.print(F("        TUESDAY"));
        break;
      case 4:
        u8g.print(F("WEDNESDAY"));
        break;
      case 5:
        u8g.print(F("    THURSDAY"));
        break;
      case 6:
        u8g.print(F("            FRIDAY"));
        break;
      case 7:
        u8g.print(F("    SATURDAY"));
        break;
    }
  }
}
void loop()
{
  if (millis() >= time_now3)
  {
    time_now3 += period3;
    switch (night) {
      case 0:
        //Serial.println("Sunrise");
        tod[8] = "SUNRISE";
        if (red > SRR) {
          red--;
        }
        if (red < SRR) {
          red++;
        }
        if (green < SRG) {
          green++;
        }
        if (green > SRG) {
          green--;
        }
        if (blue < SRB) {
          blue++;
        }
        if (blue > SRB) {
          blue--;
        }
        break;

      case 1:
        //Serial.println("Day");
        tod[8] = "DAY";
        if (red > dayR) {
          red--;
        }
        if (red < dayR) {
          red++;
        }
        if (green > dayG) {
          green--;
        }
        if (green < dayG) {
          green++;
        }
        if (blue > dayB) {
          blue--;
        }
        if (blue < dayB) {
          blue++;
        }
        break;

      case 2:
        //Serial.println("Sunset ");
        tod[8] = "SUNSET";
        if (red < SSR) {
          red++;
        }
        if (red > SSR) {
          red--;
        }
        if (green < SSG) {
          green++;
        }
        if (green > SSG) {
          green--;
        }
        if (blue < SSB) {
          blue++;
        }
        if (blue > SSB) {
          blue--;
        }
        break;

      case 3:
        //Serial.println("Night");
        tod[8] = "NIGHT";
        if (red < nightR) {
          red++;
        }
        if (red > nightR) {
          red--;
        }
        if (green < nightG) {
          green++;
        }
        if (green > nightG) {
          green--;
        }
        if (blue < nightB) {
          blue++;
        }
        if (blue > nightB) {
          blue--;
        }
        break;
    }
  }


  u8g.firstPage();
  do {
    displayTime();
    draw();
    u8g.setFont(u8g_font_blipfest_07r);
    u8g.setPrintPos(43, 5);
    uptime::calculateUptime();
    if (uptime::getDays() < 10)
    {
      u8g.print(F("0"));
    }
    if (uptime::getDays() < 100)
    {
      u8g.print(F("0"));
    }
    u8g.print(uptime::getDays());
    u8g.print(F(":"));
    if (uptime::getHours() < 10)
    {
      u8g.print(F("0"));
    }
    u8g.print(uptime::getHours());
    u8g.print(F(":"));
    if (uptime::getMinutes() < 10)
    {
      u8g.print(F("0"));
    }
    u8g.print(uptime::getMinutes());
    u8g.print(F(":"));
    if (uptime::getSeconds() < 10)
    {
      u8g.print(F("0"));
    }
    u8g.print(uptime::getSeconds());

  } while (
    u8g.nextPage() );

  //wtemp
  if (millis() >= time_now + period)
  {
    time_now += period;
    sensors.requestTemperatures();
    Celcius = sensors.getTempCByIndex(0);
    Fahrenheit = sensors.toFahrenheit(Celcius);
    wtemp = Fahrenheit;
    //temp humid
    int h = DHT.humidity;
    int f = DHT.temperature;
    DHT.read11(DHT11_PIN);
    temp = DHT.temperature;
    humid = DHT.humidity;

    //fan control
    int temp = (f * 1.8 + 32);
    int humid = h;
    if (humid <= 60 )
    {
      analogWrite(pwm, 0);
      fanspd = 1;
    }
    else if ((humid <= 67) && (temp >= 70))
    {
      analogWrite(pwm, 35 );
      fanspd = 20;
    }
    else if ((humid <= 72) && (temp >= 70))
    {
      analogWrite(pwm, 77.5);
      fanspd = 40;
    }
    else if ((humid <= 79) && (temp >= 70))
    {
      analogWrite(pwm, 120);
      fanspd = 60;
    }
    else if ((humid <= 85) && (temp >= 70))
    {
      analogWrite(pwm, 162.5);
      fanspd = 80;
    }
    else if ((humid <= 100) && (temp >= 70))
    {
      analogWrite(pwm, 200);
      fanspd = 100;
    }
  }
  //set colour or RGB, 255 off, 0 full
  setColor(red, green, blue);
  //debug serial print RBG values for troubleshooting
  //Serial.print("Red:");
  //Serial.println(red);
  //Serial.print("Green:");
  //Serial.println(green);
  //Serial.print("Blue:");
  //Serial.println(blue);
  //Serial.println("");
}

void draw()
{
  u8g.setFont(u8g_font_fixed_v0);
  u8g.drawFrame(0, 12, 128, 48);
  u8g.setPrintPos(71, 23);
  u8g.print("Water:");
  u8g.setPrintPos(105, 23);
  u8g.print(Fahrenheit, 0); u8g.setPrintPos(116, 21); u8g.write(0xB0);
  u8g.setPrintPos(71, 38);
  u8g.print(F("Terra:"));
  u8g.setPrintPos(105, 38);
  u8g.print(temp * 1.8 + 32, 0); u8g.setPrintPos(116, 36); u8g.write(0xB0);
  u8g.setPrintPos(71, 55);
  u8g.print(F("Humid:"));
  u8g.setPrintPos(105, 55);
  u8g.print(humid, 0); u8g.write(0x25);
  //left column
  u8g.setPrintPos(6, 23);
  u8g.print(F("Relay01"));
  u8g.setPrintPos(6, 38);
  u8g.print(F("Relay02"));
  u8g.setPrintPos(6, 55);
  u8g.print(F("Fan:"));
  u8g.print(fanspd, 0); u8g.write(0x25);
  u8g.setFont(u8g_font_blipfest_07r);
  u8g.setPrintPos(0, 11);
  u8g.print(tod[8]);
  //Current RGB values
  u8g.setPrintPos(39, 11);
  u8g.print("R:");
  u8g.print(red / 25.5,1);
  u8g.setPrintPos(56, 11);
  u8g.print("B:");
  u8g.print(blue / 25.5,1 );
  u8g.setPrintPos(73, 11);
  u8g.print("G:");
  u8g.print(green / 25.5,1);
}
void setColor(int red, int green, int blue)
{
  analogWrite(RPin, red);
  analogWrite(GPin, green);
  analogWrite(BPin, blue);
}
