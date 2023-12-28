#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <optional>
#include <vector>
#include <mutex>
#include "Card.h"


class DatabaseAccess {
public:
    DatabaseAccess(const std::string& dbPath); // Constructor to initialize DB connection

    std::optional<Card> insertCard(const Card& card);
    std::optional<Card> getCard(int cardId, std::string clientId);
    bool updateCard(const Card& card);
    bool deleteCard(int cardId, std::string clientId);
    std::vector<Card> getAllCards(std::string clientId);

    // Add more functions for other operations

private:
    void initializeDatabase();
    std::unique_ptr<SQLite::Database> db;
    std::mutex dbMutex;
};