#ifndef SESSION_H
#define SESSION_H

#include <sys/socket.h> // Для типа socket в Linux
#include <chrono>
#include <string>

struct Session {
    int socket; // в Linux сокет — это int
    std::chrono::steady_clock::time_point last_active;
    std::string client_name; // добавлено
    std::string state; // уже есть
};

// Объявление функции
//void update_last_active(Session& session);

#endif // SESSION_H