cmake_minimum_required(VERSION 2.4.7)
PROJECT(ffmvccd CXX C)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")
set(RULES_INSTALL_DIR "/etc/udev/rules.d")

find_package(INDI REQUIRED)
find_package(ZLIB REQUIRED)

#configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})
include_directories( ${INDI_INCLUDE_DIR})

########### Arduino Astro Control Driver ###############

set(aac_SRCS
    ${CMAKE_SOURCE_DIR}/aacdriver.cpp
    ${CMAKE_SOURCE_DIR}/aacusb.cpp
    )

add_executable(indi_aac ${aac_SRCS})

target_link_libraries(indi_aac ${INDI_DRIVER_LIBRARIES})
install(TARGETS indi_aac RUNTIME DESTINATION bin )
