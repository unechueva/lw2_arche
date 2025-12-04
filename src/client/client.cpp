#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "include/protocol.h"

// Предположим, у вас есть функции parse_command и handle_command, как и раньше.

int parse_command(const std::string& input, std::string& command, std::string& args);
int handle_command(const std::string& command, const std::string& args, const std::string& name, int sock);

void setup_console() {
    // В Linux можно настроить локаль, если нужно
    setlocale(LC_ALL, "ru_RU.UTF-8");
}

int reconnect(int& sock, struct addrinfo* result) {
    int attempts = 0;
    int backoff[] = {1, 2, 4};
    while (attempts < 3) {
        std::this_thread::sleep_for(std::chrono::seconds(backoff[attempts]));
        int new_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (new_sock == -1) {
            attempts++;
            continue;
        }
        if (connect(new_sock, result->ai_addr, result->ai_addrlen) == -1) {
            close(new_sock);
            attempts++;
            continue;
        }
        close(sock);
        sock = new_sock;
        return 0;
    }
    return -1;
}

void receive_messages(int sock, bool& running) {
    const int MAX_MSG_LEN = 1024;
    char buffer[MAX_MSG_LEN];
    while (running) {
        ssize_t received = recv(sock, buffer, MAX_MSG_LEN - 1, 0);
        if (received <= 0) {
            if (running) {
                std::cerr << "\n[Сервер закрыл соединение]\n> ";
            }
            break;
        }
        buffer[received] = '\0';
        std::cout << "\n[Сообщение от сервера]: " << buffer << "> " << std::flush;
    }
}

int main(int argc, char* argv[]) {
    setup_console();

    if (argc < 3) {
        std::cout << "Использование: " << argv[0] << " <хост> <порт> [-v] [--name <имя>]\n";
        return 1;
    }

    const char* host = argv[1];
    const char* port_str = argv[2];
    bool verbose = false;
    std::string name = "default";

    for (int i = 3; i < argc; ++i) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "--name") == 0 && i + 1 < argc) {
            name = argv[++i];
        } else {
            std::cerr << "Неизвестный параметр: " << argv[i] << "\n";
            return 1;
        }
    }

    struct addrinfo hints{}, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host, port_str, &hints, &result);
    if (status != 0) {
        std::cerr << "Ошибка: getaddrinfo() не удался: " << gai_strerror(status) << "\n";
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Ошибка: socket() не удался.\n";
        freeaddrinfo(result);
        return 1;
    }

    if (connect(sock, result->ai_addr, result->ai_addrlen) == -1) {
        std::cerr << "Ошибка: подключение не удалось.\n";
        close(sock);
        freeaddrinfo(result);
        return 1;
    }

    std::cout << "Подключено к " << host << ":" << port_str << "\n";

    bool running = true;
    std::thread receiver_thread(receive_messages, sock, std::ref(running));

    std::string input;
    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);
        if (input.empty()) continue;

        std::string command, args;
        parse_command(input, command, args);

        int res = handle_command(command, args, name, sock);
        if (res == 1) break;
        if (res == -1) {
            std::cerr << "Соединение потеряно. Попытка переподключения...\n";
            if (reconnect(sock, result) == 0) {
                std::cout << "Переподключено.\n";
            } else {
                std::cerr << "Не удалось восстановить соединение. Выход.\n";
                break;
            }
        }
    }

    running = false;
    if (receiver_thread.joinable()) {
        receiver_thread.join();
    }

    close(sock);
    freeaddrinfo(result);
    std::cout << "Выход.\n";
    return 0;
}