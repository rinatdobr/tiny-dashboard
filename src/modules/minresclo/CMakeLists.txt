cmake_minimum_required(VERSION 3.10)

project(minresclo)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    module.cpp
)

set(SPDLOG_DIR ${TD_DIR}/third-party/spdlog)

add_library(minresclo SHARED
    ${SOURCES}
)

target_include_directories(minresclo PRIVATE
    ${SPDLOG_DIR}/include
    ${TD_DIR}/include
    ${TD_DIR}/src
)

install(TARGETS minresclo DESTINATION ${CMAKE_INSTALL_LIBDIR})
