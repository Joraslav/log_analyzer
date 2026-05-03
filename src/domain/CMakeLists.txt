add_library(lib_domain INTERFACE)

target_include_directories(lib_domain
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)