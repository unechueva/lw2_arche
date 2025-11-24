#include <string>
#include <cstring>
#include <iostream>
#include <winsock2.h>
#include "../../include/protocol.h"

int parse_command(const std::string& input, std::string& command, std::string& args) {
    size_t pos = input.find(' ');
    if (pos == std::string::npos) {
        command = input;
        args = "";
    } else {
        command = input.substr(0, pos);
        args = input.substr(pos + 1);
    }
    return 0;
}

int handle_command(const std::string& command, const std::string& args, const std::string& name, SOCKET sock) {
    if (command == "ping") {
        if (send(sock, MSG_PING, strlen(MSG_PING), 0) == SOCKET_ERROR) {
            return -1;
        }
    } else if (command == "msg") {
        std::string msg = "MSG " + name + " " + args + "\n";
        if (send(sock, msg.c_str(), msg.length(), 0) == SOCKET_ERROR) {
            return -1;
        }
    } else if (command == "hist") {
        std::string hist = "HIST " + args + "\n";
        if (send(sock, hist.c_str(), hist.length(), 0) == SOCKET_ERROR) {
            return -1;
        }
    } else if (command == "quit") {
        std::string quit = "QUIT\n";
        send(sock, quit.c_str(), quit.length(), 0);
        return 1;
    } else {
        std::cerr << "Неизвестная команда: " << command << "\n";
        return 0;
    }
    return 0;
}
