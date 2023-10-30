#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "magnetometer.h"

// Define the constants used for setting up I2C and the sensor addresses
#define I2C_PORT                i2c0
#define I2C_DATA_RATE           400000  // Data rate in Hz
#define MAGNETOMETER_SCL_PIN    1       // SCL pin number   
#define MAGNETOMETER_SDA_PIN    0       // SDA pin number
#define PI                      3.14159265358979323846

// Sensor I2C addresses
#define ACCELEROMETER_ADDRESS   0x19    // 0011001b
#define MAGNETOMETER_ADDRESS    0x1E    // 0011110b

/**
 * @brief Main task to read data from the magnetometer and accelerometer.
 * 
 * @param params Unused parameter for task compatibility.
*/

void magnetometer_task(__unused void *params)
{
    // Initialise the magnetometer once at the start
    magnetometer_init();

    // Main loop to continuously read the accelerometer and magnetometer data
    while (true)
    {
        accel_t acceleration = accelerometer_read();
        float compass_heading = compass_read_degrees();

        printf("accel x: %d accel y: %d accel z: %d compass: %.2f\n", acceleration.raw_x_axis, acceleration.raw_y_axis, acceleration.raw_z_axis, compass_heading);
    }
}

/**
 * @brief Initialise the I2C communication with the magnetometer and accelerometer.
*/

void magnetometer_init(void)
{
    // Initialise I2C with the data rate
    i2c_init(I2C_PORT,I2C_DATA_RATE);

    // Set up the SCL and SDA pins for I2C communication
    gpio_set_function(MAGNETOMETER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_set_function(MAGNETOMETER_SDA_PIN, GPIO_FUNC_I2C);

    // Enable the internal pull-ups for the I2C pins
    gpio_pull_up(MAGNETOMETER_SCL_PIN);
    gpio_pull_up(MAGNETOMETER_SDA_PIN);

    // Initialise the accelerometer and compass modules
    accelerometer_init();
    compass_init();
}

/**
 * @brief Initialise the acclerometer by setting up the control registers.
*/

void accelerometer_init(void)
{
    // Control register addresses and values for the accelerometer setup
    const uint8_t CTRL_REG1_A = 0x20;
    const uint8_t ENABLE_ACCEL = 0x57;  // 100 Hz data rate, all axes enabled, normal mode
    const uint8_t CTRL_REG4_A = 0x23;
    const uint8_t FULL_SCALE = 0x00;    // +/- 2g scale

    // Enable accelerometer by writing to the control register 1
    uint8_t config[] = {CTRL_REG1_A, ENABLE_ACCEL};
    i2c_write_blocking(I2C_PORT, ACCELEROMETER_ADDRESS, config, sizeof(config), true);

    // Set the full scale by writingto the control register 4
    config[0] = CTRL_REG4_A;
    config[1] = FULL_SCALE;
    i2c_write_blocking(I2C_PORT, ACCELEROMETER_ADDRESS, config, sizeof(config), true);
}

/**
 * @brief Read the current acceleration values from the accelerometer.
 * 
 * @return accel_t Struct containing the raw acceleration data for x, y and z axes.
*/

accel_t accelerometer_read(void)
{
    // Addresses for the accelerometer data registers with auto-increment bit set
    const uint8_t OUT_X_L_A = 0x28 | 0x80;
    uint8_t accel_data[6] = {0};            // Buffer to hold the raw accelerometer data

    // Request the accelerometer data
    i2c_write_blocking(I2C_PORT, ACCELEROMETER_ADDRESS, &OUT_X_L_A, 1, true);
    i2c_read_blocking(I2C_PORT, ACCELEROMETER_ADDRESS, accel_data, sizeof(accel_data), true);

    // Combine the high and low bytes for each axis
    int16_t raw_x_axis = (int16_t)((accel_data[1] << 8) | accel_data[0]) >> 4;
    int16_t raw_y_axis = (int16_t)((accel_data[3] << 8) | accel_data[2]) >> 4;
    int16_t raw_z_axis = (int16_t)((accel_data[5] << 8) | accel_data[4]) >> 4;

    // Return the raw acceleration data in a struct
    accel_t data = {
        .raw_x_axis = raw_x_axis,
        .raw_y_axis = raw_y_axis,
        .raw_z_axis = raw_z_axis
    };

    return data;
}

/**
 * @brief Initialise the compass by setting up control registers.
*/

void compass_init(void)
{
    // Address and values for the magnetometer control registers
    const uint8_t MR_REG_M = 0x02;
    const uint8_t CONTINUOUS_CONVERSION = 0x00; 
    const uint8_t CRA_REG_M = 0x00;
    const uint8_t DATA_RATE = 0x10;             // 15Hz data rate
    const uint8_t CRB_REG_M = 0x01;
    const uint8_t GAIN = 0x20;                  // +/- 1.3g scale

    // Enable continuous conversion mode on the magnetometer
    uint8_t config[] = {MR_REG_M, CONTINUOUS_CONVERSION};
    i2c_write_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, config, sizeof(config), true);

    // Set the data rate on the magnetometer
    config[0] = CRA_REG_M;
    config[1] = DATA_RATE;
    i2c_write_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, config, sizeof(config), true);

    // Set the gain on the magnetometer
    config[0] = CRB_REG_M;
    config[1] = GAIN;
    i2c_write_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, config, sizeof(config), true);
}

/**
 * @brief Read the current heading from the magnetometer and convert it to degress
 * 
 * @return float The current compass heading in degrees
*/

float compass_read_degrees(void)
{
    uint8_t reg[1] = {0};
    uint8_t data[1] = {0};

    // Read 6 bytes of data
    // msb first
    // Read xMag msb data from register(0x03)
    reg[0] = 0x03;
    i2c_write_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, data, sizeof(data), true);
    uint8_t data1_0 = data[0];

    // Read xMag lsb data from register(0x04)
    reg[0] = 0x04;
    i2c_write_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, data, sizeof(data), true);
    uint8_t data1_1 = data[0];

    // Read yMag msb data from register(0x05)
    reg[0] = 0x07;
    i2c_write_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, data, sizeof(data), true);
    uint8_t data1_2 = data[0];

    // Read yMag lsb data from register(0x06)
    reg[0] = 0x08;
    i2c_write_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, data, sizeof(data), true);
    uint8_t data1_3 = data[0];

    // Read zMag msb data from register(0x07)
    reg[0] = 0x05;
    i2c_write_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, data, sizeof(data), true);
    uint8_t data1_4 = data[0];

    // Read zMag lsb data from register(0x08)
    reg[0] = 0x06;
    i2c_write_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, reg, sizeof(reg), true);
    i2c_read_blocking(I2C_PORT, MAGNETOMETER_ADDRESS, data, sizeof(data), true);
    uint8_t data1_5 = data[0];

    // Convert the data
    int16_t xMag = (data1_0 << 8) | data1_1;
   

    int16_t yMag = (data1_2 << 8) | data1_3;
  

    int16_t zMag = (data1_4 << 8) | data1_5;
  


    // Calculate the angle of the vector y,x
    float heading = (atan2(yMag, xMag) * 180.0) / PI;

    // Normalize to 0-360
    if (heading < 0)
    {
        heading += 360;
    }

    return heading;
}


int main(void)
{
    // Initialise the standard library for Raspberry Pi Pico
    stdio_init_all();

    // Start the magnetometer task
    magnetometer_task(NULL);

    return 0;
}