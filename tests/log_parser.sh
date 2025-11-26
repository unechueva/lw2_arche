#!/bin/bash

echo "=== Анализатор логов ==="

LOG_FILE="${1:-tests/tmp/auto_ping_test.log}"

if [ ! -f "$LOG_FILE" ]; then
    echo "Ошибка: Лог-файл $LOG_FILE не найден"
    echo "Использование: $0 [путь_к_лог_файлу]"
    exit 1
fi

echo "Анализируем: $LOG_FILE"

TOTAL_LINES=$(wc -l < "$LOG_FILE")
echo "Всего строк в логе: $TOTAL_LINES"

echo ""
echo "=== ОШИБКИ И ПРЕДУПРЕЖДЕНИЯ ==="
ERRORS=$(grep -i -E "ERROR|FAIL|WARN" "$LOG_FILE")
if [ -n "$ERRORS" ]; then
    echo "$ERRORS" | head -10
else
    echo "Ошибок не обнаружено"
fi

echo ""
echo "=== СТАТИСТИКА СООБЩЕНИЙ ==="
echo "PING отправлено: $(grep -c "Отправляю PING" "$LOG_FILE")"
echo "PONG получено: $(grep -c "Получил PONG" "$LOG_FILE")"
echo "Успешных циклов: $(grep -c "PASS:" "$LOG_FILE")"
echo "Неудачных циклов: $(grep -c "FAIL:" "$LOG_FILE")"

echo ""
echo "=== ВРЕМЕННОЙ АНАЛИЗ ==="
START_TIME=$(grep "Тест начат:" "$LOG_FILE" | head -1)
END_TIME=$(grep "Тест завершен:" "$LOG_FILE" | tail -1)
echo "Начало теста: $START_TIME"
echo "Окончание теста: $END_TIME"

echo ""
echo "=== ПРОВЕРКА ЦЕЛОСТНОСТИ ==="
if grep -q "ОБЩИЙ РЕЗУЛЬТАТ: PASSED" "$LOG_FILE"; then
    echo "✅ Тест завершен успешно"
else
    echo "❌ Тест завершен с ошибками"
fi
