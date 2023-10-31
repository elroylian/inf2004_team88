#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/timer.h"

const uint8_t ADC_PIN = 0; // Use ADC 0
const uint16_t THRESHOLD = 1000; // ADC threshold value for black vs white
const uint8_t PERIOD = 100; // Set timer period to 100 us
struct repeating_timer timer;

bool adc_callback(struct repeating_timer *t) {
    // Read adc value and print black or white
    uint16_t result = adc_read();
    
    if (result >= THRESHOLD) {
        // Above or equal threshold
        printf("%u, BLACK DETECTED\n", result);
    } else {           
        // Below threshold
        printf("%u, WHITE DETECTED\n", result);
    }
    return true;
}

int main() {
    stdio_init_all();
    // Initalise ADC 
    adc_init();
    adc_select_input(ADC_PIN);

    // Add a timer to trigger the ADC callback every 100us
    add_repeating_timer_us(PERIOD, adc_callback, NULL, &timer);
    while (1);
    return 0;
}
