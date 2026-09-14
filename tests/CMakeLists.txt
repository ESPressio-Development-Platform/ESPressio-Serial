cmake_minimum_required(VERSION 3.16)
project(ESPressioSerialTests LANGUAGES CXX)

enable_testing()

set(ESPRESSIO_SYSTEM_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio System headers")
set(ESPRESSIO_LOGGING_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Logging headers")
set(ESPRESSIO_PRIMITIVE_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Primitive headers")
set(ESPRESSIO_COMMAND_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Command headers")
set(ESPRESSIO_EVENT_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Event headers")
set(ESPRESSIO_STATE_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio State headers")
set(ESPRESSIO_TASK_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Task headers")
set(ESPRESSIO_THREADS_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Threads headers")
set(ESPRESSIO_TIMING_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Timing headers")
set(ESPRESSIO_UNITS_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Units headers")
set(ESPRESSIO_OBSERVABLE_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Observable headers")
set(ESPRESSIO_SERIALIZABLE_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Serializable headers")
set(ESPRESSIO_PERSISTENCE_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Persistence headers")
set(ESPRESSIO_SECURITY_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio Security headers")
set(ESPRESSIO_WIFI_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio WiFi headers")
set(ESPRESSIO_TIMING_TEST_STUB_DIR "" CACHE PATH "Timing host platform stubs")
set(ESPRESSIO_TASK_TEST_DIR "" CACHE PATH "Task host test support")

set(ESPRESSIO_FINAL_INCLUDE_DIRS
    stubs
    ../src
    ${ESPRESSIO_SYSTEM_INCLUDE_DIR}
    ${ESPRESSIO_LOGGING_INCLUDE_DIR}
    ${ESPRESSIO_PRIMITIVE_INCLUDE_DIR}
    ${ESPRESSIO_COMMAND_INCLUDE_DIR}
    ${ESPRESSIO_EVENT_INCLUDE_DIR}
    ${ESPRESSIO_STATE_INCLUDE_DIR}
    ${ESPRESSIO_TASK_INCLUDE_DIR}
    ${ESPRESSIO_THREADS_INCLUDE_DIR}
    ${ESPRESSIO_TIMING_INCLUDE_DIR}
    ${ESPRESSIO_UNITS_INCLUDE_DIR}
    ${ESPRESSIO_OBSERVABLE_INCLUDE_DIR}
    ${ESPRESSIO_SERIALIZABLE_INCLUDE_DIR}
    ${ESPRESSIO_PERSISTENCE_INCLUDE_DIR}
    ${ESPRESSIO_SECURITY_INCLUDE_DIR}
    ${ESPRESSIO_WIFI_INCLUDE_DIR}
    ${ESPRESSIO_TIMING_TEST_STUB_DIR}
    ${ESPRESSIO_TASK_TEST_DIR}
)

function(espressio_serial_test target source)
    add_executable(${target} ${source})
    target_compile_features(${target} PRIVATE cxx_std_17)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_include_directories(${target} PRIVATE ${ESPRESSIO_FINAL_INCLUDE_DIRS})
endfunction()

espressio_serial_test(test_console test_console.cpp)
target_compile_definitions(test_console PRIVATE ESPRESSIO_SERIAL_TESTING)
add_test(NAME Console COMMAND test_console)

espressio_serial_test(test_serial_log_sink test_serial_log_sink.cpp)
add_test(NAME SerialLogSink COMMAND test_serial_log_sink)

espressio_serial_test(test_event_console test_event_console.cpp)
add_test(NAME EventConsoleContract COMMAND test_event_console)

espressio_serial_test(test_command_console test_command_console.cpp)
add_test(NAME CommandConsoleContract COMMAND test_command_console)

espressio_serial_test(test_primitive_family_tooling test_primitive_family_tooling.cpp)
add_test(NAME PrimitiveFamilyToolingContract COMMAND test_primitive_family_tooling)

espressio_serial_test(test_wifi_monitor test_wifi_monitor.cpp)
target_compile_options(test_wifi_monitor PRIVATE -Wno-error=misleading-indentation)
add_test(NAME WiFiMonitor COMMAND test_wifi_monitor)
