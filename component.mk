COMPONENT_ADD_INCLUDEDIRS := \
    src \
    src/console \
    src/command \
    src/command-console \
    src/diagnostics \
    src/espnow \
    src/event \
    src/event-console \
    src/logging \
    src/security \
    src/sockets \
    src/threads \
    src/timing \
    src/wifi

COMPONENT_SRCDIRS := src

CXXFLAGS += -std=gnu++17

CPPFLAGS += \
    -DESPRESSIO_SERIAL \
    -DESPRESSIO_SERIAL_VERSION_MAJOR=0 \
    -DESPRESSIO_SERIAL_VERSION_MINOR=8 \
    -DESPRESSIO_SERIAL_VERSION_PATCH=1 \
    -DESPRESSIO_SERIAL_VERSION_STRING=\"0.8.1\"
