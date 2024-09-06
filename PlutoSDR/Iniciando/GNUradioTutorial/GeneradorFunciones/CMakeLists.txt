# GNU Radio C++ Flow Graph CMakeLists.txt
#
# Title: Not titled yet
# Author: andresflores
# GNU Radio version: v3.8.5.0-6-g57bd109d


cmake_minimum_required(VERSION 3.8)

find_package(Qt5Widgets REQUIRED)

include_directories(
    ${GNURADIO_ALL_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDES}
    $ENV{HOME}/.grc_gnuradio
)

add_definitions(${Qt5Widgets_DEFINITIONS})

set(CMAKE_AUTOMOC TRUE)



set(GR_LIBRARIES
    boost_system
    gnuradio-blocks
    gnuradio-runtime
    gnuradio-pmt
    log4cpp
    gnuradio-analog
    gnuradio-qtgui
    Qt5::Widgets
)

add_executable(GeneradorFunciones GeneradorFunciones.cpp)
target_link_libraries(GeneradorFunciones ${GR_LIBRARIES})
