cmake_minimum_required(VERSION 3.10)
project(RufusPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find udev
find_package(PkgConfig REQUIRED)
pkg_check_modules(UDEV REQUIRED libudev)

# Main executable
add_executable(rufuspp 
    src/main.cpp
    src/usb_detector.cpp
    src/storage/format.cpp
)

# Project structure
target_include_directories(rufuspp PRIVATE 
    src 
    ${UDEV_INCLUDE_DIRS}
    thirdparty/bit7z/include
)

link_directories(${UDEV_LIBRARY_DIRS})
add_definitions(${UDEV_CFLAGS_OTHER})

target_link_libraries(rufuspp PRIVATE ${UDEV_LIBRARIES})


# Tests
add_subdirectory(tests)
