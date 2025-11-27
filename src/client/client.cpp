#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../../include/protocol.h"

int parse_command(const std::string& input, std::string& command, std::string& args);
int handle_command(const std::string& command, const std::string& args, const std::string& name, SOCKET sock);

#pragma comment(lib, "ws2_32.lib")

void setup_console() {
    system("chcp 65001");
    setlocale(LC_ALL, "ru_RU.UTF-8");
}

int reconnect(SOCKET& sock, struct addrinfo* result) {
    int attempts = 0;
    int backoff[] = {1, 2, 4};
    while (attempts < 3) {
        std::this_thread::sleep_for(std::chrono::seconds(backoff[attempts]));
        SOCKET new_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (new_sock == INVALID_SOCKET) {
            attempts++;
            continue;
        }
        if (connect(new_sock, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR) {
            closesocket(new_sock);
            attempts++;
            continue;
        }
        closesocket(sock);
        sock = new_sock;
        return 0;
    }
    return -1;
}

void receive_messages(SOCKET sock, bool& running) {
    char buffer[MAX_MSG_LEN];
    while (running) {
        int received = recv(sock, buffer, MAX_MSG_LEN - 1, 0);
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

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка: WSAStartup не удался.\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Ошибка: socket() не удался.\n";
        WSACleanup();
        return 1;
    }

    struct addrinfo hints, *result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host, port_str, &hints, &result);
    if (status != 0) {
        std::cerr << "Ошибка: getaddrinfo() не удался.\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (connect(sock, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "Ошибка: подключение не удалось.\n";
        freeaddrinfo(result);
        closesocket(sock);
        WSACleanup();
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

    closesocket(sock);
    freeaddrinfo(result);
    WSACleanup();
    std::cout << "Выход.\n";
    return 0;
}
