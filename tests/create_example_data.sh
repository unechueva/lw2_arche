#!/bin/bash

echo "=== Генерация тестовых данных ==="

mkdir -p tests/tmp/input tests/tmp/output tests/tmp/logs

echo "PING 123456789" > tests/tmp/input/client_msg_1.txt
echo "MSG Hello, Server!" > tests/tmp/input/client_msg_2.txt
echo "ERROR Test error" > tests/tmp/input/client_msg_3.txt

echo "PONG 123456789" > tests/tmp/output/server_msg_1.txt
echo "MSG Hello, Client!" > tests/tmp/output/server_msg_2.txt
echo "ERROR Unknown command" > tests/tmp/output/server_msg_3.txt

echo "$(date) - CLIENT: Connected to localhost:5555" > tests/tmp/logs/client.log
echo "$(date) - CLIENT: Sent PING 123456789" >> tests/tmp/logs/client.log
echo "$(date) - SERVER: Started on port 5555" > tests/tmp/logs/server.log
echo "$(date) - SERVER: Received PING, sent PONG 123456789" >> tests/tmp/logs/server.log

echo "Данные созданы в tests/tmp/"
echo "Содержимое tests/tmp/:"
ls -la tests/tmp/
