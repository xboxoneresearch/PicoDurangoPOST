#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "version.h"

#define __DEBUG__

#if defined(__DEBUG__)
#define DBG(format, ...) Serial.printf("[DBG] "#format"\r\n", ##__VA_ARGS__)
#else
#define DBG(x, ...)
#endif