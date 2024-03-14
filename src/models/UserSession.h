#pragma once
#include <string>

struct UserSession {
    std::string token;
    int userId;
    std::string expiration;
};