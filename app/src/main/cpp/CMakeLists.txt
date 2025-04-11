cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0091 NEW)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(aap-airwindows LANGUAGES CXX)

set(AW2R_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../../../external/airwin2rack)
set(CMIDI2_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../../external/cmidi2)
get_filename_component(CMIDI2_INCLUDE_DIR ${CMIDI2_SOURCE_DIR} ABSOLUTE)

file(GLOB aap-airwindows_SOURCES_GENSRC
        ${AW2R_ROOT}/src/autogen_airwin/*.cpp
)
set(aap-airwindows_SOURCES
        ${AW2R_ROOT}/src/AirwinRegistry.cpp
        ${AW2R_ROOT}/src/airwin_consolidated_base.cpp
        aap-airwindows.cpp
        stub.cpp
)


add_library (aap-airwindows
        SHARED
        ${aap-airwindows_SOURCES}
        ${aap-airwindows_SOURCES_GENSRC}
)

find_package(androidaudioplugin CONFIG REQUIRED)

target_compile_options (aap-airwindows
        PRIVATE
        -std=c++17 -Wall -Wshadow
)

target_include_directories(aap-airwindows PRIVATE
        .
        ${AW2R_ROOT}/src
        ${CMIDI2_INCLUDE_DIR}
)

target_link_libraries (aap-airwindows
        android
        log
        androidaudioplugin::androidaudioplugin
)
