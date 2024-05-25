# 
# LIB_PREFIX: LWDTC
#
# This file provides set of variables for end user
# and also generates one (or more) libraries, that can be added to the project using target_link_libraries(...)
#
# Before this file is included to the root CMakeLists file (using include() function), user can set some variables:
#
# LWDTC_OPTS_FILE: If defined, it is the path to the user options file. If not defined, one will be generated for you automatically
# LWDTC_COMPILE_OPTIONS: If defined, it provide compiler options for generated library.
# LWDTC_COMPILE_DEFINITIONS: If defined, it provides "-D" definitions to the library build
#

# Custom include directory
set(LWDTC_CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib_inc)

# Library core sources
set(lwdtc_core_SRCS 
    ${CMAKE_CURRENT_LIST_DIR}/src/lwdtc/lwdtc.c
)

# Setup include directories
set(lwdtc_include_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/src/include
    ${LWDTC_CUSTOM_INC_DIR}
)

# Register library to the system
add_library(lwdtc INTERFACE)
target_sources(lwdtc INTERFACE ${lwdtc_core_SRCS})
target_include_directories(lwdtc INTERFACE ${lwdtc_include_DIRS})
target_compile_options(lwdtc PRIVATE ${LWDTC_COMPILE_OPTIONS})
target_compile_definitions(lwdtc PRIVATE ${LWDTC_COMPILE_DEFINITIONS})

# Create config file if user didn't provide one info himself
if(NOT LWDTC_OPTS_FILE)
    message(STATUS "Using default lwdtc_opts.h file")
    set(LWDTC_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/src/include/lwdtc/lwdtc_opts_template.h)
else()
    message(STATUS "Using custom lwdtc_opts.h file from ${LWDTC_OPTS_FILE}")
endif()
configure_file(${LWDTC_OPTS_FILE} ${LWDTC_CUSTOM_INC_DIR}/lwdtc_opts.h COPYONLY)
