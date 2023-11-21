#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/time.h"

const uint trigPin = 0;
const uint echoPin = 26;
int timeout = 38000;
struct repeating_timer repeating_timer;

// Define a flag to control the timer
bool timer_enabled = true;

void setupUltrasonicPins(uint trigPin, uint echoPin) {
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);
    gpio_set_dir(echoPin, GPIO_IN);
}

uint64_t getCm(uint trigPin, uint echoPin) {
    gpio_put(trigPin, 1);
    busy_wait_us(10);  // Use busy_wait_us instead of sleep_us for non-blocking wait
    gpio_put(trigPin, 0);

    uint64_t width = 0;
    uint64_t max_width = timeout;

    // Wait for the echo signal to go HIGH with a timeout
    while (gpio_get(echoPin) == 0 && max_width > 0) {
        busy_wait_us(1);  // Use busy_wait_us for non-blocking wait
        max_width--;
    }

    if (max_width <= 0) {
        return 0;  // Return 0 if the timeout is reached
    }

    absolute_time_t startTime = get_absolute_time();
    max_width = timeout;

    // Measure the pulse width of the echo signal
    while (gpio_get(echoPin) == 1 && max_width > 0) {
        width++;
        busy_wait_us(1);  // Use busy_wait_us for non-blocking wait
        max_width--;
    }

    if (max_width <= 0) {
        return 0;  // Return 0 if the timeout is reached
    }

    absolute_time_t endTime = get_absolute_time();
    return absolute_time_diff_us(startTime, endTime) / 29 / 2;
}

bool repeating_timer_callback(struct repeating_timer* t) {
        uint64_t distance_cm = getCm(trigPin, echoPin);
        printf("Distance in cm: %llu\n", distance_cm);

        // Check if the distance is less than 7 cm
        if (distance_cm < 7) {
            printf("Distance is less than 7 cm.\n");
            cancel_repeating_timer(&repeating_timer);  // Stop the timer
        }
    return true;
}

int main() {
    stdio_init_all();
    setupUltrasonicPins(trigPin, echoPin);

    // Create the repeating timer structure.
    //struct repeating_timer repeating_timer;

    // Set up the timer callback to trigger every 1000ms (1 second).
    add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &repeating_timer);
    
    while (1) {
        tight_loop_contents();
    }

    return 0;
}
