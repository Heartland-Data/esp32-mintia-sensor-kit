# UART Log Component for ESP-IDF

file(GLOB_RECURSE COMPONENT_SRCS
    "*.c"
)

idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS "."
    REQUIRES driver esp_common freertos
)
