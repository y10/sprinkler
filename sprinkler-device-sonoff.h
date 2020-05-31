#ifndef SPRINKLER_SONOFF_DEVICE_H
#define SPRINKLER_SONOFF_DEVICE_H

#define BTN_PIN 0
#define LED_PIN 13
#define RELAY_PIN 12

#include "sprinkler-device.h"

extern SprinklerDevice Device = SprinklerDevice([]()
{
    pinMode(LED_PIN, OUTPUT);

    pinMode(BTN_PIN, INPUT_PULLDOWN_16);

    pinMode(RELAY_PIN, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(BTN_PIN), []() 
    {
        if (Sprinkler.isWatering())
        {
        Sprinkler.stop();
        }
        else
        {
        Sprinkler.start();
        }
    }, CHANGE);
    
});

#endif
