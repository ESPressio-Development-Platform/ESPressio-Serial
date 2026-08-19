COMPONENT_ADD_INCLUDEDIRS := \
    src \
    src/console \
    src/diagnostics \
    src/event \
    src/event-console \
    src/logging \
    src/threads \
    src/timing

COMPONENT_SRCDIRS := src

CXXFLAGS += -std=gnu++17

CPPFLAGS += \
    -DESPRESSIO_SERIAL \
    -DESPRESSIO_SERIAL_VERSION_MAJOR=0 \
    -DESPRESSIO_SERIAL_VERSION_MINOR=3 \
    -DESPRESSIO_SERIAL_VERSION_PATCH=2 \
    -DESPRESSIO_SERIAL_VERSION_STRING=\"0.3.2\"
