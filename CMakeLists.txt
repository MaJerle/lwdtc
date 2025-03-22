cmake_minimum_required(VERSION 3.22)

# Setup project
project(LwLibPROJECT)

if(NOT PROJECT_IS_TOP_LEVEL)
    add_subdirectory(lwdtc)
else()
    enable_testing()
    add_executable(${PROJECT_NAME})
    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/dev/main.c

        # Examples
        ${CMAKE_CURRENT_LIST_DIR}/examples/cron_basic.c
        ${CMAKE_CURRENT_LIST_DIR}/examples/cron_multi.c
        ${CMAKE_CURRENT_LIST_DIR}/examples/cron_calc_range.c
        ${CMAKE_CURRENT_LIST_DIR}/examples/cron_dt_range.c
        ${CMAKE_CURRENT_LIST_DIR}/tests/test_cron.c
    )
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/dev
    )

    # Add subdir with lwdtc and link to project
    set(LWDTC_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/dev/lwdtc_opts.h)
    add_subdirectory(lwdtc)
    target_link_libraries(${PROJECT_NAME} PUBLIC lwdtc)

    # Add compile options to the library, which will propagate options to executable through public link
    target_compile_definitions(lwdtc PUBLIC WIN32 _DEBUG CONSOLE LWDTC_DEV)
    target_compile_options(lwdtc PUBLIC -Wall -Wextra -Wpedantic)

    # Add test
    add_test(NAME Test COMMAND $<TARGET_FILE:${CMAKE_PROJECT_NAME}>)
endif()