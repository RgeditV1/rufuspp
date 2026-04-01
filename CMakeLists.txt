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
    src/iso.cpp
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

# Definir la ruta de las librerías de bit7z
set(BIT7Z_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/bit7z/lib/x64)

target_link_libraries(rufuspp PRIVATE 
    ${UDEV_LIBRARIES}
    debug ${BIT7Z_LIB_DIR}/libbit7z64_d.a
    optimized ${BIT7Z_LIB_DIR}/libbit7z64.a
)



# Tests
add_subdirectory(tests)
