#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <csignal>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "../../include/protocol.h"
#include "../../include/session.h"

// Глобальные переменные
std::ofstream log_file("logs/server.log", std::ios::app);
std::mutex log_mutex;
volatile bool running = true;
std::vector<std::thread> client_threads;
std::mutex threads_mutex;
int listen_sock = -1;


// Функция логирования ошибок
void log_error(const std::string& msg) {
    std::lock_guard<std::mutex> lock(log_mutex);
    log_file << msg << std::endl;
}

// Обновление времени активного соединения
void update_last_active(Session& session) {
    // Предполагается, что в Session есть поле last_active типа std::chrono::steady_clock::time_point
    session.last_active = std::chrono::steady_clock::now();
}

// Настройка консоли (в Linux обычно не требуется)
void setup_console() {
    // Можно оставить пустым или вывести сообщение
    std::cout << "Настройка консоли завершена.\n";
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
        memset(buffer, 0, sizeof(buffer));
        int received = recv(session.socket, buffer, MAX_MSG_LEN - 1, 0);
        if (received == -1) {
            int err = errno;
            if (err == EINTR || err == EWOULDBLOCK || err == EAGAIN) {
                continue;
            }
            log_error("recv() не удался: " + std::string(strerror(err)));
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
            if (sent == -1) {
                log_error("send() не удался: " + std::string(strerror(errno)));
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
            // Открываем файл логов
            log_error("history send");
            std::ifstream log_stream("logs/server.log");
            if (log_stream.is_open()) {
                // Считываем весь файл в строку
                std::string log_content((std::istreambuf_iterator<char>(log_stream)),
                                        std::istreambuf_iterator<char>());
                log_stream.close();

                // Логируем отправку истории
                log_error("Отправка истории: " + log_content);

                // Отправляем содержимое клиенту
                send(session.socket, log_content.c_str(), log_content.size(), 0);
            } else {
                // Если не удалось открыть лог-файл
                const char* err_msg = "Ошибка при чтении истории";
                log_error(err_msg);
                send(session.socket, err_msg, strlen(err_msg), 0);
            }
        } else {
            log_error("Получено неизвестное сообщение: " + std::string(buffer));
        }
    }
    close(session.socket);
}

// Цикл приема новых соединений
void accept_loop(int listen_sock) {
    while (running) {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sock = accept(listen_sock, (sockaddr*)&client_addr, &client_addr_len);
        if (client_sock == -1) {
            if (running) {
                log_error("accept() не удался: " + std::string(strerror(errno)));
            }
            break;
        }

        // Получаем имя клиента
        char name_buffer[256];
        memset(name_buffer, 0, sizeof(name_buffer));
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
        session.last_active = std::chrono::steady_clock::now();

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


    // Создаем сокет
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        close(listen_sock);
        return 1;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(listen_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(listen_sock);
        return 1;
    }

    if (listen(listen_sock, SOMAXCONN) == -1) {
        perror("listen");
        close(listen_sock);
        return 1;
    }

    std::cout << "Сервер запущен на порту " << port << ". Ждем подключений...\n";

    std::thread acceptThread(accept_loop, listen_sock);

    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Завершение работы
    close(listen_sock);
    acceptThread.join();

    {
        std::lock_guard<std::mutex> lock(threads_mutex);
        for (auto& t : client_threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    std::cout << "Сервер завершил работу.\n";
    return 0;
}