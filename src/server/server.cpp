#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <csignal>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

#include "../../include/protocol.h"
#include "../../include/session.h"

#pragma comment(lib, "ws2_32.lib")

std::ofstream log_file("logs/server.log", std::ios::app);
std::mutex log_mutex;
volatile bool running = true;
std::vector<std::thread> client_threads;
std::mutex threads_mutex;
SOCKET listen_sock = INVALID_SOCKET;

// Функция логирования ошибок
void log_error(const std::string& msg) {
    std::lock_guard<std::mutex> lock(log_mutex);
    log_file << msg << std::endl;
}

// Настройка консоли
void setup_console() {
    system("chcp 65001");
    setlocale(LC_ALL, "ru_RU.UTF-8");
}

// Обработчик сигнала завершения
BOOL WINAPI console_handler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        {
            std::lock_guard<std::mutex> lock(log_mutex);
            std::cerr << "Получен сигнал SIGINT, завершение...\n";
        }
        running = false;
        if (listen_sock != INVALID_SOCKET) {
            closesocket(listen_sock);
        }
        return TRUE;
    }
    return FALSE;
}

// Инициализация Winsock
void init_winsock() {
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        log_error("Ошибка WSAStartup: " + std::to_string(res));
        exit(1);
    }
}

// Очистка Winsock
void cleanup_winsock() {
    WSACleanup();
}

// Обработка клиента
void handle_client(Session session) {

    log_error("Обработка клиента " + session.client_name);
    char buffer[MAX_MSG_LEN];
    while (running) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - session.last_active).count();
        if (duration > 60) {
            log_error("Таймаут соединения с " + session.client_name + ", закрытие");
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        ZeroMemory(buffer, sizeof(buffer));
        int received = recv(session.socket, buffer, MAX_MSG_LEN - 1, 0);
        if (received == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err == WSAEINTR || err == WSAEWOULDBLOCK) {
                continue;
            }
            log_error("recv() не удался: " + std::to_string(err));
            break;
        }
        if (received == 0) {
            log_error("Клиент " + session.client_name + " отключился");
            break;
        }
        buffer[received] = '\0';
        update_last_active(session);

        // Обработка команд
        if (strcmp(buffer, MSG_PING) == 0) {
            log_error("Ping");
            int sent = send(session.socket, MSG_PONG, strlen(MSG_PONG), 0);
            if (sent == SOCKET_ERROR) {
                log_error("send() не удался: " + std::to_string(WSAGetLastError()));
                break;
            }
        } else if (strncmp(buffer, "QUIT", 4) == 0) {
            log_error("Quit");
            break;
        } else if (strncmp(buffer, "MSG ", 4) == 0) {
            const char* ack = "Message received";
            log_error(std::string(buffer));
            send(session.socket, ack, strlen(ack), 0);
        } else if (strncmp(buffer, "HIST ", 5) == 0) {
            const char* hist = "No history";
            log_error(hist);
            send(session.socket, hist, strlen(hist), 0);
        } else {
            log_error("Получено неизвестное сообщение: " + std::string(buffer));
        }
    }
    closesocket(session.socket);
}

// Цикл приема новых соединений
void accept_loop(SOCKET listen_sock) {
    while (running) {
        sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        SOCKET client_sock = accept(listen_sock, (sockaddr*)&client_addr, &client_addr_len);
        if (client_sock == INVALID_SOCKET) {
            if (running) {
                log_error("accept() не удался: " + std::to_string(WSAGetLastError()));
            }
            break;
        }

        // Получаем имя клиента
        char name_buffer[256];
        ZeroMemory(name_buffer, sizeof(name_buffer));
        int received = recv(client_sock, name_buffer, sizeof(name_buffer) - 1, 0);
        std::string client_name = "неизвестен";

        if (received > 0) {
            name_buffer[received] = '\0';
            if (strncmp(name_buffer, "NAME ", 5) == 0) {
                client_name = std::string(name_buffer + 5);
                size_t pos = client_name.find('\n');
                if (pos != std::string::npos) {
                    client_name.erase(pos);
                }
            }
        }

        // Создаем сессию
        Session session;
        session.socket = client_sock;
        session.client_name = client_name;
        update_last_active(session);

        log_error("Клиент " + session.client_name + " подключен");
        std::lock_guard<std::mutex> lock(threads_mutex);
        client_threads.emplace_back(std::thread(handle_client, session));
    }
}

int main(int argc, char* argv[]) {
    setup_console();
    if (argc < 2) {
        std::cerr << "Использование: " << argv[0] << " <порт>\n";
        return 1;
    }
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        std::cerr << "Некорректный порт.\n";
        return 1;
    }
    if (!SetConsoleCtrlHandler(console_handler, TRUE)) {
        std::cerr << "Не удалось установить обработчик сигнала.\n";
        return 1;
    }

    init_winsock();

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET) {
        log_error("Ошибка socket(): " + std::to_string(WSAGetLastError()));
        cleanup_winsock();
        return 1;
    }

    BOOL opt = TRUE;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in server_addr;
    ZeroMemory(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(listen_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        log_error("Ошибка bind(): " + std::to_string(WSAGetLastError()));
        closesocket(listen_sock);
        cleanup_winsock();
        return 1;
    }

    if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) {
        log_error("Ошибка listen(): " + std::to_string(WSAGetLastError()));
        closesocket(listen_sock);
        cleanup_winsock();
        return 1;
    }

    std::cout << "Сервер запущен на порту " << port << ". Ждем подключений...\n";

    std::thread acceptThread(accept_loop, listen_sock);
    
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Завершение работы
    closesocket(listen_sock);
    acceptThread.join();

    {
        std::lock_guard<std::mutex> lock(threads_mutex);
        for (auto& t : client_threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    cleanup_winsock();
    std::cout << "Сервер завершил работу.\n";
    return 0;
}