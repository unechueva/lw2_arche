#!/bin/bash

echo "=== ЗАПУСК ВСЕХ ТЕСТОВ ==="
echo "Время начала: $(date)"
echo ""

mkdir -p tests/tmp/results
OVERALL_RESULT="PASSED"

TESTS=(
    "env_check.sh"
    "create_example_data.sh"
    "manual_client_server_simple.sh" 
    "auto_ping_test.sh"
    "auto_msg_test.sh"
    "reconnect_test.sh"
)

for test in "${TESTS[@]}"; do
    echo "--- Запуск $test ---"
    if ./tests/$test; then
        echo "✅ $test: PASSED"
        echo "$test: PASSED" >> tests/tmp/results/summary.txt
    else
        echo "❌ $test: FAILED"
        echo "$test: FAILED" >> tests/tmp/results/summary.txt
        OVERALL_RESULT="FAILED"
    fi
    echo ""
done

echo "--- Анализ логов ---"
./tests/log_parser.sh tests/tmp/auto_ping_test.log
echo ""

echo "=== ИТОГОВЫЙ ОТЧЕТ ==="
cat tests/tmp/results/summary.txt 2>/dev/null || echo "Файл с результатами не найден"
echo ""
echo "ОБЩИЙ РЕЗУЛЬТАТ: $OVERALL_RESULT"
echo "Время окончания: $(date)"

echo "Тесты завершены с результатом: $OVERALL_RESULT" > tests/tmp/final_result.txt
