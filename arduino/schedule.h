#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <Arduino.h>
#include <Ticker.h>
#include <Time.h>
#include <TimeAlarms.h>

class ScheduleClass
{
protected:
  unsigned int Duration;
  AlarmID_t AlarmID;
  OnTick_t OnTick;
  time_t Time;

public:
  ScheduleClass()
      : Time(0), Duration(0), AlarmID(dtINVALID_ALARM_ID)
  {
  }

  void set(OnTick_t func) 
  {
    OnTick = func;
  }

  bool isEnabled()
  {
    return Alarm.isAllocated(AlarmID);
  }

  void disable()
  {
    if (Alarm.isAllocated(AlarmID))
    {
      Alarm.free(AlarmID);
      AlarmID = dtINVALID_ALARM_ID;
    }
  }

  virtual bool enable() = 0;

  unsigned int getDuration()
  {
    return Duration;
  }

  virtual void setDuration(int value)
  {
    disable();

    Duration = value;
  }

  unsigned int getHour()
  {
    return hour(Time);
  }

  virtual void setHour(int value)
  {
    disable();

    tmElements_t te;
    breakTime(Time ? Time : now(), te);
    te.Hour = value;
    if (!Time)
    {
      te.Minute = 0;
    }
    Time = makeTime(te);
  }

  unsigned int getMinute()
  {
    return minute(Time);
  }

  virtual void setMinute(int value)
  {
    disable();
    
    tmElements_t te;
    breakTime(Time ? Time : now(), te);
    te.Minute = value;
    if (!Time)
    {
      te.Hour = 0;
    }
    Time = makeTime(te);
  }

  virtual String toJSON()
  {
    return "{ \"AlarmID\": " + (String)AlarmID + ", \"enabled\": " + (String)Alarm.isAllocated(AlarmID) + ", \"d\": " + (String)Duration + ", \"t\": \"" + (String)hour(Time) + ":" + (String)minute(Time) + "\" }";
  }

  friend class ScheduleDay;
  friend class ScheduleWeek;
};

class ScheduleDay : public ScheduleClass
{
protected:
  timeDayOfWeek_t Day;

public:
  ScheduleDay(timeDayOfWeek_t day)
      : ScheduleClass(),
        Day(day)
  {
  }

  bool enable() override
  {
    disable();

    AlarmID = Alarm.alarmRepeat(Day, hour(Time), minute(Time), 0, OnTick);

    return Alarm.isAllocated(AlarmID);
  }
};

class ScheduleWeek : public ScheduleClass
{

public:
  ScheduleDay Mon;
  ScheduleDay Tue;
  ScheduleDay Wed;
  ScheduleDay Thu;
  ScheduleDay Fri;
  ScheduleDay Sat;
  ScheduleDay Sun;

  ScheduleWeek()
      : ScheduleClass(),
        Mon(dowMonday),
        Tue(dowTuesday),
        Wed(dowWednesday),
        Thu(dowThursday),
        Fri(dowFriday),
        Sat(dowSaturday),
        Sun(dowSunday)
  {
  }

  void attach() 
  {
    if (!Duration)
    {
      for (int day = (int)dowSunday; day <= (int)dowSaturday; day++)
      {
        ScheduleClass &skd = get((timeDayOfWeek_t)day);
        if (skd.Duration) {
          skd.enable();
        }
      }
    }
    else
    {
      disable();
      AlarmID = Alarm.alarmRepeat(hour(Time), minute(Time), 0, OnTick);
    }
  }

  bool enable() override
  {
    for (int day = (int)dowSunday; day <= (int)dowSaturday; day++)
    {
      ScheduleClass &skd = get((timeDayOfWeek_t)day);

      skd.disable();
      skd.Time = Time;
      skd.Duration = Duration;
    }

    disable();

    AlarmID = Alarm.alarmRepeat(hour(Time), minute(Time), 0, OnTick);

    return Alarm.isAllocated(AlarmID);
  }

  ScheduleClass &get(timeDayOfWeek_t day)
  {
    switch (day)
    {
    case dowMonday:
      return Mon;
    case dowTuesday:
      return Tue;
    case dowWednesday:
      return Wed;
    case dowThursday:
      return Thu;
    case dowFriday:
      return Fri;
    case dowSaturday:
      return Sat;
    case dowSunday:
      return Sun;
    }
  }

  String toJSON() override
  {
    return "{\r\n" +
           ((Mon.isEnabled()) ? " \"mon\": " + (String)Mon.toJSON() + ",\r\n" : "") + "" +
           ((Tue.isEnabled()) ? " \"tue\": " + (String)Tue.toJSON() + ",\r\n" : "") + "" +
           ((Wed.isEnabled()) ? " \"wed\": " + (String)Wed.toJSON() + ",\r\n" : "") + "" +
           ((Thu.isEnabled()) ? " \"thu\": " + (String)Thu.toJSON() + ",\r\n" : "") + "" +
           ((Fri.isEnabled()) ? " \"fri\": " + (String)Fri.toJSON() + ",\r\n" : "") + "" +
           ((Sat.isEnabled()) ? " \"sat\": " + (String)Sat.toJSON() + ",\r\n" : "") + "" +
           ((Sun.isEnabled()) ? " \"sun\": " + (String)Sun.toJSON() + ",\r\n" : "") + "" +
           " \"AlarmID\": " + (String)AlarmID + ", \"enabled\": " + (String)isEnabled() + ", \"d\": " + (String)Duration + ", \"t\": \"" + (String)hour(Time) + ":" + (String)minute(Time) + "\"" +
           "\r\n}";
  }
};

extern ScheduleWeek Schedule = ScheduleWeek();

#endif
