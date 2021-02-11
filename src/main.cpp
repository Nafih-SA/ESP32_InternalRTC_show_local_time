#include <Arduino.h>

#include <WiFi.h>
#include "time.h"
#include "EEPROM.h"

const char *ssid = "Elacsta";
const char *password = "112131123";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;
struct tm timeinfo;
struct tm timeinfo_temp;
int eeprom_address = 0;
bool time_get_flag = false;
bool wifi_notconnect_flag = false;
unsigned long previousTime = 0;

void printLocalTime()
{
  if (!getLocalTime(&timeinfo) && !time_get_flag)
  {
    Serial.println("Failed to obtain time\nUsing locally stored values");
    // time_t t_of_day;
    eeprom_address = 0;
    timeinfo.tm_year = EEPROM.readInt(eeprom_address);
    eeprom_address += sizeof(int);
    timeinfo.tm_mon = EEPROM.readInt(eeprom_address);
    eeprom_address += sizeof(int);
    timeinfo.tm_mday = EEPROM.readInt(eeprom_address);
    eeprom_address += sizeof(int);
    // timeinfo.tm_wday = 1;
    timeinfo.tm_hour = EEPROM.readInt(eeprom_address);
    eeprom_address += sizeof(int);
    timeinfo.tm_min = EEPROM.readInt(eeprom_address);
    eeprom_address += sizeof(int);
    timeinfo.tm_sec = EEPROM.readInt(eeprom_address);
    eeprom_address += sizeof(int);
    timeinfo.tm_isdst = -1;
    time_t t_of_day;
    t_of_day = mktime(&timeinfo);
    struct timeval tv;
    tv.tv_sec = t_of_day; // epoch time (seconds)
    tv.tv_usec = 0;       // microseconds
    settimeofday(&tv, 0);
    time_get_flag = true;
    return;
  }

  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setup()
{
  Serial.begin(115200);
  //Initialize EEPROM
  if (!EEPROM.begin(1000))
  {
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    count++;
    delay(500);
    Serial.print(".");
    if (count > 20)
    {
      Serial.print(" NOT");
      wifi_notconnect_flag = true;
      break;
    }
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop()
{
  unsigned long currentTime = millis();
  if ((currentTime - previousTime) > 60000)
  {
    eeprom_address = 0;
    previousTime = currentTime;

    EEPROM.writeInt(eeprom_address, timeinfo.tm_year); // Year
    eeprom_address += sizeof(int);
    EEPROM.writeInt(eeprom_address, timeinfo.tm_mon); // Month
    eeprom_address += sizeof(int);
    EEPROM.writeInt(eeprom_address, timeinfo.tm_mday); // Day
    eeprom_address += sizeof(int);
    EEPROM.writeInt(eeprom_address, timeinfo.tm_hour); // Hour
    eeprom_address += sizeof(int);
    EEPROM.writeInt(eeprom_address, timeinfo.tm_min); // Min
    eeprom_address += sizeof(int);
    EEPROM.writeInt(eeprom_address, timeinfo.tm_sec); // Sec
    eeprom_address += sizeof(int);
    EEPROM.commit();
    Serial.println("Saved latest time to EEPROM");

    if (wifi_notconnect_flag)
    {
      WiFi.begin(ssid, password);
      int count = 0;
      while (WiFi.status() != WL_CONNECTED)
      {
        wifi_notconnect_flag = false;
        count++;
        delay(500);
        if (count > 10)
        {
          wifi_notconnect_flag = true;
          Serial.print("NOT ");
          break;
        }
      }

      Serial.println(" CONNECTED");
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      if(!getLocalTime(&timeinfo_temp))
      {
        Serial.println("Failed to obtain time");
        wifi_notconnect_flag = true;
      }
      delay(1000);

      WiFi.disconnect(true);
      WiFi.mode(WIFI_OFF);
    }
  }
  delay(1000);
  printLocalTime();
}