#ifndef SPRINKLER_H
#define SPRINKLER_H

#include <Arduino.h>
#include <Ticker.h>
#include <TimeAlarms.h>
#include <vector>
#include <functional>
#include "schedule.h"

typedef std::function<void()> Delegate;

void startSprinkler();
void startNextSprinklerZone();
void startSprinklerEveryday();
void startSprinklerMonday();
void startSprinklerTuesday();
void startSprinklerWednesday();
void startSprinklerThursday();
void startSprinklerFriday();
void startSprinklerSaturday();
void startSprinklerSunday();

class SprinklerClass
{
private:

  std::vector<Delegate> onChangeEventHandlers;
  
  Delegate onAction;
  Delegate offAction;
  
  unsigned int times;
  unsigned int duration;
  unsigned long startTime;
  unsigned long pauseTime;
  Ticker countdown;

  void notify()
  {
    for (auto &event : onChangeEventHandlers) // access by reference to avoid copying
    {  
      event();
    }
  }

public:
  SprinklerClass() : times(0), duration(0), startTime(0), pauseTime(0)
  {
  }

  void onChange(Delegate event)
  {
    onChangeEventHandlers.push_back(event);
  }

  void onTurnOn(Delegate onCallback)
  {
    onAction = onCallback;
  }

  void onTurnOff(Delegate offCallback)
  {
    offAction = offCallback;
  }

  unsigned int getDuration()
  {
    return duration;
  }

  void setDuration(unsigned int miliseconds)
  {
    duration = miliseconds;
    notify();
  }

  void setTimes(unsigned int number)
  {
    times = number;
    notify();
  }

  String toJSON()
  {
    return "{\r\n"
           "\"zones\":" +
           (String)(times) + "," +
           "\"timer\":" +
           (String)(startTime ? (duration ? duration - (millis() - startTime) + (pauseTime ? (millis() - pauseTime) : 0) : 0) : 0) + "," +
           "\"started\":" +
           (String)(startTime ? "true" : "false") + "," +
           "\"paused\":" +
           (String)(pauseTime ? "true" : "false") + "," +
           "\"time\": \"" +
           (String)hour() + ":" + (String)minute() + "\"" +
           "\r\n}";
  }

  void startEveryday()
  {
    duration = Schedule.getDuration() * 1000 * 60;
    times = 5;
    start();
  }

  void startDay(timeDayOfWeek_t day)
  {
    ScheduleClass &skd = Schedule.get(day);
    duration = skd.getDuration() * 1000 * 60;
    times = 5;
    start();
  }

  bool isWatering()
  {
    return startTime ? true : false;
  }

  void start()
  {
    if (duration)
      countdown.once_ms(duration - 10000, startNextSprinklerZone);

    onAction();
    startTime = millis();
    pauseTime = 0;
    notify();  
  }

  void startNextZone()
  {
    if (times)
    {
      times--;
    }
  
    if (startTime)
    {
      if (times)
      {
        offAction();
        countdown.once(10, startSprinkler);
      }
      else
      {
        stop();
      }
    }

    notify();  
  }

  void stop()
  {
    offAction();
    startTime = 0;
    pauseTime = 0;
    countdown.detach();
    notify();  
  }

  void pause()
  {
    offAction();

    pauseTime = millis();

    notify();  
  }

  void resume()
  {
    onAction();

    startTime += (millis() - pauseTime);
    pauseTime = 0;
    
    notify();  
  }

  void schedule(timeDayOfWeek_t day, int hours, int minutes, int duration, int enable)
  {
    if (timeStatus() != timeNotSet)
    {
      if (enable == 1)
      {
        Schedule.disable();
      }

      ScheduleClass &skd = Schedule.get(day);

      if (hours != -1)
        skd.setHour(hours);

      if (minutes != -1)
        skd.setMinute(minutes);

      if (duration != -1)
        skd.setDuration(duration);

      if (enable == 1)
      {
        switch (day)
        {
        case dowMonday:
          skd.enable(startSprinklerMonday);
          break;
        case dowTuesday:
          skd.enable(startSprinklerTuesday);
          break;
        case dowWednesday:
          skd.enable(startSprinklerWednesday);
          break;
        case dowThursday:
          skd.enable(startSprinklerThursday);
          break;
        case dowFriday:
          skd.enable(startSprinklerFriday);
          break;
        case dowSaturday:
          skd.enable(startSprinklerSaturday);
          break;
        case dowSunday:
          skd.enable(startSprinklerSunday);
          break;
        }
      }
      else if (enable == 0)
      {
        skd.disable();
      }
    }
  }

  void schedule(int hours, int minutes, int duration, int enable)
  {
    if (timeStatus() != timeNotSet)
    {
      if (hours != -1)
        Schedule.setHour(hours);

      if (minutes != -1)
        Schedule.setMinute(minutes);

      if (duration != -1)
        Schedule.setDuration(duration);

      if (enable == 1)
      {
        Schedule.enable(startSprinklerEveryday);
      }
      else if (enable == 0)
      {
        Schedule.disable();
      }
    }
  }

  void restart()
  {
    Serial.println("[MAIN] Restarting...");
    system_restart();
  }

  void reset()
  {
    Serial.println("[MAIN] Factory reset...");
    WiFi.disconnect(true);
    SPIFFS.format();
    system_restart();
  }  
};

extern SprinklerClass Sprinkler = SprinklerClass();

void startSprinkler()
{
  Sprinkler.start();
}

void startNextSprinklerZone()
{
  Sprinkler.startNextZone();
}

void startSprinklerEveryday()
{
  Sprinkler.startEveryday();
}

void startSprinklerMonday()
{
  Sprinkler.startDay(dowMonday);
}

void startSprinklerTuesday()
{
  Sprinkler.startDay(dowTuesday);
}

void startSprinklerWednesday()
{
  Sprinkler.startDay(dowWednesday);
}

void startSprinklerThursday()
{
  Sprinkler.startDay(dowThursday);
}

void startSprinklerFriday()
{
  Sprinkler.startDay(dowFriday);
}

void startSprinklerSaturday()
{
  Sprinkler.startDay(dowSaturday);
}

void startSprinklerSunday()
{
  Sprinkler.startDay(dowSunday);
}

#endif
