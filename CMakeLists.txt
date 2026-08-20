idf_component_register(
    INCLUDE_DIRS
        "src"
        "src/console"
        "src/command-console"
        "src/diagnostics"
        "src/event"
        "src/event-console"
        "src/logging"
        "src/threads"
        "src/timing"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
