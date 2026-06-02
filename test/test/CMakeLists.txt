cmake_minimum_required(VERSION 3.14)
project(XM125_IR_MINTIA
    DESCRIPTION "XM125 IR MINTIA Unit Tests"
)

# Include FetchContent module
include(FetchContent)

# Fetch cJSON library
FetchContent_Declare(
    cjson
    GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
    GIT_TAG v1.7.18 # ESP-IDF v5.4.1で使用されているバージョンを使用
)

FetchContent_MakeAvailable(cjson)

# Coverage flags for GCC/Clang
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

# Locate GTest (includes Google Mock as well)
find_package(GTest REQUIRED)


# Add the library directory
add_subdirectory(../double/ build/double)

set(PRODUCT_CODE
    ../../main/src/domain/command_interface/command_request.cpp
    ../../main/src/domain/command_interface/command_response.cpp
    ../../main/src/domain/command_interface/sensor_data_message.cpp
    ../../main/src/domain/command_interface/communication_packet.cpp
    ../../main/src/service/data_parser_service.cpp
    ../../main/src/service/amg88xx_ir_sensor_service.cpp
    ../../main/src/service/mlx90640_ir_sensor_service.cpp
)

set(PRODUCT_HEADER
    ../../main/include/domain
    ../../main/include/service
)

set(TEST_CODE
    ${CMAKE_CURRENT_SOURCE_DIR}/domain/command_interface/test_command_request.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/domain/command_interface/test_command_response.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/domain/command_interface/test_sensor_data_message.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/service/test_amg88xx_ir_sensor_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/service/test_data_parser_service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/service/test_mlx90640_ir_sensor_service.cpp
)

message("PRODUCT_CODE : ${PRODUCT_CODE}")
message("PRODUCT_HEADER : ${PRODUCT_HEADER}")

include_directories(${GTEST_INCLUDE_DIRS}
    ../double
    ${PRODUCT_HEADER}
    ${cjson_SOURCE_DIR} # cJSONのヘッダーファイルパス
)

# Link runTests with what we want to test and the GTest and pthread library
add_executable(runTests test_main.cpp ${PRODUCT_CODE} ${TEST_CODE})
target_link_libraries(runTests ${GTEST_LIBRARIES} gmock gmock_main pthread cjson double)
