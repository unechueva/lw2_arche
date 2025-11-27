#ifndef SESSION_H
#define SESSION_H

#include <winsock2.h>
#include <chrono>
#include <string>

struct Session {
    SOCKET socket;
    std::chrono::steady_clock::time_point last_active;
    std::string client_name; // добавлено
    std::string state; // уже есть
};

void update_last_active(Session& session);

#endif