cmake_minimum_required(VERSION 3.20)
project(ullog
        VERSION 0.1.0
        DESCRIPTION "Tiny streaming logger for embedded C++"
        LANGUAGES CXX)

option(ULLOG_BUILD_TESTS "Build ullog tests" OFF)

add_library(ullog STATIC
        src/converter_to_cstring.cpp
        src/logger.cpp
        src/record.cpp
)
add_library(ullog::ullog ALIAS ullog)

target_include_directories(ullog PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(ullog PUBLIC cxx_std_17)

if(ULLOG_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()