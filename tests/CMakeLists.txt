cmake_minimum_required(VERSION 3.16)
project(ESPressioSerialTests LANGUAGES CXX)

enable_testing()

set(ESPRESSIO_SYSTEM_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio_Memory.hpp")
set(ESPRESSIO_COMMAND_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio_Command.hpp")
set(ESPRESSIO_OBSERVABLE_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio_Observable.hpp")
set(ESPRESSIO_SERIALIZABLE_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio_BinaryArchive.hpp")
set(ESPRESSIO_WIFI_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio_WiFi.hpp")
set(ESPRESSIO_LOGGING_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio_Logging.hpp")
set(ESPRESSIO_TIMING_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio_SystemClock.hpp")
set(ESPRESSIO_UNITS_INCLUDE_DIR "" CACHE PATH "Path containing ESPressio_Time.hpp")

add_executable(test_console test_console.cpp)
target_compile_features(test_console PRIVATE cxx_std_17)
target_compile_options(test_console PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_compile_definitions(test_console PRIVATE ESPRESSIO_SERIAL_TESTING)
target_include_directories(test_console PRIVATE stubs ../src ${ESPRESSIO_SYSTEM_INCLUDE_DIR})
add_test(NAME Console COMMAND test_console)

add_executable(test_serial_log_sink test_serial_log_sink.cpp)
target_compile_features(test_serial_log_sink PRIVATE cxx_std_17)
target_compile_options(test_serial_log_sink PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_include_directories(test_serial_log_sink PRIVATE
    stubs
    ../src
    ${ESPRESSIO_SYSTEM_INCLUDE_DIR}
    ${ESPRESSIO_LOGGING_INCLUDE_DIR}
    ${ESPRESSIO_OBSERVABLE_INCLUDE_DIR}
    ${ESPRESSIO_TIMING_INCLUDE_DIR}
    ${ESPRESSIO_UNITS_INCLUDE_DIR}
)
add_test(NAME SerialLogSink COMMAND test_serial_log_sink)

add_executable(test_event_console test_event_console.cpp)
target_compile_features(test_event_console PRIVATE cxx_std_17)
target_compile_options(test_event_console PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_include_directories(test_event_console PRIVATE stubs ../src ${ESPRESSIO_SYSTEM_INCLUDE_DIR} ${ESPRESSIO_COMMAND_INCLUDE_DIR} ${ESPRESSIO_OBSERVABLE_INCLUDE_DIR})
add_test(NAME EventConsoleContract COMMAND test_event_console)

add_executable(test_command_console test_command_console.cpp)
target_compile_features(test_command_console PRIVATE cxx_std_17)
target_compile_options(test_command_console PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_include_directories(test_command_console PRIVATE stubs ../src ${ESPRESSIO_SYSTEM_INCLUDE_DIR} ${ESPRESSIO_COMMAND_INCLUDE_DIR} ${ESPRESSIO_OBSERVABLE_INCLUDE_DIR})
add_test(NAME CommandConsoleContract COMMAND test_command_console)

add_executable(test_event_monitor_payload_safety test_event_monitor_payload_safety.cpp)
target_compile_features(test_event_monitor_payload_safety PRIVATE cxx_std_17)
target_compile_options(test_event_monitor_payload_safety PRIVATE -Wall -Wextra -Wpedantic -Werror)
target_include_directories(test_event_monitor_payload_safety PRIVATE stubs ../src ${ESPRESSIO_SYSTEM_INCLUDE_DIR} ${ESPRESSIO_SERIALIZABLE_INCLUDE_DIR})
add_test(NAME EventMonitorPayloadSafety COMMAND test_event_monitor_payload_safety)

add_executable(test_wifi_monitor test_wifi_monitor.cpp)
target_compile_features(test_wifi_monitor PRIVATE cxx_std_17)
target_compile_options(test_wifi_monitor PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-error=misleading-indentation)
target_include_directories(test_wifi_monitor PRIVATE
    stubs
    ../src
    ${ESPRESSIO_SYSTEM_INCLUDE_DIR}
    ${ESPRESSIO_WIFI_INCLUDE_DIR}
    ${ESPRESSIO_OBSERVABLE_INCLUDE_DIR}
    ${ESPRESSIO_SERIALIZABLE_INCLUDE_DIR}
)
add_test(NAME WiFiMonitor COMMAND test_wifi_monitor)
