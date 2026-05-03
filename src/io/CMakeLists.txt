add_library(lib_io STATIC
    FileScanner.cpp
)

target_include_directories(lib_io
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(lib_io
    PUBLIC
        lib_domain
)

LOG_ANALYZER_CONFIGURE_TARGET(lib_io)