cmake_minimum_required(VERSION 3.31.0)
project(log_analyzer VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

option(LOG_ANALYZER_ENABLE_SANITIZERS "Enable address and undefined sanitizers for Debug builds" OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/functions.cmake)

LOG_ANALYZER_SETUP_PROJECT_OPTIONS()

# ---------- Модули (статические библиотеки) ----------
add_subdirectory(src/domain)
add_subdirectory(src/io)
add_subdirectory(src/parser)
add_subdirectory(src/analysis)
add_subdirectory(src/concurrency)
add_subdirectory(src/report)
add_subdirectory(src/app)

# ---------- Исполняемый файл ----------
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE lib_app)
LOG_ANALYZER_CONFIGURE_TARGET(${PROJECT_NAME})

# ---------- Установка ----------
install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${CMAKE_SOURCE_DIR}/README.md
	DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# ---------- Тесты ----------
enable_testing()
add_subdirectory(tests)

# ---------- Пакетирование ----------
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Packaging.cmake)
