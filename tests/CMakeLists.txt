cmake_minimum_required(VERSION 3.16)
project(ESPressioSerialTests LANGUAGES CXX)

enable_testing()

set(ESPRESSIO_COMMAND_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio_Command.hpp")
set(ESPRESSIO_OBSERVABLE_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio_Observable.hpp")

add_executable(
    test_console
    test_console.cpp
)

target_compile_features(test_console PRIVATE cxx_std_17)
target_compile_options(test_console PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_include_directories(test_console PRIVATE stubs ../src)
add_test(NAME Console COMMAND test_console)

add_executable(
    test_event_console
    test_event_console.cpp
)

target_compile_features(test_event_console PRIVATE cxx_std_17)
target_compile_options(test_event_console PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_include_directories(test_event_console PRIVATE stubs ../src ${ESPRESSIO_COMMAND_INCLUDE_DIR} ${ESPRESSIO_OBSERVABLE_INCLUDE_DIR})
add_test(NAME EventConsoleContract COMMAND test_event_console)

add_executable(
    test_command_console
    test_command_console.cpp
)

target_compile_features(test_command_console PRIVATE cxx_std_17)
target_compile_options(test_command_console PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_include_directories(test_command_console PRIVATE stubs ../src ${ESPRESSIO_COMMAND_INCLUDE_DIR} ${ESPRESSIO_OBSERVABLE_INCLUDE_DIR})
add_test(NAME CommandConsoleContract COMMAND test_command_console)
