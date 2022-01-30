cmake_minimum_required(VERSION 3.22)

# Debug message
message("Exiting ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")

# Register library to the system
add_library(lwdtc INTERFACE)
target_sources(lwdtc INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/lwdtc/lwdtc.c)
target_include_directories(lwdtc INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/include)

# Debug message
message("Exiting ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")
