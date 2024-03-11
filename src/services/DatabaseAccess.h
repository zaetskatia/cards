#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <optional>
#include <vector>
#include <mutex>
#include "models/Card.h"
#include "models/Folder.h"

class DatabaseAccess
{
public:
    DatabaseAccess(const std::string &dbPath);

    std::optional<Card> insertCard(const Card &card);
    std::optional<Card> getCard(int cardId, std::string clientId, int folderId);
    bool updateCard(const Card &card);
    bool deleteCard(int cardId, std::string clientId, int folderId);
    std::vector<Card> getAllCards(std::string clientId, int folderId);

    std::optional<Folder> insertFolder(const Folder &folder);
    std::optional<Folder> getFolder(int folderId, std::string clientId);
    bool updateFolder(const Folder &folder);
    bool deleteFolder(int folderId, std::string clientId);
    std::vector<Folder> getAllFolders(std::string clientId);

private:
    void initializeDatabase();
    std::unique_ptr<SQLite::Database> db;
    std::mutex dbMutex;
};