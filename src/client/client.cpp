#include <iostream>
#include <string>
#include <cstring>
#include <winsock2.h>
#include <ws2tcpip.h>

#define MAX_MSG_LEN 1024
#define MSG_PING "PING"
#define MSG_PONG "PONG"

// Предположим, parse_command и handle_command объявлены где-то еще
// int parse_command(const std::string& input, std::string& command, std::string& args);
// int handle_command(const std::string& command, const std::string& args, const std::string& name, SOCKET sock);
// int reconnect(SOCKET& sock, addrinfo* result);

int main(int argc, char* argv[]) {
    const char* host = "127.0.0.1";  // пример
    const char* port_str = "12345";   // пример
    const char* name = "user";        // пример
    int count = 5;                     // количество PING
    bool verbose = true;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "Ошибка: WSAStartup не удался.\n";
        return 1;
    }

    addrinfo hints{}, *result = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port_str, &hints, &result) != 0) {
        std::cerr << "Ошибка: getaddrinfo не удался.\n";
        WSACleanup();
        return 1;
    }

    SOCKET sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Ошибка: socket() не удался.\n";
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    if (connect(sock, result->ai_addr, result->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "Ошибка: connect() не удался. Код ошибки: " << WSAGetLastError() << "\n";
        freeaddrinfo(result);
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Подключено к " << host << ":" << port_str << "\n";

    // Базовый PING
    for (int i = 0; i < count; ++i) {
        if (send(sock, MSG_PING, strlen(MSG_PING), 0) == SOCKET_ERROR) {
            std::cerr << "Ошибка: send() не удался. Код ошибки: " << WSAGetLastError() << "\n";
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        if (verbose) std::cout << "отправлено " << MSG_PING << "\n";

        char buffer[MAX_MSG_LEN];
        int received = recv(sock, buffer, MAX_MSG_LEN - 1, 0);
        if (received == SOCKET_ERROR) {
            std::cerr << "Ошибка: recv() не удался. Код ошибки: " << WSAGetLastError() << "\n";
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        buffer[received] = '\0';

        if (verbose) std::cout << "получено " << buffer << "\n";

        if (strncmp(buffer, MSG_PONG, strlen(MSG_PONG)) != 0) {
            std::cerr << "Ошибка: ожидался PONG, получен: " << buffer << "\n";
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        std::cout << "Пинг #" << (i + 1) << " OK\n";
    }

    // Интерактивный режим
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

    closesocket(sock);
    freeaddrinfo(result);
    WSACleanup();
    std::cout << "Выход.\n";
    return 0;
}