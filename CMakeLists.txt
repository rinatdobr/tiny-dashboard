cmake_minimum_required(VERSION 3.10)

project(tiny-dashboard)

set(CMAKE_CXX_STANDARD 17)

find_package(X11 REQUIRED)

set(X11_BACKEND_SOURCES
    src/windows-backend/x11/backend.cpp
    src/windows-backend/x11/window.cpp
    src/windows-backend/x11/window_info.cpp
)

set(UI_SOURCES
    src/ui/centre.cpp
)

set(SOURCES
    src/main.cpp
)

set(SPDLOG_DIR third-party/spdlog)
add_subdirectory(${SPDLOG_DIR} spdlog)

add_executable(tiny-dashboard
    ${X11_BACKEND_SOURCES}
    ${UI_SOURCES}
    ${SOURCES}
)

target_link_libraries(tiny-dashboard ${X11_LIBRARIES} pthread)
target_include_directories(tiny-dashboard PRIVATE
    ${X11_INCLUDE_DIR}
    src
    src/ui
    src/windows-backend
    ${SPDLOG_DIR}/include
)