/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Output PWM signals on pins 0 and 1
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define LEFT_MOTOR_PIN_A 15
#define LEFT_MOTOR_PIN_B 14
#define RIGHT_MOTOR_PIN_A 11
#define RIGHT_MOTOR_PIN_B 10
#define TIME_INTERVAL_MS 1

#define ADC_PIN_LEFT 0
#define ADC_PIN_RIGHT 1

#define IRPOWER_LEFT 22
#define IRPOWER_RIGHT 21

#define DIST_GAUGE 1.0210176

static char event_str[128];

struct repeating_timer timer;
volatile bool timer_running = false;
volatile uint32_t elapsed_time = 0;
uint32_t start_time_right;
uint32_t start_time_left;
int rise_counter = 0;

// const uint8_t ADC_PIN_LEFT = 0; // Use ADC 0
// const uint8_t ADC_PIN_LEFT = 1; // Use ADC 1
const uint16_t THRESHOLD = 1000; // ADC threshold value for black vs white
const uint8_t PERIOD = 100; // Set timer period to 100 us
volatile uint16_t RESULT_ADC_LEFT;
volatile uint16_t RESULT_ADC_RIGHT;
struct repeating_timer timer;

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

void calculate_left() {

    float current_time = time_us_32();

    float elapsed_time_left = current_time - start_time_left;
    float speed_left = DIST_GAUGE / (elapsed_time_left/1000000);

    printf("Current left wheel speed is %f cm/s \n", speed_left);
}

void calculate_right() {
    
    float current_time = time_us_32();

    float elapsed_time_right = current_time - start_time_right;
    float speed_right = DIST_GAUGE / (elapsed_time_right/1000000);

    printf("Current right wheel speed is %f cm/s \n", speed_right);
}

void calculate_distance() {
    float total_distance = rise_counter * DIST_GAUGE;

    printf("Total distance travelled %0.2f cm \n", total_distance);
}

void gpio_callback(uint gpio, uint32_t events) {
    
    gpio_event_string(event_str, events);

    rise_counter = rise_counter + 1;

    if (gpio == 2) {
        calculate_right();
        start_time_right = time_us_32();
    }

    if (gpio == 3) {
        calculate_left();
        start_time_left = time_us_32();
    }

}

bool adc_callback(struct repeating_timer *t) {
    // Read adc value and print black or white
    uint16_t result_left = adc_read();
    uint16_t result_right = adc_read();
    RESULT_ADC_LEFT = result_left; 
    RESULT_ADC_RIGHT = result_right; 

    if (result_left >= THRESHOLD) {
        // Above or equal threshold
        printf("%u, BLACK DETECTED ON LEFT\n", result_left);
    } else {           
        // Below threshold
        printf("%u, WHITE DETECTED ON LEFT\n", result_left);
    }

    if (result_right >= THRESHOLD) {
        // Above or equal threshold
        printf("%u, BLACK DETECTED ON RIGHT\n", result_right);
    } else {           
        // Below threshold
        printf("%u, WHITE DETECTED ON RIGHT\n", result_right);
    }

    return true;
}

uint16_t read_adc_channel(uint channel) {
    adc_select_input(channel);
    return adc_read();
}

bool read_left_ir_sensor() {
    RESULT_ADC_LEFT = read_adc_channel(ADC_PIN_LEFT);
    RESULT_ADC_RIGHT = read_adc_channel(ADC_PIN_RIGHT);

    if (RESULT_ADC_LEFT >= THRESHOLD) {
        // Above or equal threshold
        printf("%u, BLACK DETECTED ON LEFT\n", RESULT_ADC_LEFT);
    } else {           
        // Below threshold
        printf("%u, WHITE DETECTED ON LEFT\n", RESULT_ADC_LEFT);
    }

    if (RESULT_ADC_RIGHT >= THRESHOLD) {
        // Above or equal threshold
        printf("%u, BLACK DETECTED ON RIGHT\n", RESULT_ADC_RIGHT);
    } else {           
        // Below threshold
        printf("%u, WHITE DETECTED ON RIGHT\n", RESULT_ADC_RIGHT);
    }
    
    return true;
}

