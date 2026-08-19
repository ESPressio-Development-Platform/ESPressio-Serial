cmake_minimum_required(VERSION 3.16)
project(ESPressioSerialTests LANGUAGES CXX)

enable_testing()

add_executable(
    test_console
    test_console.cpp
)

target_compile_features(
    test_console
    PRIVATE
    cxx_std_17
)

target_compile_options(
    test_console
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

target_include_directories(
    test_console
    PRIVATE
    stubs
    ../src
)

add_test(
    NAME Console
    COMMAND test_console
)


add_executable(
    test_event_console
    test_event_console.cpp
)

target_compile_features(
    test_event_console
    PRIVATE
    cxx_std_17
)

target_compile_options(
    test_event_console
    PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

target_include_directories(
    test_event_console
    PRIVATE
    stubs
    ../src
)

add_test(
    NAME EventConsoleContract
    COMMAND test_event_console
)
