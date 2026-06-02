# IR-AMG8833 Driver Component for ESP-IDF

# Specify source files explicitly for better control
set(COMPONENT_SRCS
    "amg88xx_esp32.c"
)

# Register the component
idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS "."
    REQUIRES driver esp_common freertos
    PRIV_REQUIRES hal
)
