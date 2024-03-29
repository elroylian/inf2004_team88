# inf2004_team88

## Car Flowchart

![Car Flowchart](Car_Flowchart.jpg)

## Raspberry Pi Pico W Micromouse Project
### Overview
This project is focused on creating an embedded C language micromouse using the Raspberry Pi Pico W. The micromouse is designed to navigate through mazes autonomously, leveraging the capabilities of the Raspberry Pi Pico W.

### Driver Components
#### IR Sensor
Detect black lines and also barcode
#### Magnetometer
Used as a compass to align the micromouse
#### Motor
Motor encoder for speed and turning control
#### Ultrasonic 
Detection of obstacles 
#### Wifi
Runs a server on the Pico W and communicates via the WiFi module

### Requirements
Raspberry Pi Pico W
CMake for building the project
Basic understanding of embedded C programming
### Building the Project
Setup CMake: Ensure that CMake is installed on your system.
Clone the Repository: Clone this repository to your local machine.
Build the Project:
Run CMake to build the project under /integration.
A .uf2 file will be generated as the build output.
### Flashing to Raspberry Pi Pico W
Connect your Raspberry Pi Pico W to your computer.
Hold the BOOTSEL button and plug your Pico into the USB port of your computer.
Release the BOOTSEL button after your Pico is connected. It will mount as a Mass Storage Device.
Drag and drop the generated .uf2 file into the Raspberry Pi Pico W.
### Usage
Once flashed, the Raspberry Pi Pico W will run the micromouse program. Further details on operation and configuration will be added as the project develops.

