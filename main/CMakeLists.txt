FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.cpp)

idf_component_register(SRCS ${app_sources}
    INCLUDE_DIRS
        "."
        "include"
        "include/task"
        # "include/dt_plus"
        "include/domain"
        "include/service"
    REQUIRES dt_plus driver esp_common freertos amg8833 mlx90640 json uart_log bt nvs_flash xm125)
