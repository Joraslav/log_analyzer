add_library(lib_parser STATIC
	Tokenizer.cpp
)

target_include_directories(lib_parser
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
		$<INSTALL_INTERFACE:include>
)

LOG_ANALYZER_CONFIGURE_TARGET(lib_parser)
