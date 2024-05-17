# Flora-Green: Indoor Sound Pollution Mechanical Plant Avatar 
## Project and Scenario Description
The aim of this project is to judge the quality of the environment using three sensor mediums. These mediums are temperature, Co2 content and acoustic sound level. 

The acoustic sound level and temperature dictates whether the flower is open (sound level is low and temperature suitable) or closed (sound level is high or temperature not suitable).
Acoustic sound level will be sensed by a Ky-038 Sound Sensor connected to an analog input on the Thingy52. The GUI will display the current sound level and will be able to control the threshold that affects whether the flower is open or closed. 
Temperature will be sensed by the on the Thingy52 and displayed on the GUI. The threshold will also be controllable by the GUI. 

The vibrancy of light from the petal LEDs will signal The CO2 content within the surrounding air. This will be displayed on the GUI. 

As the Thingy52 has Bluetooth connectivity, the Thingy52 will communicate sensor data with the nRF52 board via iBeacon protocol. The nrf52 board will serialise this data and send it through UART to the PC, where most of the data processing will occur. The PC will process this data using Kalman Filtering and data fusion and the thresholds and publish a message to MQTT, most likely comprising of LED strength and Open or Close status.  As the flower does not has WiFi connectivity, the m5Core2 will receive these MQTT packets and respond with 1 digital signal PWM signal for open(180 degrees) or close(0 degrees) and 1 analog signal corresponding to an LED level, 1-10.

## Build Instructions 
Each device will need to be programmed using Zephyr and/or Arduino IDE. Please follow the zephyr RTOS installation guide and clone this repo into your west directory. 
#### nRF52840 Board 
Connect the micro-usb port to a com port. Then attach that port to your linux machine. Please execute the following commands

west build -p always -b nrf52840dk_nrf52840 Nrf52840\
west flash

#### Thingy52
Connect the debug connector to the debug out connector on the nRF52840 board (P10).Please execute the following commands

west build -p always -b thingy52_nrf52832 Thingy52\
west flash

#### Mechanical Flower
Connect the microusb to your computer and open the code within 'Mechanical_Flower" in arduino IDE. Upload it to the playground express board. 

#### m5Core2
Connect the m5core2 to your computer and open 'm5Core2' within arduino IDE. Upload it to the device. 

## Wiring Diagrams
Below is a diagram of the wiring between the thingy52 and the sound sensor.

<img width="218" alt="image" src="https://github.com/Shreya-Personal/Flora-Green/assets/141000874/d1dc18f2-2598-4e94-8dd2-a3ce2b6ae773">

Below is a wiring diagram of the wiring between the mechanical flower and the m5Core2. 

<img width="245" alt="image" src="https://github.com/Shreya-Personal/Flora-Green/assets/141000874/4f9bb840-0958-4ad1-975f-d2c07c632de5">


## Operation
Open repo within your WSL workspace. Enter the following command. 

python PC/main.py 

From there, a GUI will pop up which can be used to control the sensitivity thresholds for what is classified as a 'bad' environment. 
