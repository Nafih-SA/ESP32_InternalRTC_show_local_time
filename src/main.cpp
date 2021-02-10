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
int eeprom_address = 0;
bool flag = false;

void printLocalTime()
{
  if (!getLocalTime(&timeinfo) && !flag)
  {
    Serial.println("Failed to obtain time");
    // time_t t_of_day;
    timeinfo.tm_year = 2021 - 1900;
    timeinfo.tm_mon = 0;
    timeinfo.tm_mday = 1;
    // timeinfo.tm_wday = 1;
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
    timeinfo.tm_isdst = -1;
    time_t t_of_day;
    t_of_day = mktime(&timeinfo);
    struct timeval tv;
    tv.tv_sec = t_of_day; // epoch time (seconds)
    tv.tv_usec = 0;       // microseconds
    settimeofday(&tv, 0);
    flag = true;
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
  while (WiFi.status() != WL_CONNECTED)
  {
    int count = 0;
    count++;
    delay(500);
    Serial.print(".");
    if(count>10)
      break;
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
  delay(1000);
  printLocalTime();
}