bool read_right_ir_sensor() {
    RESULT_ADC_RIGHT = read_adc_channel(ADC_PIN_RIGHT);
    return true;
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
    adc_init();
    adc_select_input(26);
    adc_select_input(27);

    // add_repeating_timer_us(PERIOD, read_right_ir_sensor, NULL, &timer);
    add_repeating_timer_us(PERIOD, read_left_ir_sensor, NULL, &timer);

    //GP22
    gpio_init(IRPOWER_LEFT);
    gpio_set_dir(IRPOWER_LEFT, GPIO_OUT);
    gpio_put(IRPOWER_LEFT, 1);

    //GP21
    gpio_init(IRPOWER_RIGHT);
    gpio_set_dir(IRPOWER_RIGHT, GPIO_OUT);
    gpio_put(IRPOWER_RIGHT, 1);
    
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
    gpio_init(19);
    gpio_init(20);
    gpio_set_dir(19, GPIO_OUT);
    gpio_set_dir(20, GPIO_OUT);
    gpio_put(19, 0);
    gpio_put(20, 1);

    gpio_init(17);
    // gpio_init(16);
    gpio_set_dir(17, GPIO_OUT);
    // gpio_set_dir(16, GPIO_OUT);
    gpio_put(17, 1);
    // gpio_put(16, 1);



    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
    gpio_set_function(1, GPIO_FUNC_PWM);

    gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(3, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_clkdiv(slice_num, 100);
    // Set period of 4 cycles (0 to 3 inclusive)
    pwm_set_wrap(slice_num, 12500);
    // Set the PWM running
    pwm_set_enabled(slice_num, true);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/2);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/2);

    while(1) {

        // printf("Enter 1 to move forward, 2 to move reverse: ");
        // char userInput = getchar();

        if ((RESULT_ADC_LEFT < THRESHOLD) && (RESULT_ADC_RIGHT < THRESHOLD)) {
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 1);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 1);
        // } else if ((RESULT_ADC_LEFT >= THRESHOLD) && (true)) {
        //     gpio_put(LEFT_MOTOR_PIN_A, 0);
        //     gpio_put(LEFT_MOTOR_PIN_B, 1);
        //     gpio_put(RIGHT_MOTOR_PIN_A, 1);
        //     gpio_put(RIGHT_MOTOR_PIN_B, 0);
        //     // setMotorDirections(false, false);
        // } else if ((RESULT_ADC_LEFT < THRESHOLD) && (true)) {
        //     gpio_put(LEFT_MOTOR_PIN_A, 1);
        //     gpio_put(LEFT_MOTOR_PIN_B, 0);
        //     gpio_put(RIGHT_MOTOR_PIN_A, 0);
        //     gpio_put(RIGHT_MOTOR_PIN_B, 1);
            // turnMotorDirections(true, true);
        // } else if (userInput == '4') {
        //     gpio_put(LEFT_MOTOR_PIN_A, 0);
        //     gpio_put(LEFT_MOTOR_PIN_B, 1);
        //     gpio_put(RIGHT_MOTOR_PIN_A, 1);
        //     gpio_put(RIGHT_MOTOR_PIN_B, 0);
            // turnMotorDirections(false, false);
        } else if ((RESULT_ADC_LEFT >= THRESHOLD) && (RESULT_ADC_RIGHT >= THRESHOLD)) {
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 0);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 0);
            // calculate_distance();
        // } else if (userInput == '6') {
        //     pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/4);
        //     pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/4);

        // } else if (userInput == '7') {
        //     pwm_set_chan_level(slice_num, PWM_CHAN_A, 25000/2);
        //     pwm_set_chan_level(slice_num, PWM_CHAN_B, 25000/2);

        } else {
            printf("Re-enter input!\n");
        };
    };
}
