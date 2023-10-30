add_executable(magnetometer
magnetometer.c magnetometer.h
        )

# pull in common dependencies and additional pwm hardware support
target_link_libraries(magnetometer pico_stdlib hardware_pwm hardware_i2c)

# create map/bin/hex file etc.
pico_add_extra_outputs(magnetometer)
pico_enable_stdio_usb(magnetometer 1)
pico_enable_stdio_uart(magnetometer 0)

# add url via pico_set_program_url
example_auto_set_url(magnetometer)