# Tests/CMakeLists.txt
find_package(GTest REQUIRED)

enable_testing()

add_executable(rufuspp_tests
    test_usb_detector.cpp
    test_format.cpp
    test_iso.cpp
    ../src/usb_detector.cpp
    ../src/storage/format.cpp
    ../src/extractor/iso.cpp
)

# Definir la ruta de las librerías de bit7z
set(BIT7Z_LIB_DIR ${CMAKE_SOURCE_DIR}/thirdparty/bit7z/lib/x64)

target_include_directories(rufuspp_tests PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/bit7z/include
    ${UDEV_INCLUDE_DIRS}
)

target_link_libraries(rufuspp_tests
    GTest::gtest_main
    ${UDEV_LIBRARIES}
    debug ${BIT7Z_LIB_DIR}/libbit7z64_d.a
    optimized ${BIT7Z_LIB_DIR}/libbit7z64.a
    stdc++fs
)

# Copiar 7z.so junto al binario de tests para que find7zLib() lo encuentre en runtime
add_custom_command(TARGET rufuspp_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/thirdparty/7zip/7z.so
        $<TARGET_FILE_DIR:rufuspp_tests>/7z.so
    COMMENT "Copiando thirdparty/7zip/7z.so junto a los tests"
)

include(GoogleTest)
gtest_discover_tests(rufuspp_tests)
