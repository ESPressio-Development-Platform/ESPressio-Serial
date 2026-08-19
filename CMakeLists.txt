idf_component_register(
    INCLUDE_DIRS
        "src"
        "src/event"
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
