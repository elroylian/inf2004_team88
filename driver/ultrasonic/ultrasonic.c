#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/time.h"

// const uint trigPin = 0;
// const uint echoPin = 1;
int timeout = 38000;
struct repeating_timer repeating_timer;

// Zero error adjustment
float zero_error = -0.3;

void setupUltrasonicPins(uint trigPin, uint echoPin) {
    gpio_init(trigPin);
    gpio_init(echoPin);
    gpio_set_dir(trigPin, GPIO_OUT);
    gpio_set_dir(echoPin, GPIO_IN);
}

float getDistance(uint trigPin, uint echoPin) {
    gpio_put(trigPin, 1);
    busy_wait_us(10);
    gpio_put(trigPin, 0);

    uint64_t width = 0;
    uint64_t max_width = timeout;

    // Wait for the echo signal to go HIGH with a timeout
    while (gpio_get(echoPin) == 0 && max_width > 0) {
        busy_wait_us(1);
        max_width--;
    }

    if (max_width <= 0) {
        return 0.0;
    }

    absolute_time_t startTime = get_absolute_time();
    max_width = timeout;

    // Measure the pulse width of the echo signal
    while (gpio_get(echoPin) == 1 && max_width > 0) {
        width++;
        busy_wait_us(1);
        max_width--;
    }

    if (max_width <= 0) {
        return 0.0;
    }

    absolute_time_t endTime = get_absolute_time();
    // Return the distance in centimeters as a float, rounded to 1 decimal place
    return roundf(10.0 * absolute_time_diff_us(startTime, endTime) / 29.0 / 2.0) / 10.0;
}

// bool repeating_timer_callback(struct repeating_timer* t) {
//     float distance_cm = getDistance(trigPin, echoPin);
//     printf("Distance of Obstruction: %.1fcm\n", distance_cm);

//     // Check if the adjusted distance is less than 7 cm
//     if (distance_cm < 7.0) {
//         printf("Distance is less than 7 cm.\n");
//         // Perform other actions here.
//     }
//     return true;
// }

// int main() {
//     stdio_init_all();
//     setupUltrasonicPins(trigPin, echoPin);

//     // Set up the timer callback to trigger every 1000ms (1 second).
//     add_repeating_timer_ms(1000, repeating_timer_callback, NULL, &repeating_timer);

//     while (1) {
//         tight_loop_contents();
//     }

//     return 0;
// }
