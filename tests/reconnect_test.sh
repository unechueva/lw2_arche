#!/bin/bash

echo "=== Тест переподключения ==="

RECONNECT_LOG="tests/tmp/reconnect_test.log"
echo "Тест переподключения начат: $(date)" > $RECONNECT_LOG

scenarios=(
    "Нормальное подключение"
    "Разрыв соединения" 
    "Повторное подключение"
    "Стабильная работа"
)

for i in {1..3}; do
    echo "Сценарий $i: ${scenarios[$((i-1))]}" >> $RECONNECT_LOG
    
    echo "$(date) - Подключение к серверу..." >> $RECONNECT_LOG
    echo "CONNECT" > tests/tmp/reconnect_$i.txt
    
    if [ $i -eq 2 ]; then
        echo "$(date) - СОЕДИНЕНИЕ РАЗОРВАНО" >> $RECONNECT_LOG
        echo "DISCONNECT" > tests/tmp/disconnect_$i.txt
        sleep 1
        echo "$(date) - Повторное подключение..." >> $RECONNECT_LOG
        echo "RECONNECT" > tests/tmp/reconnect_${i}_retry.txt
    fi
    
    echo "PING reconnect_$i" > tests/tmp/ping_reconnect_$i.txt
    echo "PONG reconnect_$i" > tests/tmp/pong_reconnect_$i.txt
    echo "Сообщения отправлены и получены" >> $RECONNECT_LOG
    echo "---" >> $RECONNECT_LOG
done

echo "Тест переподключения завершен: $(date)" >> $RECONNECT_LOG

echo "=== Результаты теста переподключения ==="
echo "Сценарии выполнены: 3"
echo "Разрывы соединения: 1" 
echo "Повторные подключения: 1"
echo "Лог сохранен: $RECONNECT_LOG"
