#!/bin/bash

echo "=== Проверка окружения ==="

ALL_CHECKS_PASSED=true

# Проверка компилятора g++
echo -n "Проверка g++... "
if command -v g++ >/dev/null 2>&1; then
    echo "OK"
else
    echo "WARNING: g++ не найден. (Для компиляции C++ потребуется установить MinGW)"
    ALL_CHECKS_PASSED=false
fi

# Проверка cmake
echo -n "Проверка cmake... "
if command -v cmake >/dev/null 2>&1; then
    echo "OK"
else
    echo "WARNING: cmake не найден. (Для сборки проекта потребуется установить CMake)"
    ALL_CHECKS_PASSED=false
fi

# Проверка netcat (nc)
echo -n "Проверка netcat (nc)... "
if command -v nc >/dev/null 2>&1; then
    echo "OK"
else
    echo "WARNING: netcat не найден. (Некоторые сетевые тесты будут использовать эмуляцию)"
    ALL_CHECKS_PASSED=false
fi

# Проверка порта 5555 (пропускаем если netstat недоступен)
echo -n "Проверка порта 5555... "
if command -v netstat >/dev/null 2>&1; then
    if netstat -an 2>/dev/null | grep -E "(:5555|:5555 )" | grep -i listen >/dev/null 2>&1; then
        echo "ERROR: Порт 5555 уже занят."
        exit 1
    else
        echo "OK"
    fi
else
    echo "SKIP (netstat не доступен)"
fi

# Проверка прав на запись в папку tests/tmp
echo -n "Проверка прав на tests/tmp... "
if mkdir -p tests/tmp && touch tests/tmp/test_write >/dev/null 2>&1; then
    rm tests/tmp/test_write
    echo "OK"
else
    echo "ERROR: Нет прав на создание/запись в tests/tmp."
    exit 1
fi

# Проверка, что скрипты исполняемые
chmod +x tests/*.sh

if [ "$ALL_CHECKS_PASSED" = true ]; then
    echo "=== Все проверки пройдены! ==="
    exit 0
else
    echo "=== Основные проверки пройдены, но есть предупреждения ==="
    echo "Тестовая инфраструктура готова к работе!"
    exit 0
fi
