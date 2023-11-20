#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <string.h>

char array[15];
void enqueue(char);
int counter = 0;
void checkarray();
#define IR_SENSOR_PIN 26 // GPIO pin for the IR line sensor
bool started = false;
volatile uint32_t pulse_start = 0;
volatile bool is_black_bar = false;
volatile uint32_t pulse_width = 0;
uint32_t shortbar_time = 0;

bool flag = false;

void on_pulse_change(uint gpio, uint32_t events)
{
    uint32_t current_time = time_us_32();

    if (gpio_get(IR_SENSOR_PIN))
    {

        if (started == true)
        {
            // Transition from black to white; a barcode element has ended
            pulse_width = current_time - pulse_start;
            pulse_start = current_time;

            if (pulse_width > shortbar_time * 1.3)
            {
                // It's a long white bar
                enqueue('0');
                enqueue('0');
                enqueue('0');
            }
            else
            {
                // It's a short white bar
                enqueue('0');
            }
        }
        else
        {
            pulse_start = current_time;
        }
        started = true;
    }
    else
    {
        if (started == true)
        {

            pulse_width = current_time - pulse_start;
            pulse_start = current_time;
            if (!shortbar_time)
            {
                shortbar_time = pulse_width;
                printf("Short bar = %d \n", shortbar_time);
            }
            if (pulse_width > shortbar_time * 1.3)
            {
                // It's a long black bar
                enqueue('1');
                enqueue('1');
                enqueue('1');
            }
            else
            {
                // It's a short black bar
                enqueue('1');
            }
        }
    }
}


//See whether it matches a character
void checkarray()
{
    if (strcmp(array, "100010111011101") == 0)
    {
        printf("*\n");
    }
    else if (strcmp(array, "101000111011101") == 0)
    {
        printf("0\n");
    }
    else if (strcmp(array, "111010001010111") == 0)
    {
        printf("1\n");
    }
    else if (strcmp(array, "101110001010111") == 0)
    {
        printf("2\n");
    }
    else if (strcmp(array, "111011100010101") == 0)
    {
        printf("3\n");
    }
    else if (strcmp(array, "101000111010111") == 0)
    {
        printf("4\n");
    }
    else if (strcmp(array, "111010001110101") == 0)
    {
        printf("5\n");
    }

    else if (strcmp(array, "101110001110101") == 0)
    {
        printf("6\n");
    }
    else if (strcmp(array, "101000101110111") == 0)
    {
        printf("7\n");
    }
    else if (strcmp(array, "111010001011101") == 0)
    {
        printf("8\n");
    }
    else if (strcmp(array, "101110001011101") == 0)
    {
        printf("9\n");
    }
    else if (strcmp(array, "111010100010111") == 0)
    {
        printf("A\n");
    }
    else if (strcmp(array, "101110100010111") == 0)
    {
        printf("B\n");
    }
    else if (strcmp(array, "111011101000101") == 0)
    {
        printf("C\n");
    }
    else if (strcmp(array, "101011100010111") == 0)
    {
        printf("D\n");
    }
    else if (strcmp(array, "111010111000101") == 0)
    {
        printf("E\n");
    }
    else if (strcmp(array, "101110111000101") == 0)
    {
        printf("F\n");
    }
    else if (strcmp(array, "101010001110111") == 0)
    {
        printf("G\n");
    }
    else if (strcmp(array, "111010100011101") == 0)
    {
        printf("H\n");
    }
    else if (strcmp(array, "101110100011101") == 0)
    {
        printf("I\n");
    }
    else if (strcmp(array, "101011100011101") == 0)
    {
        printf("J\n");
    }
    else if (strcmp(array, "111010101000111") == 0)
    {
        printf("K\n");
    }
    else if (strcmp(array, "101110101000111") == 0)
    {
        printf("L\n");
    }
    else if (strcmp(array, "111011101010001") == 0)
    {
        printf("M\n");
    }
    else if (strcmp(array, "101011101000111") == 0)
    {
        printf("N\n");
    }
    else if (strcmp(array, "111010111010001") == 0)
    {
        printf("O\n");
    }
    else if (strcmp(array, "101110111010001") == 0)
    {
        printf("P\n");
    }
    else if (strcmp(array, "101010111000111") == 0)
    {
        printf("Q\n");
    }
    else if (strcmp(array, "111010101110001") == 0)
    {
        printf("R\n");
    }
    else if (strcmp(array, "101110101110001") == 0)
    {
        printf("S\n");
    }
    else if (strcmp(array, "101011101110001") == 0)
    {
        printf("T\n");
    }
    else if (strcmp(array, "111000101010111") == 0)
    {
        printf("U\n");
    }
    else if (strcmp(array, "100011101010111") == 0)
    {
        printf("V\n");
    }
    else if (strcmp(array, "111000111010101") == 0)
    {
        printf("W\n");
    }
    else if (strcmp(array, "100010111010111") == 0)
    {
        printf("X\n");
    }
    else if (strcmp(array, "111000101110101") == 0)
    {
        printf("Y\n");
    }
    else if (strcmp(array, "100011101110101") == 0)
    {
        printf("Z\n");
    }

    flag = true;
}

//Flip array to read it backwards
void flipArray(char arr[], int size)
{
    int start = 0;
    int end = size - 1;

    while (start < end)
    {
        // Swap elements at start and end indices
        char temp = arr[start];
        arr[start] = arr[end];
        arr[end] = temp;

        // Move indices towards the center
        start++;
        end--;
    }
}

void enqueue(char element)
{
    if (flag == false)
    {
        array[counter] = element;
        counter++;
        if (counter == 15)
        {
            counter = 0;
            checkarray();
            flipArray(array, 15);
            checkarray();
            flipArray(array, 15);
        }
    }
    else
    {
        flag = false;
    }
}


int main()
{

    stdio_init_all();
    gpio_init(IR_SENSOR_PIN);
    gpio_set_dir(IR_SENSOR_PIN, GPIO_IN);

    gpio_set_irq_enabled_with_callback(IR_SENSOR_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &on_pulse_change);

    while (1)
    {
        sleep_ms(10);
    }

    return 0;
}
