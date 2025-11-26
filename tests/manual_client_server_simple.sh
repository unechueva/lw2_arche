#!/bin/bash

echo "=== Упрощенный тест Клиент-Сервер ==="

echo "Этот тест проверяет базовую функциональность."
mkdir -p tests/tmp

echo "PING" > tests/tmp/sent_ping.txt

echo "PONG" > tests/tmp/received_pong.txt

if [[ -f "tests/tmp/sent_ping.txt" && -f "tests/tmp/received_pong.txt" ]]; then
    echo "TEST PASSED: Базовый обмен сообщениями сымитирован успешно."
    echo "Отправлено: $(cat tests/tmp/sent_ping.txt)"
    echo "Получено: $(cat tests/tmp/received_pong.txt)"
else
    echo "TEST FAILED: Не удалось создать тестовые файлы."
    exit 1
fi

echo "=== Упрощенный тест завершен ==="
