#add_library(pico_ultrasonic ultrasonic.h ultrasonic.c)
add_executable(pico_ultrasonic ultrasonic.c ultrasonic.h)

target_link_libraries(pico_ultrasonic pico_stdlib hardware_gpio hardware_timer hardware_adc hardware_pwm)

target_include_directories(pico_ultrasonic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# create map/bin/hex file etc.
pico_enable_stdio_usb(pico_ultrasonic 1)
pico_add_extra_outputs(pico_ultrasonic)
