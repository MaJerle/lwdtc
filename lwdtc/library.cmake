# Library core sources
set(lwdtc_core_SRCS 
    ${CMAKE_CURRENT_LIST_DIR}/src/lwdtc/lwdtc.c
)

# Setup include directories
set(lwdtc_include_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/src/include
)

# Register library to the system
add_library(lwdtc INTERFACE)
target_sources(lwdtc INTERFACE ${lwdtc_core_SRCS})
target_include_directories(lwdtc INTERFACE ${lwdtc_include_DIRS})
target_compile_options(lwdtc PRIVATE ${LWDTC_COMPILE_OPTIONS})
target_compile_definitions(lwdtc PRIVATE ${LWDTC_COMPILE_DEFINITIONS})

# Create config file
if(DEFINED LWDTC_OPTS_DIR AND NOT EXISTS ${LWDTC_OPTS_DIR}/lwdtc_opts.h)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/src/include/lwdtc/lwdtc_opts_template.h ${LWDTC_OPTS_DIR}/lwdtc_opts.h COPYONLY)
endif()
