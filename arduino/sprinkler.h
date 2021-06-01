#ifndef SPRINKLER_H
#define SPRINKLER_H

#include <Arduino.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <TimeAlarms.h>
#include <vector>
#include <functional>
#include "schedule.h"
#include "sprinkler-device.h"

typedef std::function<void()> Delegate;

class SprinklerClass
{
private:

  std::vector<Delegate> onChangeEventHandlers;
  
  SprinklerDevice* device;
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

  void handle(ScheduleClass& sdk)
  {
    duration = sdk.getDuration() * 1000 * 60;

    times = 5;

    start();
  }

  void everydayHandler()
  {
    handle(Schedule);
  }

  void mondayHandler()
  {
    handle(Schedule.Mon);
  }

  void tuesdayHandler()
  {
    handle(Schedule.Tue);
  }

  void wednsdayHandler()
  {
    handle(Schedule.Wed);
  }

  void thursdayHandler()
  {
    handle(Schedule.Thu);
  }

  void fridayHandler()
  {
    handle(Schedule.Fri);
  }

  void saturdayHandler()
  {
    handle(Schedule.Sat);
  }

  void sundayHandler()
  {
    handle(Schedule.Sun);
  }

public:

  SprinklerClass() : times(0), duration(0), startTime(0), pauseTime(0)
  {
    Schedule.set(std::bind(&SprinklerClass::everydayHandler, this));
    Schedule.Sun.set(std::bind(&SprinklerClass::sundayHandler, this));
    Schedule.Mon.set(std::bind(&SprinklerClass::mondayHandler, this));
    Schedule.Tue.set(std::bind(&SprinklerClass::tuesdayHandler, this));
    Schedule.Wed.set(std::bind(&SprinklerClass::wednsdayHandler, this));
    Schedule.Thu.set(std::bind(&SprinklerClass::thursdayHandler, this));
    Schedule.Fri.set(std::bind(&SprinklerClass::fridayHandler, this));
    Schedule.Sat.set(std::bind(&SprinklerClass::saturdayHandler, this));
  }

  void setup(SprinklerDevice& d)
  {
    d.load();

    onAction = [&]() {
      d.turnOn();
    };

    offAction = [&]() {
      d.turnOff();
    };

    device = &d;
  }

  void onChange(Delegate event)
  {
    onChangeEventHandlers.push_back(event);
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
           " \"on\": " + 
           (String)(startTime ? "1" : "0") + "," +
           "\"started\":" +
           (String)(startTime ? "true" : "false") + "," +
           "\"paused\":" +
           (String)(pauseTime ? "true" : "false") + "," +
           "\"time\": \"" +
           (String)hour() + ":" + (String)minute() + "\"" +
           "\r\n}";
  }

  bool isWatering()
  {
    return startTime ? true : false;
  }

  void start()
  {
     if (duration)
      countdown.once_ms(duration - 10000, std::bind(&SprinklerClass::startNextZone, this));

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
        countdown.once(10, std::bind(&SprinklerClass::start, this));
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
      
      if (enable != -1)
      {
        if (enable == 1)
        {
          skd.enable();
        }
        else
        {
          skd.disable();
        }
        if (device) device->save();
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

      if (enable != -1)
      {
        if (enable == 1)
        {
          Schedule.enable();
        }
        else
        {
          Schedule.disable();
        }
        if (device) device->save();
      }
    }
  }

  virtual void reset()
  {
    if (device) device->reset();
  }

  virtual void restart()
  {
    if (device) device->restart();
  }
};

extern SprinklerClass Sprinkler = SprinklerClass();

#endif
