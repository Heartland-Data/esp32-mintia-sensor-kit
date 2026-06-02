# Create the double library
FILE(GLOB_RECURSE SOURCES *.cpp *.c)

add_library(double SHARED
    ${SOURCES}
)

message("CMAKE_CURRENT_SOURCE_DIR : ${CMAKE_CURRENT_SOURCE_DIR}")

set(PRODUCT_HEADER
    ../../main/include/domain
)
target_include_directories(double PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    ${PRODUCT_HEADER}
)
