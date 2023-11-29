#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/time.h"


int timeout = 38000;  // Timeout for echo signal in microseconds

// Function to set up ultrasonic sensor pins
void setupUltrasonicPins(uint trigPin, uint echoPin) {
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);
    gpio_set_dir(echoPin, GPIO_IN);
}

// Function to get distance from ultrasonic sensor
float getDistance(uint trigPin, uint echoPin) {
    // Send a 10us pulse to trigger the ultrasonic sensor
    gpio_put(trigPin, 1);
    busy_wait_us(10);
    gpio_put(trigPin, 0);

    // Wait for the echo signal to go HIGH with a timeout
    uint64_t width = 0;
    uint64_t max_width = timeout;
    while (gpio_get(echoPin) == 0 && max_width > 0) {
        busy_wait_us(1);
        max_width--;
    }

    // Check for timeout
    if (max_width <= 0) {
        return 0.0;
    }

    // Measure the pulse width of the echo signal
    absolute_time_t startTime = get_absolute_time();
    max_width = timeout;
    while (gpio_get(echoPin) == 1 && max_width > 0) {
        width++;
        busy_wait_us(1);
        max_width--;
    }

    // Check for timeout
    if (max_width <= 0) {
        return 0.0;
    }

    // Calculate and return the distance in centimeters
    absolute_time_t endTime = get_absolute_time();
    return roundf(10.0 * absolute_time_diff_us(startTime, endTime) / 29.0 / 2.0) / 10.0;
}