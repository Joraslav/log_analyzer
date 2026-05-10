add_library(lib_concurrency STATIC
    ThreadPool.cpp
    ThreadSafeAggregator.cpp
)

target_include_directories(lib_concurrency
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

find_package(Threads REQUIRED)
target_link_libraries(lib_concurrency PUBLIC
    Threads::Threads
    lib_domain
)

LOG_ANALYZER_CONFIGURE_TARGET(lib_concurrency)
