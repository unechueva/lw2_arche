#!/bin/bash

echo "=== Запуск автоматического MSG-теста ==="

TEST_LOG="tests/tmp/auto_msg_test.log"
echo "Тест начат: $(date)" > $TEST_LOG

# Имитируем отправку различных сообщений
messages=("PING" "MSG Hello" "MSG Test" "ERROR" "PING 123")

for msg in "${messages[@]}"; do
    echo "Отправка: $msg" >> $TEST_LOG
    # Имитируем отправку сообщения
    echo "$msg" > "tests/tmp/sent_${msg// /_}.txt"

    # Имитируем ответ сервера
    if [[ "$msg" == PING* ]]; then
        response="PONG ${msg#PING}"
    elif [[ "$msg" == MSG* ]]; then
        response="MSG Received"
    else
        response="ERROR Unknown command"
    fi
    
    echo "Ответ: $response" >> $TEST_LOG
    echo "$response" > "tests/tmp/received_${msg// /_}.txt"
    echo "---" >> $TEST_LOG
done

echo "Тест завершен: $(date)" >> $TEST_LOG
echo "=== Автоматический MSG-тест завершен ==="
echo "Лог сохранен: $TEST_LOG"
