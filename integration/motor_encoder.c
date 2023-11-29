#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "lwip/apps/httpd.h"
#include "pico/cyw43_arch.h"
#include "ssi.h"
#include "cgi.h"
#include "lwipopts.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

//for motor
#define LEFT_MOTOR_PIN_A 15
#define LEFT_MOTOR_PIN_B 14
#define RIGHT_MOTOR_PIN_A 11
#define RIGHT_MOTOR_PIN_B 10
#define TIME_INTERVAL_MS 1
#define DIST_GAUGE 1.0210176

//for IR sensor
#define ADC_PIN_LEFT 0
#define ADC_PIN_RIGHT 1


//for ultrasonic sensor
#define TRIGGER_PIN 21
#define ECHO_PIN 20

int timeout = 38000;

static char event_str[128];

volatile uint32_t elapsed_time = 0;
uint32_t start_time_right;
uint32_t start_time_left;
int rise_counter = 0; //calculates how many edge rises to calculate distance

const uint16_t THRESHOLD = 500; // ADC threshold value for black vs white
const uint8_t PERIOD = 100; // Set timer period to 100 us
volatile uint16_t RESULT_ADC_LEFT;
volatile uint16_t RESULT_ADC_RIGHT;
struct repeating_timer timer;
bool reverse = false; //if reverse == true, means wall is detected on both ir sensors

//wifi settings for FreeRTOS
const char WIFI_SSID[] = "KK-iPhone";
const char WIFI_PASSWORD[] = "Password";

//sets both motor to go forward or backwards
void setMotorDirections(bool left_forward, bool right_forward) {
    gpio_put(LEFT_MOTOR_PIN_A, !left_forward);
    gpio_put(LEFT_MOTOR_PIN_B, left_forward);
    gpio_put(RIGHT_MOTOR_PIN_A, !right_forward);
    gpio_put(RIGHT_MOTOR_PIN_B, right_forward);
}

//does the turning for the motor
void turnMotorDirections(bool left_forward, bool right_forward) {
    gpio_put(LEFT_MOTOR_PIN_A, left_forward);
    gpio_put(LEFT_MOTOR_PIN_B, !left_forward);
    gpio_put(RIGHT_MOTOR_PIN_A, !right_forward);
    gpio_put(RIGHT_MOTOR_PIN_B, right_forward);
}

static const char *gpio_irq_str[] = {
        "LEVEL_LOW",  // 0x1
        "LEVEL_HIGH", // 0x2
        "EDGE_FALL",  // 0x4
        "EDGE_RISE"   // 0x8
};

//calculates left wheel encoder speed
void calculate_left() {

    float current_time = time_us_32();

    float elapsed_time_left = current_time - start_time_left;
    float speed_left = DIST_GAUGE / (elapsed_time_left/1000000);

    printf("Current left wheel speed is %f cm/s \n", speed_left);
}

//calculates right wheel encoder speed
void calculate_right() {
    
    float current_time = time_us_32();

    float elapsed_time_right = current_time - start_time_right;
    float speed_right = DIST_GAUGE / (elapsed_time_right/1000000);

    printf("Current right wheel speed is %f cm/s \n", speed_right);
}

//calculates total distance after end of maze. not implemented in the partial integration
void calculate_distance() {
    float total_distance = rise_counter * DIST_GAUGE;

    printf("Total distance travelled %0.2f cm \n", total_distance);
}

void gpio_callback(uint gpio, uint32_t events) {

    rise_counter = rise_counter + 1;

    //calculates speed for right wheel encoder
    if (gpio == 27) {
        calculate_right();
        start_time_right = time_us_32();
    }

    //calculates speed for left wheel encoder
    if (gpio == 26) {
        calculate_left();
        start_time_left = time_us_32();
    }
}

//function to read adc value to get threshold value for IR sensor
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

    //Calls get_cm function to get the distance
    uint64_t distance_cm = get_cm(TRIGGER_PIN, ECHO_PIN);

    //If distance is less than 10cm
    if (distance_cm < 10) {
        reverse = true;
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

int main() {

    stdio_init_all();
    setup_ultrasonic_pins(TRIGGER_PIN, ECHO_PIN);
    adc_init();
    cyw43_arch_init();
    cyw43_arch_enable_sta_mode();
    adc_select_input(26);
    adc_select_input(27);

    add_repeating_timer_us(PERIOD, read_ir_sensor, NULL, &timer);
    // Connect to the WiFI network - loop until connected
    while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000) != 0) {
        printf("Attempting to connect...\n");
    }
    // Print a success message once connected
    printf("Connected!\n");
    // Initialise web server
    httpd_init();
    printf("Http server initialised\n");
    // Configure SSI and CGI handler
    ssi_init();
    printf("SSI Handler initialised\n");
    cgi_init();
    printf("CGI Handler initialised\n");

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

    // Wheel Encoder
    gpio_set_irq_enabled_with_callback(2, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled(3, GPIO_IRQ_EDGE_RISE, true);

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

        if ((RESULT_ADC_LEFT < THRESHOLD) && (RESULT_ADC_RIGHT < THRESHOLD) && (reverse == 0)) {

            //motor goes forward
            pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/2);
            pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/2);
            setMotorDirections(1,1);

        } else if ((RESULT_ADC_LEFT < THRESHOLD) && (RESULT_ADC_RIGHT >= THRESHOLD) && (reverse == 0)) {

            //if right IR detects wall, speeds right motor to go back to the middle
            pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/3);

        } else if ((RESULT_ADC_LEFT >= THRESHOLD) && (RESULT_ADC_RIGHT < THRESHOLD) && (reverse == 0)) {

            //if left IR detects wall, speeds left motor to go back to the middle
            pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/3);

        } else if ((RESULT_ADC_LEFT >= THRESHOLD) && (RESULT_ADC_RIGHT >= THRESHOLD) && (reverse == 0)) {

            //detects black on both IR sensors, stops and is set to turn left
            pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/2);
            pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/2);
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 0);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 0);
            sleep_ms(2000);
            turnMotorDirections(0,0);
            sleep_ms(650);
        } else if ((RESULT_ADC_LEFT < THRESHOLD) && (RESULT_ADC_RIGHT < THRESHOLD) && (reverse == 1)) {
            
            //detects ultrasonic and reverses for partial integration
            pwm_set_chan_level(slice_num, PWM_CHAN_A, 12500/2);
            pwm_set_chan_level(slice_num, PWM_CHAN_B, 12500/2);
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 0);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 0);
            sleep_ms(2000);
            setMotorDirections(0,0);
            sleep_ms(1000);
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 0);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 0);
            reverse = false;
        } else {

            //does nothing if the above is not met
            gpio_put(LEFT_MOTOR_PIN_A, 0);
            gpio_put(LEFT_MOTOR_PIN_B, 0);
            gpio_put(RIGHT_MOTOR_PIN_A, 0);
            gpio_put(RIGHT_MOTOR_PIN_B, 0);
        };
    };
}
