#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/time.h"

const uint trigPin = 0;
const uint echoPin = 1;

#ifndef ultrasonic_h
#define ultrasonic_h
void setupUltrasonicPins(uint trigPin, uint echoPin);
float getDistance(uint trigPin, uint echoPin);
#endif