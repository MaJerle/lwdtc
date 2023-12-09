# 
# This file provides set of variables for end user
# and also generates one (or more) libraries, that can be added to the project using target_link_libraries(...)
#
# Before this file is included to the root CMakeLists file (using include() function), user can set some variables:
#
# LWDTC_OPTS_DIR: If defined, it should set the folder path where options file shall be generated.
# LWDTC_COMPILE_OPTIONS: If defined, it provide compiler options for generated library.
# LWDTC_COMPILE_DEFINITIONS: If defined, it provides "-D" definitions to the library build
#

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
