FIND_LIB(GTest)
include(GoogleTest)

# ---------- Тесты ----------
ADD_GTEST_TARGET(Test_FileScanner
    LINK_LIBS
        lib_io
        lib_domain
    COMPILE_DEFINITIONS
        TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

ADD_GTEST_TARGET(Test_Parser
    LINK_LIBS
        lib_parser
)

ADD_GTEST_TARGET(Test_FileAnalyzer
    LINK_LIBS
        lib_analysis
        lib_domain
    COMPILE_DEFINITIONS
        TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)

ADD_GTEST_TARGET(Test_Aggregator
    LINK_LIBS
        lib_analysis
        lib_domain
)

ADD_GTEST_TARGET(Test_ConsoleReport
    LINK_LIBS
        lib_report
        lib_domain
)

ADD_GTEST_TARGET(Test_JsonReport
    LINK_LIBS
        lib_report
        lib_domain
)

ADD_GTEST_TARGET(Test_Cli
    LINK_LIBS
        lib_app
        lib_domain
)

ADD_GTEST_TARGET(Test_App
    LINK_LIBS
        lib_app
        lib_domain
    COMPILE_DEFINITIONS
        TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data"
)