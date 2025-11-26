#!/bin/bash

echo "=== Генерация тестовых данных ==="

# Создаем необходимые папки
mkdir -p tests/tmp/input tests/tmp/output tests/tmp/logs

# Создаем пример входящих сообщений
echo "PING 123456789" > tests/tmp/input/client_msg_1.txt
echo "MSG Hello, Server!" > tests/tmp/input/client_msg_2.txt
echo "ERROR Test error" > tests/tmp/input/client_msg_3.txt

# Создаем пример исходящих сообщений
echo "PONG 123456789" > tests/tmp/output/server_msg_1.txt
echo "MSG Hello, Client!" > tests/tmp/output/server_msg_2.txt
echo "ERROR Unknown command" > tests/tmp/output/server_msg_3.txt

# Создаем примеры лог-файлов
echo "$(date) - CLIENT: Connected to localhost:5555" > tests/tmp/logs/client.log
echo "$(date) - CLIENT: Sent PING 123456789" >> tests/tmp/logs/client.log
echo "$(date) - SERVER: Started on port 5555" > tests/tmp/logs/server.log
echo "$(date) - SERVER: Received PING, sent PONG 123456789" >> tests/tmp/logs/server.log

echo "Данные созданы в tests/tmp/"
echo "Содержимое tests/tmp/:"
ls -la tests/tmp/
