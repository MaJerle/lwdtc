cmake_minimum_required(VERSION 3.22)

# Register library to the system
add_library(lwdtc INTERFACE)
target_sources(lwdtc INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/lwdtc/lwdtc.c)
target_include_directories(lwdtc INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/include)

# Create config file
if(DEFINED LWDTC_OPTS_DIR AND NOT EXISTS ${LWDTC_OPTS_DIR}/lwdtc_opts.h)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/src/include/lwdtc/lwdtc_opts_template.h ${LWDTC_OPTS_DIR}/lwdtc_opts.h COPYONLY)
endif()
