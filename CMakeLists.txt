cmake_minimum_required(VERSION 3.22)

# Setup project
project(LwLibPROJECT)

if(NOT PROJECT_IS_TOP_LEVEL)
    add_subdirectory(lwdtc)
else()
    # Set as executable
    add_executable(${PROJECT_NAME})

    # Add key executable block
    target_sources(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/dev/main.c

        # Examples
        ${CMAKE_CURRENT_LIST_DIR}/examples/cron_basic.c
        ${CMAKE_CURRENT_LIST_DIR}/examples/cron_multi.c
        ${CMAKE_CURRENT_LIST_DIR}/examples/cron_calc_range.c
        ${CMAKE_CURRENT_LIST_DIR}/examples/cron_dt_range.c
    )

    # Add key include paths
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/dev
    )

    # Compilation definition information
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        WIN32
        _DEBUG
        CONSOLE
        LWDTC_DEV
    )

    # Compiler options
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    # Add subdir with lwdtc and link to project
    set(LWDTC_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/dev/lwdtc_opts.h)
    add_subdirectory(lwdtc)
    target_link_libraries(${PROJECT_NAME} lwdtc)
endif()