# Tests/CMakeLists.txt
find_package(GTest REQUIRED)

enable_testing()

add_executable(rufuspp_tests
    test_usb_detector.cpp
    test_format.cpp
    ../src/usb_detector.cpp
    ../src/storage/format.cpp
)

target_include_directories(rufuspp_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${UDEV_INCLUDE_DIRS}
)

target_link_libraries(rufuspp_tests
    GTest::gtest_main
    ${UDEV_LIBRARIES}
)

include(GoogleTest)
gtest_discover_tests(rufuspp_tests)
