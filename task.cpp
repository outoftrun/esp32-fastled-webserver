/*
   ESP32 FastLED WebServer: https://github.com/jasoncoon/esp32-fastled-webserver
   Copyright (C) 2017 Jason Coon

   Built upon the amazing FastLED work of Daniel Garcia and Mark Kriegsman:
   https://github.com/FastLED/FastLED

   ESP32 support provided by the hard work of Sam Guyer:
   https://github.com/samguyer/FastLED

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <SPIFFS.h>
#include <EEPROM.h>



extern WebServer webServer;



struct params_t
{
  uint32_t from;
  uint32_t to;
};

const uint32_t ARRAY_SIZE = 1024 * 64;

uint8_t array[ARRAY_SIZE];
SemaphoreHandle_t semaphore;

uint8_t RTC_DATA_ATTR tasks = 0;

static void taskProducer(void *pvParam)
{
  for (uint32_t i = ((params_t *)pvParam)->from; i < ((params_t *)pvParam)->to; ++i)
  {
    array[i] = random(256);
  }
  xSemaphoreGive(semaphore);
  vTaskDelete(NULL);
}

static void halt(const char *msg)
{
  Serial.println(msg);
  Serial.println("System halted!");
  Serial.flush();
  esp_deep_sleep_start();
}



void setup_tasks()
{

  if (!tasks)
    tasks = 1;
  else
    tasks *= 2;

  if (tasks == 1)
    semaphore = xSemaphoreCreateBinary();
  else
    semaphore = xSemaphoreCreateCounting(tasks, 0);
  if (!semaphore)
    halt("Error creating semaphore!");

  Serial.printf("*** Creating %u task(s) ***\r\n", tasks);

  uint32_t time = micros();

  for (uint8_t i = 0; i < tasks; ++i)
  {
    params_t params;

    params.from = ARRAY_SIZE / tasks * i;
    params.to = params.from + ARRAY_SIZE / tasks;
    if (xTaskCreatePinnedToCore(&taskProducer, "producer", 1024, &params, 1, NULL, i & 0x01) != pdPASS)
      halt("Error creating task!");
    //    Serial.printf("Task #%u (%u .. %u)\r\n", i, params.from, params.to - 1);
  }

  for (uint8_t i = 0; i < tasks; ++i)
  {
    xSemaphoreTake(semaphore, portMAX_DELAY);
  }
  time = micros() - time;
  Serial.printf("Execution time: %u us.\r\n", time);
  Serial.flush();
  if (tasks >= 8)
    esp_deep_sleep_start();
  else
  {
    esp_deep_sleep_disable_rom_logging();
    esp_deep_sleep(0);
  }
}

