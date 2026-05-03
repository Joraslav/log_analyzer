FIND_LIB(glaze)

add_library(lib_report STATIC
    ConsoleReport.cpp
    JsonReport.cpp
)

target_include_directories(lib_report
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(lib_report PUBLIC
    lib_domain
    glaze::glaze
)

LOG_ANALYZER_CONFIGURE_TARGET(lib_report)
