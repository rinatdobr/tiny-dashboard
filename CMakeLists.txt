cmake_minimum_required(VERSION 3.10)

project(tiny-dashboard)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Core Qml Quick REQUIRED)

find_package(X11 REQUIRED)

set(X11_BACKEND_SOURCES
    src/backend/x11/backend.cpp
    src/backend/x11/window.cpp
    src/backend/x11/window_info.cpp
)

set(UI_SOURCES
    src/ui/centre.cpp
)

set(SOURCES
    src/main.cpp
)

qt5_wrap_cpp(SOURCES
TARGET tiny-dashboard)

qt5_add_resources(SOURCES src/ui/resources.qrc)

set(SPDLOG_DIR third-party/spdlog)
add_subdirectory(${SPDLOG_DIR} spdlog)

add_executable(tiny-dashboard
    ${X11_BACKEND_SOURCES}
    ${UI_SOURCES}
    ${SOURCES}
)

target_link_libraries(tiny-dashboard ${X11_LIBRARIES} ${X11_Xi_LIB} pthread Qt5::Core Qt5::Qml Qt5::Quick)
target_include_directories(tiny-dashboard PRIVATE
    ${X11_INCLUDE_DIR}
    src
    src/ui
    src/backend
    ${SPDLOG_DIR}/include
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
)

install(FILES src/ui/center.qml
        DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(TARGETS tiny-dashboard DESTINATION ${CMAKE_INSTALL_BINDIR})
