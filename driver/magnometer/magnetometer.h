
typedef struct
{
    int16_t raw_x_axis;
    int16_t raw_y_axis;
    int16_t raw_z_axis;
} accel_t;


void magnetometer_task(__unused void *params);
void magnetometer_init(void);
void accelerometer_init(void);
accel_t accelerometer_read(void);
void compass_init(void);
float compass_read_degrees(void);