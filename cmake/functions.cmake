include(${CMAKE_CURRENT_LIST_DIR}/ProjectOptions.cmake)

function(FIND_LIB LIB_NAME)
    find_package(${LIB_NAME} QUIET)
    if(NOT ${LIB_NAME}_FOUND)
        unset(${LIB_NAME}_DIR CACHE)
        list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/generators)
        find_package(${LIB_NAME} QUIET PATHS ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/generators NO_DEFAULT_PATH)
        if(NOT ${LIB_NAME}_FOUND)
            message(FATAL_ERROR "Lib ${LIB_NAME} not found. Execute 'create_env'")
        endif()
    endif()
endfunction()

function(LOG_ANALYZER_CONFIGURE_TARGET TARGET_NAME)
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target ${TARGET_NAME} does not exist")
    endif()

    get_target_property(target_type ${TARGET_NAME} TYPE)

    if(target_type STREQUAL "INTERFACE_LIBRARY")
        target_link_libraries(${TARGET_NAME}
            INTERFACE
                log_analyzer_project_defaults
        )
        return()
    endif()

    target_link_libraries(${TARGET_NAME}
        PRIVATE
            log_analyzer_project_defaults
    )
endfunction()

function(ADD_GTEST_TARGET TEST_NAME)
    set(options)
    set(one_value_args)
    set(multi_value_args SOURCES LINK_LIBS COMPILE_DEFINITIONS)
    cmake_parse_arguments(ARG "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT ARG_SOURCES)
        set(ARG_SOURCES ${TEST_NAME}.cpp)
    endif()

    add_executable(${TEST_NAME} ${ARG_SOURCES})

    target_link_libraries(${TEST_NAME}
        PRIVATE
            GTest::gtest_main
            ${ARG_LINK_LIBS}
    )

    if(ARG_COMPILE_DEFINITIONS)
        target_compile_definitions(${TEST_NAME}
            PRIVATE
                ${ARG_COMPILE_DEFINITIONS}
        )
    endif()

    LOG_ANALYZER_CONFIGURE_TARGET(${TEST_NAME})

    gtest_discover_tests(${TEST_NAME}
        NO_PRETTY_VALUES
        # NO_PRETTY_TYPES
    )
endfunction()
