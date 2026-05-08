#!/bin/bash

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

BUILD_DIR="build"
BUILD_TYPE="Debug"
BUILD_JOBS=""
TEST_JOBS=""
TEST_FILTER=""
LIST_ONLY=false
NO_BUILD=false

print_usage() {
	echo "Использование: $0 [--build-dir <path>] [--build-type <Debug|Release|...>] [--build-jobs <N>] [--jobs <N>] [--filter <gtest_pattern>] [--list] [--no-build]"
}

while [ $# -gt 0 ]; do
	case "$1" in
		--build-dir)
			if [ -z "$2" ]; then
				echo -e "${RED}✗ Не указан путь для --build-dir.${NC}"
				print_usage
				exit 1
			fi
			BUILD_DIR="$2"
			shift 2
			;;
		--build-type)
			if [ -z "$2" ]; then
				echo -e "${RED}✗ Не указан тип сборки для --build-type.${NC}"
				print_usage
				exit 1
			fi
			BUILD_TYPE="$2"
			shift 2
			;;
		--build-jobs)
			if [ -z "$2" ] || ! [[ "$2" =~ ^[1-9][0-9]*$ ]]; then
				echo -e "${RED}✗ --build-jobs должен быть положительным целым числом.${NC}"
				print_usage
				exit 1
			fi
			BUILD_JOBS="$2"
			shift 2
			;;
		--jobs)
			if [ -z "$2" ] || ! [[ "$2" =~ ^[1-9][0-9]*$ ]]; then
				echo -e "${RED}✗ --jobs должен быть положительным целым числом.${NC}"
				print_usage
				exit 1
			fi
			TEST_JOBS="$2"
			shift 2
			;;
		--filter)
			if [ -z "$2" ]; then
				echo -e "${RED}✗ Не указано значение для --filter.${NC}"
				print_usage
				exit 1
			fi
			TEST_FILTER="$2"
			shift 2
			;;
		--list)
			LIST_ONLY=true
			shift
			;;
		--no-build)
			NO_BUILD=true
			shift
			;;
		--help)
			print_usage
			exit 0
			;;
		*)
			echo -e "${RED}✗ Неизвестный аргумент: $1${NC}"
			print_usage
			exit 1
			;;
	esac
done

if ! command -v cmake &> /dev/null; then
	echo -e "${RED}✗ cmake не найден. Установите его и повторите попытку.${NC}"
	exit 1
fi

if ! command -v xargs &> /dev/null; then
	echo -e "${RED}✗ xargs не найден. Установите findutils и повторите попытку.${NC}"
	exit 1
fi

if [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
	echo -e "${YELLOW}Сборочная директория не настроена. Запускаю CMake configure...${NC}"
	cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
fi

if [ "$NO_BUILD" = false ]; then
	echo -e "${YELLOW}Собираю проект перед запуском тестов...${NC}"
	build_cmd=(cmake --build "$BUILD_DIR")
	if [ -n "$BUILD_JOBS" ]; then
		build_cmd+=(--parallel "$BUILD_JOBS")
	fi
	"${build_cmd[@]}"
fi

TEST_BIN_DIR="$BUILD_DIR/tests"

if [ ! -d "$TEST_BIN_DIR" ]; then
	echo -e "${RED}✗ Папка с тестами не найдена: $TEST_BIN_DIR${NC}"
	exit 1
fi

mapfile -t test_binaries < <(find "$TEST_BIN_DIR" -maxdepth 1 -type f -name "Test_*" -executable | sort)

if [ ${#test_binaries[@]} -eq 0 ]; then
	echo -e "${RED}✗ Не найдены исполняемые тестовые бинарники в $TEST_BIN_DIR${NC}"
	exit 1
fi

if [ "$LIST_ONLY" = true ]; then
	echo -e "${YELLOW}Список доступных тестов:${NC}"
	for binary in "${test_binaries[@]}"; do
		echo -e "${YELLOW}--- $(basename "$binary") ---${NC}"
		if [ -n "$TEST_FILTER" ]; then
			"$binary" --gtest_list_tests | grep -E "$TEST_FILTER" || true
		else
			"$binary" --gtest_list_tests
		fi
	done
	echo -e "${GREEN}✓ Список тестов получен успешно.${NC}"
    exit 0
else
    echo -e "${YELLOW}Запускаю тесты напрямую через GoogleTest...${NC}"
fi

if [ -z "$TEST_JOBS" ]; then
	TEST_JOBS=1
fi

if ! [[ "$TEST_JOBS" =~ ^[1-9][0-9]*$ ]]; then
	TEST_JOBS=1
fi

if [ -n "$TEST_FILTER" ]; then
	echo -e "${YELLOW}Применён фильтр GoogleTest: $TEST_FILTER${NC}"
fi

set +e
if [ "$TEST_JOBS" -gt 1 ]; then
	echo -e "${YELLOW}Параллельный запуск тестовых бинарников: ${TEST_JOBS} поток(ов).${NC}"
	printf '%s\0' "${test_binaries[@]}" | xargs -0 -n1 -P "$TEST_JOBS" bash -c '
		binary="$1"
		if [ -n "$TEST_FILTER" ]; then
			"$binary" --gtest_color=yes --gtest_filter="$TEST_FILTER"
		else
			"$binary" --gtest_color=yes
		fi
	' _
    run_exit_code=$?
else
	for binary in "${test_binaries[@]}"; do
		echo -e "${YELLOW}Запуск $(basename "$binary")...${NC}"
		if [ -n "$TEST_FILTER" ]; then
			"$binary" --gtest_color=yes --gtest_filter="$TEST_FILTER"
		else
			"$binary" --gtest_color=yes
		fi
		binary_exit_code=$?
		if [ $binary_exit_code -ne 0 ]; then
			run_exit_code=$binary_exit_code
			break
		fi
		unset binary_exit_code
	done
fi
set -e

if [ -z "$run_exit_code" ]; then
	run_exit_code=0
fi

if [ $run_exit_code -eq 0 ]; then
	echo -e "${GREEN}✓ Все тесты прошли успешно!${NC}"
else
	echo -e "${RED}✗ Тесты завершились с ошибкой. Код возврата: $run_exit_code${NC}"
	exit $run_exit_code
fi