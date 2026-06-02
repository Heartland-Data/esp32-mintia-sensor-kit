idf_component_register(
    SRCS "xm125_esp32.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_common freertos
)
