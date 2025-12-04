cmake_minimum_required(VERSION 3.10)
project(lw2_arche)

set(CMAKE_CXX_STANDARD 11)

# Клиент
add_executable(client src/client/client.cpp)

# Сервер
add_executable(server src/server/server.cpp)
