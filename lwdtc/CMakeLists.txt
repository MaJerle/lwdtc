cmake_minimum_required(VERSION 3.22)

# Register library to the system
add_library(lwdtc INTERFACE)
target_sources(lwdtc INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/lwdtc/lwdtc.c)
target_include_directories(lwdtc INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/include)
