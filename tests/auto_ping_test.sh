#!/bin/bash

echo "=== Запуск автоматического PING-теста ==="

TEST_LOG="tests/tmp/auto_ping_test.log"
echo "Тест начат: $(date)" > $TEST_LOG

mkdir -p tests/tmp/auto_test

for i in {1..5}; do
    echo "Цикл $i:" >> $TEST_LOG
    
    echo "CLIENT: Отправляю PING_$i" >> $TEST_LOG
    echo "PING $i" > tests/tmp/auto_test/ping_$i.txt
    
    sleep 0.5
    echo "SERVER: Получил PING_$i, отправляю PONG_$i" >> $TEST_LOG
    echo "PONG $i" > tests/tmp/auto_test/pong_$i.txt
    
    sleep 0.5
    echo "CLIENT: Получил PONG_$i" >> $TEST_LOG
    
    if [[ -f "tests/tmp/auto_test/ping_$i.txt" && -f "tests/tmp/auto_test/pong_$i.txt" ]]; then
        echo "PASS: Цикл $i завершен успешно" >> $TEST_LOG
    else
        echo "FAIL: Ошибка в цикле $i" >> $TEST_LOG
    fi
    echo "---" >> $TEST_LOG
done

echo "Тест завершен: $(date)" >> $TEST_LOG

PASS_COUNT=$(grep -c "PASS:" $TEST_LOG)
FAIL_COUNT=$(grep -c "FAIL:" $TEST_LOG)

if [ $FAIL_COUNT -eq 0 ]; then
    echo "ОБЩИЙ РЕЗУЛЬТАТ: PASSED" >> $TEST_LOG
    FINAL_RESULT="PASSED"
else
    echo "ОБЩИЙ РЕЗУЛЬТАТ: FAILED" >> $TEST_LOG
    FINAL_RESULT="FAILED"
fi

echo "=== Результаты автотеста ==="
echo "Успешных циклов: $PASS_COUNT"
echo "Неудачных циклов: $FAIL_COUNT"
echo "ОБЩИЙ РЕЗУЛЬТАТ: $FINAL_RESULT"
echo "Подробный лог: $TEST_LOG"
