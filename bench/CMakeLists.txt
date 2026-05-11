FIND_LIB(benchmark)

add_executable(Benchmark_AppModes
    AppModesBenchmark.cpp
)

target_link_libraries(Benchmark_AppModes
    PRIVATE
        benchmark::benchmark_main
        lib_analysis
        lib_concurrency
        lib_domain
        lib_io
)

target_include_directories(Benchmark_AppModes
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

target_compile_definitions(Benchmark_AppModes
    PRIVATE
        LOG_ANALYZER_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

LOG_ANALYZER_CONFIGURE_TARGET(Benchmark_AppModes)