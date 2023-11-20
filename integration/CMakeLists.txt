add_executable(motor_encoder
        motor_encoder.c
        )

# pull in common dependencies and additional pwm hardware support
target_link_libraries(motor_encoder pico_stdlib hardware_pwm hardware_adc)

# create map/bin/hex file etc.
pico_add_extra_outputs(motor_encoder)
pico_enable_stdio_usb(motor_encoder 1)

# add url via pico_set_program_url
example_auto_set_url(motor_encoder)
