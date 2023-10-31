/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Output PWM signals on pins 0 and 1
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define LEFT_MOTOR_PIN_A 15
#define LEFT_MOTOR_PIN_B 14
#define RIGHT_MOTOR_PIN_A 11
#define RIGHT_MOTOR_PIN_B 10
#define TIME_INTERVAL_MS 1

#define DIST_GAUGE 1.0210176

static char event_str[128];

struct repeating_timer timer;
volatile bool timer_running = false;
volatile uint32_t elapsed_time = 0;
uint32_t start_time;
int rise_counter = 0;

void gpio_event_string(char *buf, uint32_t events);

void setMotorDirections(bool left_forward, bool right_forward) {
    gpio_put(LEFT_MOTOR_PIN_A, !left_forward);
    gpio_put(LEFT_MOTOR_PIN_B, left_forward);
    gpio_put(RIGHT_MOTOR_PIN_A, !right_forward);
    gpio_put(RIGHT_MOTOR_PIN_B, right_forward);
}

void turnMotorDirections(bool left_forward, bool right_forward) {
    gpio_put(LEFT_MOTOR_PIN_A, left_forward);
    gpio_put(LEFT_MOTOR_PIN_B, !left_forward);
    gpio_put(RIGHT_MOTOR_PIN_A, right_forward);
    gpio_put(RIGHT_MOTOR_PIN_B, !right_forward);
}

void calculate_speed() {

    uint32_t current_time = time_us_32();

    uint32_t elapsed_time = current_time - start_time;

    float speed = DIST_GAUGE / (elapsed_time/1000);

    // printf("Current elapsed time %d us\n", elapsed_time/1000);
    printf("Current speed is %f cm/ms \n", speed);
}

void calculate_distance() {
    float total_distance = rise_counter * DIST_GAUGE;

    printf("Total distance travelled %0.2f cm \n", total_distance);
}

void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it
    gpio_event_string(event_str, events);

    rise_counter = rise_counter + 1;
    calculate_speed();
    start_time = time_us_32();
    
    // if (strcmp(event_str, "EDGE_FALL") == 0) {
    //     rise_counter = rise_counter + 1;

    //     // calculate_speed();

    // }
}

static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

void gpio_event_string(char *buf, uint32_t events) {
    for (uint i = 0; i < 4; i++) {
        uint mask = (1 << i);
        if (events & mask) {
            // Copy this event string into the user string
            const char *event_str = gpio_irq_str[i];
            while (*event_str != '\0') {
                *buf++ = *event_str++;
            }
            events &= ~mask;

            // If more events add ", "
            if (events) {
                *buf++ = ',';
                *buf++ = ' ';
            }
        }
    }
    *buf++ = '\0';
}

int main() {
    stdio_init_all();
    
    //Left wheel
    gpio_init(LEFT_MOTOR_PIN_A);
    gpio_set_dir(LEFT_MOTOR_PIN_A, GPIO_OUT);
    gpio_init(LEFT_MOTOR_PIN_B);
    gpio_set_dir(LEFT_MOTOR_PIN_B, GPIO_OUT);
    
    //Right wheel
    gpio_init(RIGHT_MOTOR_PIN_A);
    gpio_set_dir(RIGHT_MOTOR_PIN_A, GPIO_OUT);
    gpio_init(RIGHT_MOTOR_PIN_B);
    gpio_set_dir(RIGHT_MOTOR_PIN_B, GPIO_OUT);

    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
    gpio_set_function(1, GPIO_FUNC_PWM);

    gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_clkdiv(slice_num, 100);
    // Set period of 4 cycles (0 to 3 inclusive)
    pwm_set_wrap(slice_num, 12500);
    // Set the PWM running
    pwm_set_enabled(slice_num, true);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 25000/2);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 25000/2);

    while(1) {

        // Set channel A output high for one cycle before dropping

        printf("Enter 1 to move forward, 2 to move reverse: ");
        char userInput = getchar();

        if (userInput == '1') {
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 1);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 1);
            // setMotorDirections(true, true);
            // printf("Moving forward!\n");
        } else if (userInput == '2') {
            gpio_put(LEFT_MOTOR_PIN_A, 1);
            gpio_put(LEFT_MOTOR_PIN_B, 0);
            gpio_put(RIGHT_MOTOR_PIN_A, 1);
            gpio_put(RIGHT_MOTOR_PIN_B, 0);
            // setMotorDirections(false, false);
            // printf("Moving backwards!\n");

        } else if (userInput == '3') {
            gpio_put(LEFT_MOTOR_PIN_A, 1);
            gpio_put(LEFT_MOTOR_PIN_B, 0);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 1);
            // turnMotorDirections(true, true);
            // printf("Turning left!\n");

        } else if (userInput == '4') {
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 1);
            gpio_put(RIGHT_MOTOR_PIN_A, 1);
            gpio_put(RIGHT_MOTOR_PIN_B, 0);
            // turnMotorDirections(false, false);
            // printf("Turning right!\n");

        } else if (userInput == '5') {
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 0);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 0);
            calculate_distance();

        } else if (userInput == '6') {
            pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/4);
            pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/4);

        } else if (userInput == '7') {
            pwm_set_chan_level(slice_num, PWM_CHAN_A, 25000/2);
            pwm_set_chan_level(slice_num, PWM_CHAN_B, 25000/2);

        } else {
            printf("Re-enter input!\n");
        };
    
        
    };
}
