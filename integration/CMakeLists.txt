cmake_minimum_required(VERSION 3.13)

set(PROGRAM_NAME motor_encoder)
set(PICO_BOARD pico_w)

message("Running makefsdata python script")
execute_process(COMMAND
    py makefsdata.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

add_executable(${PROGRAM_NAME}
        motor_encoder.c
        )

target_include_directories(${PROGRAM_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)        

# pull in common dependencies and additional pwm hardware support
target_link_libraries(${PROGRAM_NAME} 
        pico_stdlib 
        hardware_pwm 
        hardware_adc 
        pico_lwip_http 
        pico_cyw43_arch_lwip_threadsafe_background
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${PROGRAM_NAME})
pico_enable_stdio_usb(${PROGRAM_NAME} 1)
pico_enable_stdio_uart(${PROGRAM_NAME} 0)


# add url via pico_set_program_url
example_auto_set_url(${PROGRAM_NAME})
