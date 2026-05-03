include(CMakePushCheckState)
include(CheckCXXSourceCompiles)

option(LOG_ANALYZER_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

function(LOG_ANALYZER_CHECK_SANITIZER_SUPPORT RESULT_VAR)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
        set(${RESULT_VAR} OFF PARENT_SCOPE)
        return()
    endif()

    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_FLAGS "-fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address,undefined")
    check_cxx_source_compiles("int main() { return 0; }" log_analyzer_sanitizers_supported)
    cmake_pop_check_state()

    set(${RESULT_VAR} ${log_analyzer_sanitizers_supported} PARENT_SCOPE)
endfunction()

function(LOG_ANALYZER_SETUP_PROJECT_OPTIONS)
    if(TARGET log_analyzer_project_defaults)
        return()
    endif()

    set(log_analyzer_sanitizer_compile_options)
    set(log_analyzer_sanitizer_link_options)

    if(LOG_ANALYZER_ENABLE_SANITIZERS)
        LOG_ANALYZER_CHECK_SANITIZER_SUPPORT(LOG_ANALYZER_SANITIZERS_AVAILABLE)

        if(LOG_ANALYZER_SANITIZERS_AVAILABLE)
            set(log_analyzer_sanitizer_compile_options
                $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fsanitize=address,undefined>
                $<$<AND:$<CONFIG:Debug>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fno-omit-frame-pointer>
            )
            set(log_analyzer_sanitizer_link_options
                $<$<AND:$<CONFIG:Debug>,$<LINK_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-fsanitize=address,undefined>
            )
        else()
            message(STATUS "LOG_ANALYZER_ENABLE_SANITIZERS is ON,
                but an end-to-end sanitizer compile/link check failed.
                Continuing without sanitizers.")
        endif()
    endif()

    add_library(log_analyzer_project_options INTERFACE)
    target_compile_options(log_analyzer_project_options
        INTERFACE
            $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-fdiagnostics-color=always>
            ${log_analyzer_sanitizer_compile_options}
    )
    target_link_options(log_analyzer_project_options
        INTERFACE
            ${log_analyzer_sanitizer_link_options}
    )

    add_library(log_analyzer_project_warnings INTERFACE)
    target_compile_options(log_analyzer_project_warnings
        INTERFACE
            $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall>
            $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wextra>
            $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wpedantic>
            $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wshadow>
            $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wconversion>
            $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wsign-conversion>
            $<$<AND:$<BOOL:${LOG_ANALYZER_WARNINGS_AS_ERRORS}>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>>:-Werror>
    )

    add_library(log_analyzer_project_defaults INTERFACE)
    target_link_libraries(log_analyzer_project_defaults
        INTERFACE
            log_analyzer_project_options
            log_analyzer_project_warnings
    )
endfunction()