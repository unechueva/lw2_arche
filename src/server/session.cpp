#include "../../include/session.h"

void update_last_active(Session& session) {
    session.last_active = std::chrono::steady_clock::now();
}