#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <csignal>
#include <cstring>


#include "../../include/protocol.h"

#pragma comment(lib, "ws2_32.lib")

void setup_console() {
    system("chcp 65001"); 
    setlocale(LC_ALL, "ru_RU.UTF-8");
}

volatile bool running = true;
SOCKET listen_sock = INVALID_SOCKET;

BOOL WINAPI console_handler(DWORD signal) {
    if (signal == CTRL_C_EVENT) {
        std::cerr << "Получен сигнал SIGINT, завершение...\n";
        running = false;
        if (listen_sock != INVALID_SOCKET) {
            closesocket(listen_sock);
        }
        return TRUE;
    }
    return FALSE;
}

void init_winsock() {
    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        std::cerr << "Ошибка WSAStartup: " << res << "\n";
        exit(1);
    }
}

void cleanup_winsock() {
    WSACleanup();
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
        std::cerr << "Ошибка socket(): " << WSAGetLastError() << "\n";
        cleanup_winsock();
        return 1;
    }

    sockaddr_in server_addr;
    ZeroMemory(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; 
    server_addr.sin_port = htons(port);

    if (bind(listen_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка bind(): " << WSAGetLastError() << "\n";
        closesocket(listen_sock);
        cleanup_winsock();
        return 1;
    }

    if (listen(listen_sock, 1) == SOCKET_ERROR) {
        std::cerr << "Ошибка listen(): " << WSAGetLastError() << "\n";
        closesocket(listen_sock);
        cleanup_winsock();
        return 1;
    }

    std::cout << "Сервер запущен на порту " << port << ". Ждем подключений...\n";

    SOCKET client_sock = INVALID_SOCKET;
    sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    client_sock = accept(listen_sock, (sockaddr*)&client_addr, &client_addr_len);
    if (client_sock == INVALID_SOCKET) {
        std::cerr << "accept() не удался: " << WSAGetLastError() << "\n";
        closesocket(listen_sock);
        cleanup_winsock();
        return 1;
    }

    std::cout << "Клиент подключен.\n";

    char buffer[MAX_MSG_LEN];

    while (running) {
        ZeroMemory(buffer, sizeof(buffer));
        int received = recv(client_sock, buffer, MAX_MSG_LEN - 1, 0);
        if (received == SOCKET_ERROR) {
            std::cerr << "recv() не удался: " << WSAGetLastError() << "\n";
            break;
        }
        if (received == 0) {
            std::cout << "Клиент отключился.\n";
            break;
        }

        buffer[received] = '\0'; 


        if (strcmp(buffer, MSG_PING) == 0) {
            int sent = send(client_sock, MSG_PONG, strlen(MSG_PONG), 0);
            if (sent == SOCKET_ERROR) {
                std::cerr << "send() не удался: " << WSAGetLastError() << "\n";
                break;
            }
        }
        else {
            std::cerr << "Получено неизвестное сообщение: " << buffer << "\n";
        }
    }

    closesocket(client_sock);
    std::cout << "Клиент отключен.\n";

    closesocket(listen_sock);
    cleanup_winsock();

    return 0;
}