#pragma once

#include <string>

struct Card {
    int id;
    std::string clientId;
    int folderId = 1;
    std::string term;
    std::string translation;
};