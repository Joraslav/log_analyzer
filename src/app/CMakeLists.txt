add_library(lib_app STATIC
    Cli.cpp
    App.cpp
)

target_include_directories(lib_app
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(lib_app PUBLIC
    lib_domain
    lib_io
    lib_analysis
    lib_report
    lib_concurrency
)

LOG_ANALYZER_CONFIGURE_TARGET(lib_app)
