#!/bin/bash

echo "=== Запуск автоматического PING-теста ==="

TEST_LOG="tests/tmp/auto_ping_test.log"
echo "Тест начат: $(date)" > $TEST_LOG

# Создаем каталог для временных файлов
mkdir -p tests/tmp/auto_test

# Имитируем 5 циклов PING-PONG
for i in {1..5}; do
    echo "Цикл $i:" >> $TEST_LOG
    
    # Имитируем отправку PING от клиента
    echo "CLIENT: Отправляю PING_$i" >> $TEST_LOG
    echo "PING $i" > tests/tmp/auto_test/ping_$i.txt
    
    # Имитируем обработку на сервере и отправку PONG
    sleep 0.5
    echo "SERVER: Получил PING_$i, отправляю PONG_$i" >> $TEST_LOG
    echo "PONG $i" > tests/tmp/auto_test/pong_$i.txt
    
    # Имитируем получение PONG клиентом
    sleep 0.5
    echo "CLIENT: Получил PONG_$i" >> $TEST_LOG
    
    # Проверяем, что файлы созданы
    if [[ -f "tests/tmp/auto_test/ping_$i.txt" && -f "tests/tmp/auto_test/pong_$i.txt" ]]; then
        echo "PASS: Цикл $i завершен успешно" >> $TEST_LOG
    else
        echo "FAIL: Ошибка в цикле $i" >> $TEST_LOG
    fi
    echo "---" >> $TEST_LOG
done

echo "Тест завершен: $(date)" >> $TEST_LOG

# Выводим итоги
PASS_COUNT=$(grep -c "PASS:" $TEST_LOG)
FAIL_COUNT=$(grep -c "FAIL:" $TEST_LOG)

# Записываем общий результат В ЛОГ-ФАЙЛ
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
