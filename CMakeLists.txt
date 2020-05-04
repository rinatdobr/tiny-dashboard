cmake_minimum_required(VERSION 3.10)

project(tiny-dashboard)

set(CMAKE_CXX_STANDARD 11)

find_package(X11 REQUIRED)

set(SOURCES
    src/main.cpp
)

set(SPDLOG_DIR third-party/spdlog)
add_subdirectory(${SPDLOG_DIR} spdlog)

add_executable(tiny-dashboard
    ${SOURCES}
)

target_link_libraries(tiny-dashboard ${X11_LIBRARIES} pthread)
target_include_directories(tiny-dashboard PRIVATE
    ${X11_INCLUDE_DIR}
    ${SPDLOG_DIR}/include
)