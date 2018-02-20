cmake_minimum_required(VERSION 2.8.4)
set(ARDUINO_SDK_PATH ${HOME}/Programming/IDEs/arduino-1.8.2/) # TODO: ENVVAR
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/ArduinoToolchain.cmake)
set(PROJECT_NAME TooAutomation)
project(${PROJECT_NAME})

#add_library(./TooAutomation.h)
add_library(TooSigning.h ./modules/TooSigning/TooSigning.cpp)
add_library(TooNetworking.h ./modules/TooNetworking/TooNetworking.cpp)

set(NODE_SKETCH ./examples/TooAutomation_Node/TooAutomation_Node.ino)
set(MASTER_SKETCH ./examples/TooAutomation_Master/TooAutomation_Master.ino)

#### Uncomment below additional settings as needed.
# set(${CMAKE_PROJECT_NAME}_BOARD mega)
# set(${CMAKE_PROJECT_NAME}_PORT /dev/ttyACM0)
# set(mega.build.mcu atmega2560)
# set(mega.upload.protocol wiring)
# set(mega.upload.speed 115200)

generate_arduino_firmware(NODE)
generate_arduino_firmware(MASTER)
