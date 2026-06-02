# DT+ GPIO 2bit Driver Component for ESP-IDF

# Collect all source files in the component directory
file(GLOB_RECURSE COMPONENT_SRCS
    "*.c"
    "*.cpp"
)

# Register the component
idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS "."
    REQUIRES driver esp_common freertos
    PRIV_REQUIRES hal
)
