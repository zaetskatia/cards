#pragma once
#include <string>

struct User
{
    int id{};
    std::string googleId{};
    std::string name{};
    std::string passwordHash{};
};

struct GoogleUserInfo
{
    std::string googleId{};
    std::string name{};
};
