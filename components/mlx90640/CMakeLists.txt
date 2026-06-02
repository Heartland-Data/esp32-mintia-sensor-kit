set(COMPONENT_SRCS
    "mlx90640_api.c"
    "mlx90640_i2c_driver.c"
)

idf_component_register(
    SRCS ${COMPONENT_SRCS}
    INCLUDE_DIRS "."
    REQUIRES driver esp_common freertos
    PRIV_REQUIRES hal
)
