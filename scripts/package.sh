#!/bin/bash

set -euo pipefail

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

OUTPUT_DIR="$PROJECT_ROOT/build/Release/packages"
BUILD_DIR="$PROJECT_ROOT/build/Release"
SKIP_BUILD=false
GENERATOR=""

print_usage() {
    echo "Использование: $0 [--output-dir <путь>] [--skip-build] [--generator <DEB|RPM>]"
    echo ""
    echo "  --output-dir <путь>    Куда сохранить пакеты (по умолчанию: build/Release/packages)"
    echo "  --skip-build           Не пересобирать проект, только упаковать уже собранное"
    echo "  --generator <DEB|RPM>  Принудительно выбрать генератор пакета (по умолчанию: авто)"
}

while [ $# -gt 0 ]; do
    case "$1" in
        --output-dir)
            if [ -z "${2:-}" ]; then
                echo -e "${RED}Ошибка: не указан путь для --output-dir${NC}"
                print_usage; exit 1
            fi
            OUTPUT_DIR="$2"; shift 2 ;;
        --skip-build)
            SKIP_BUILD=true; shift ;;
        --generator)
            if [ -z "${2:-}" ]; then
                echo -e "${RED}Ошибка: не указан генератор для --generator${NC}"
                print_usage; exit 1
            fi
            case "$2" in
                DEB|RPM) GENERATOR="$2" ;;
                *)
                    echo -e "${RED}Ошибка: неверный генератор '$2'. Допустимые значения: DEB, RPM${NC}"
                    print_usage; exit 1 ;;
            esac
            shift 2 ;;
        -h|--help)
            print_usage; exit 0 ;;
        *)
            echo -e "${RED}Ошибка: неизвестный аргумент '$1'${NC}"
            print_usage; exit 1 ;;
    esac
done

for cmd in cmake cpack; do
    if ! command -v "$cmd" >/dev/null 2>&1; then
        echo -e "${RED}Ошибка: '$cmd' не найден в PATH${NC}"
        exit 1
    fi
done

cd "$PROJECT_ROOT"

if [ "$SKIP_BUILD" = false ]; then
    echo -e "${GREEN}Подготовка Release-окружения через Conan...${NC}"
    "$SCRIPT_DIR/create_env.sh" Release

    echo -e "${GREEN}Конфигурация Release-сборки...${NC}"
    cmake --preset conan-release -B "$BUILD_DIR"

    echo -e "${GREEN}Сборка проекта...${NC}"
    cmake --build "$BUILD_DIR"
fi

CPACK_CONFIG="$BUILD_DIR/CPackConfig.cmake"
if [ ! -f "$CPACK_CONFIG" ]; then
    echo -e "${RED}Ошибка: CPackConfig.cmake не найден в '$BUILD_DIR'.${NC}"
    echo -e "${YELLOW}Запустите скрипт без --skip-build или выполните конфигурацию CMake вручную.${NC}"
    exit 1
fi

echo -e "${GREEN}Создание пакетов...${NC}"

CPACK_ARGS=(--config "$CPACK_CONFIG" -B "$OUTPUT_DIR")
if [ -n "$GENERATOR" ]; then
    CPACK_ARGS+=(-G "$GENERATOR")
fi

cpack "${CPACK_ARGS[@]}"

echo ""
echo -e "${GREEN}Готово! Пакеты сохранены в: $OUTPUT_DIR${NC}"
find "$OUTPUT_DIR" -maxdepth 1 \( -name "*.deb" -o -name "*.rpm" \) -print | \
    while IFS= read -r pkg; do
        echo -e "  ${YELLOW}$(basename "$pkg")${NC}"
    done
