#pragma once
#include <SQLiteCpp/SQLiteCpp.h>
#include <optional>
#include <vector>
#include <mutex>
#include "models/Card.h"
#include "models/Folder.h"
#include "models/User.h"
#include "models/UserSession.h"

class DatabaseAccess
{
public:
    DatabaseAccess(const std::string &dbPath);


    bool insertSessionToken(int userId, const std::string &token, const std::string &expirationDateTime, const std::string &tokenRefresh, const std::string &expirationDateTimeRefresh);
    std::optional<UserSession> getSessionByToken(const std::string &token, bool isTokenRefresh = false);
    bool deleteSessionByToken(const std::string &token);
    bool updateSessionToken(const UserSession &session);
    bool invalidateUserRefreshTokens(int userId);

    std::optional<User> createUser(User& user);
    std::optional<User> getUserByUsername(const std::string &username);
    std::optional<User> getUserByGoogleId(const std::string &googleId);

    std::optional<Card> insertCard(const Card &card);
    std::optional<Card> getCard(int cardId, int userId, int folderId);
    bool updateCard(const Card &card);
    bool deleteCard(int cardId, int userId, int folderId);
    std::vector<Card> getAllCards(int userId, int folderId);

    std::optional<Folder> insertFolder(const Folder &folder);
    std::optional<Folder> getFolder(int folderId, int userId);
    bool updateFolder(const Folder &folder);
    bool deleteFolder(int folderId, int userId);
    std::vector<Folder> getAllFolders(int userId);

private:
    void initializeDatabase();
    std::unique_ptr<SQLite::Database> db;
    std::mutex dbMutex;
    bool folderExists(int folderId);
};