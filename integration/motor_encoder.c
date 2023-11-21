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

#define DIST_GAUGE 1.0210176

#define TRIGGER_PIN 21
#define ECHO_PIN 20

int timeout = 38000;

static char event_str[128];

volatile uint32_t elapsed_time = 0;
uint32_t start_time_right;
uint32_t start_time_left;
int rise_counter = 0;

const uint16_t THRESHOLD = 1000; // ADC threshold value for black vs white
const uint8_t PERIOD = 100; // Set timer period to 100 us
volatile uint16_t RESULT_ADC_LEFT;
volatile uint16_t RESULT_ADC_RIGHT;
struct repeating_timer timer;
bool left_ir = false;
bool right_ir = false;
bool reverse = false;

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
    gpio_put(RIGHT_MOTOR_PIN_A, !right_forward);
    gpio_put(RIGHT_MOTOR_PIN_B, right_forward);
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

    if (gpio == 27) {
        calculate_right();
        start_time_right = time_us_32();
    }

    if (gpio == 26) {
        calculate_left();
        start_time_left = time_us_32();
    }
}

uint16_t read_adc_channel(uint channel) {
    adc_select_input(channel);
    return adc_read();
}

uint64_t get_cm(uint trigger_pin, uint echo_pin) {
    gpio_put(trigger_pin, 1);
    busy_wait_us(10);
    gpio_put(trigger_pin, 0);

    uint64_t width = 0;
    uint64_t max_width = timeout;

    // Wait for the echo signal to go HIGH with a timeout
    while (gpio_get(echo_pin) == 0 && max_width > 0) {
        busy_wait_us(1);  // Use busy_wait_us for non-blocking wait
        max_width--;
    }

    if (max_width <= 0) {
        return 0;  // Return 0 if the timeout is reached
    }

    absolute_time_t startTime = get_absolute_time();
    max_width = timeout;

    // Measure the pulse width of the echo signal
    while (gpio_get(echo_pin) == 1 && max_width > 0) {
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

bool read_ir_sensor() {
    RESULT_ADC_LEFT = read_adc_channel(ADC_PIN_LEFT);
    RESULT_ADC_RIGHT = read_adc_channel(ADC_PIN_RIGHT);

    uint64_t distance_cm = get_cm(TRIGGER_PIN, ECHO_PIN);
    // printf("Distance in cm: %llu\n", distance_cm);

    if (distance_cm < 10) {
        // printf("Distance is less than 10cm.\n");
        // setMotorDirections(0,0);
        reverse = true;
        // sleep_ms(5000);
        // reverse = false; 
    }

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

void setup_ultrasonic_pins(uint trigger_pin, uint echo_pin) {
    gpio_init(trigger_pin);
    gpio_init(echo_pin);
    gpio_set_dir(trigger_pin, GPIO_OUT);
    gpio_set_dir(echo_pin, GPIO_IN);
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
    setup_ultrasonic_pins(TRIGGER_PIN, ECHO_PIN);
    adc_init();
    adc_select_input(26);
    adc_select_input(27);

    add_repeating_timer_us(PERIOD, read_ir_sensor, NULL, &timer);

    //GP22
    // gpio_init(IRPOWER_LEFT);
    // gpio_set_dir(IRPOWER_LEFT, GPIO_OUT);
    // gpio_put(IRPOWER_LEFT, 1);

    // //GP21
    // gpio_init(IRPOWER_RIGHT);
    // gpio_set_dir(IRPOWER_RIGHT, GPIO_OUT);
    // gpio_put(IRPOWER_RIGHT, 1);
    
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

    // gpio_init(19);
    // gpio_init(20);
    // gpio_set_dir(19, GPIO_OUT);
    // gpio_set_dir(20, GPIO_OUT);
    // gpio_put(19, 0);
    // gpio_put(20, 1);

    // gpio_init(17);
    // gpio_set_dir(17, GPIO_OUT);
    // gpio_put(17, 1);

    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(0, GPIO_FUNC_PWM);
    gpio_set_function(1, GPIO_FUNC_PWM);

    // Wheel Encoder
    // gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    // gpio_set_irq_enabled(3, GPIO_IRQ_EDGE_RISE, true);

    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_clkdiv(slice_num, 100);
    // Set period of 4 cycles (0 to 3 inclusive)
    pwm_set_wrap(slice_num, 12500);
    // Set the PWM running
    pwm_set_enabled(slice_num, true);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/3);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/3);

    while(1) {

        if ((RESULT_ADC_LEFT < THRESHOLD) && (RESULT_ADC_RIGHT < THRESHOLD) && (reverse == 0)) {
            pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/3);
            pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/3);
            setMotorDirections(1,1);
            // printf("moving forward\n");

        } else if ((RESULT_ADC_LEFT < THRESHOLD) && (RESULT_ADC_RIGHT >= THRESHOLD) && (reverse == 0)) {

            pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/2.5);

        } else if ((RESULT_ADC_LEFT >= THRESHOLD) && (RESULT_ADC_RIGHT < THRESHOLD) && (reverse == 0)) {

            pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/2.5);

        } else if ((RESULT_ADC_LEFT < THRESHOLD) && (RESULT_ADC_RIGHT < THRESHOLD) && (reverse == 1)) {
            // printf("its here");
            // ((RESULT_ADC_LEFT >= THRESHOLD) && (RESULT_ADC_RIGHT >= THRESHOLD)) ||
            // printf("motor stopped\n");
            pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/3);
            pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/3);
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 0);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 0);
            sleep_ms(2000);
            setMotorDirections(0,0);
            sleep_ms(1200);
            reverse = false;
            // printf("reversing");

        } else if ((RESULT_ADC_LEFT >= THRESHOLD) && (RESULT_ADC_RIGHT >= THRESHOLD) && (reverse == 0)) {
            pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/2);
            pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/2);
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 0);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 0);
            sleep_ms(2000);
            turnMotorDirections(0,0);
            sleep_ms(500);
            // printf("turning left\n");
        } else {
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 0);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 0);
        };
    };
}
