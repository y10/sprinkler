#ifndef SPRINKLER_EDRAGON_DEVICE_H
#define SPRINKLER_EDRAGON_DEVICE_H

#define BTN_PIN 2
#define LED_PIN 16
#define RELAY_PIN 12
#define RELAY_PIN2 13

#include "sprinkler-device.h"

extern SprinklerDevice Device = SprinklerDevice(RELAY_PIN, LED_PIN, BTN_PIN);

#endif
