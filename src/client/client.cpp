#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>
#include <cstdlib>


#include "../../include/protocol.h"

#pragma comment(lib, "ws2_32.lib")


void setup_console() {
    system("chcp 65001"); 
    setlocale(LC_ALL, "ru_RU.UTF-8");
}

int main(int argc, char* argv[]) {
    setup_console();


    if (argc < 3) {
        std::cout << "Использование: " << argv[0] << " <хост> <порт> [-n <число>] [-v]\n";
        return 1;
    }

    const char* host = argv[1];
    const char* port_str = argv[2];

    int count = 1;
    bool verbose = false;

    for (int i = 3; i < argc; ++i) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            count = atoi(argv[++i]);
            if (count <= 0) {
                std::cerr << "Ошибка: число должно быть положительным.\n";
                return 1;
            }
        }
        else if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        }
        else {
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
        std::cerr << "Ошибка: socket() не удался. Код ошибки: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }


    struct addrinfo hints, * result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(host, port_str, &hints, &result);
    if (status != 0) {
        std::cerr << "Ошибка: getaddrinfo() не удался. Код ошибки: " << status << "\n";
        closesocket(sock);
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

    freeaddrinfo(result);
    std::cout << "Подключено к " << host << ":" << port_str << "\n";

    for (int i = 0; i < count; ++i) {
        if (send(sock, MSG_PING, strlen(MSG_PING), 0) == SOCKET_ERROR) {
            std::cerr << "Ошибка: send() не удался. Код ошибки: " << WSAGetLastError() << "\n";
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        if (verbose) std::cout << "отправлено " << MSG_PING;

        char buffer[MAX_MSG_LEN];
        int received = recv(sock, buffer, MAX_MSG_LEN - 1, 0);
        if (received == SOCKET_ERROR) {
            std::cerr << "Ошибка: recv() не удался. Код ошибки: " << WSAGetLastError() << "\n";
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        buffer[received] = '\0';

        if (verbose) std::cout << "получено " << buffer;


        if (strncmp(buffer, MSG_PONG, strlen(MSG_PONG)) != 0) {
            std::cerr << "Ошибка: ожидался PONG, получен: " << buffer << "\n";
            closesocket(sock);
            WSACleanup();
            return 1;
        }

        std::cout << "Пинг #" << (i + 1) << " OK\n";
    }

    closesocket(sock);
    WSACleanup();
    std::cout << "Все пинги отправлены. Выход.\n";
    return 0;
}
