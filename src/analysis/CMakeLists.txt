add_library(lib_analysis STATIC
    FileAnalyzer.cpp
    Aggregator.cpp
)

target_include_directories(lib_analysis
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(lib_analysis PUBLIC
    lib_domain
    lib_parser
)

LOG_ANALYZER_CONFIGURE_TARGET(lib_analysis)
