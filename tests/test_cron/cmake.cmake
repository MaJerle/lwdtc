# CMake include file

# Add more sources
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/test_cron.c
    ${CMAKE_CURRENT_LIST_DIR}/test_regression.c
)

# Options file
set(LWDTC_OPTS_FILE ${CMAKE_CURRENT_LIST_DIR}/lwdtc_opts.h)