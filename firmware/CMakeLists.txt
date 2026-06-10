cmake_minimum_required(VERSION 3.20.0)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/local.conf")
    list(APPEND EXTRA_CONF_FILE "${CMAKE_CURRENT_SOURCE_DIR}/local.conf")
endif()

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(edge_environment)


zephyr_include_directories(include)

target_sources(app PRIVATE 
    src/main.cpp
    src/sensor.cpp
    src/filter.cpp
    src/wifi_connect.cpp
    src/mqtt_publisher.cpp
)
