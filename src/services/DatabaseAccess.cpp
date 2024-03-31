#include "DatabaseAccess.h"
#include <iostream>

DatabaseAccess::DatabaseAccess(const std::string &dbPath)
{
    // Open the SQLite database (will be created if it doesn't exist)
    try
    {
        db = std::make_unique<SQLite::Database>(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        initializeDatabase();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
    }
}

void DatabaseAccess::initializeDatabase()
{
    const char *createCardsTableSql = R"(
        CREATE TABLE IF NOT EXISTS Cards (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            userId INTEGER,
            folderId INTEGER,
            term TEXT,
            translation TEXT
        )
    )";

    const char *createFoldersTableSql = R"(
        CREATE TABLE IF NOT EXISTS Folders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            userId INTEGER,
            name TEXT
        )
    )";

    const char *createUserTableSql = R"(
        CREATE TABLE IF NOT EXISTS Users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL
        )
    )";

    const char *createUserSessionTableSql = R"(
        CREATE TABLE IF NOT EXISTS UserSession (
            user_id INTEGER NOT NULL,
            token TEXT PRIMARY KEY, 
            expiration DATETIME NOT NULL,
            tokenRefresh TEXT, 
            expirationRefresh DATETIME NOT NULL,
            FOREIGN KEY (user_id) REFERENCES Users(id)
        )
    )";

    try
    {
        SQLite::Statement cardsQuery(*db, createCardsTableSql);
        cardsQuery.exec();

        SQLite::Statement foldersQuery(*db, createFoldersTableSql);
        foldersQuery.exec();

        SQLite::Statement createUserQuery(*db, createUserTableSql);
        createUserQuery.exec();

        SQLite::Statement createUserSessionQuery(*db, createUserSessionTableSql);
        createUserSessionQuery.exec();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to create table: " << e.what() << std::endl;
    }
}

std::optional<User> DatabaseAccess::getUserByUsername(const std::string &username)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT id, username, password_hash FROM Users WHERE username = ?");
        query.bind(1, username);

        if (query.executeStep())
        {
            User user;
            user.id = query.getColumn(0);
            user.name = query.getColumn(1).getText();
            user.passwordHash = query.getColumn(2).getText();
            return std::make_optional(user);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to retrieve user: " << e.what() << std::endl;
    }

    return std::nullopt;
}

std::optional<User> DatabaseAccess::createUser(const std::string &username, const std::string &passwordHash)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        SQLite::Statement query(*db, "INSERT INTO Users (username, password_hash) VALUES (?, ?)");
        query.bind(1, username);
        query.bind(2, passwordHash);
        query.exec();

        int lastId = db->getLastInsertRowid();
        transaction.commit();

        User newUser;
        newUser.id = lastId;
        newUser.name = username;
        newUser.passwordHash = passwordHash;
        return std::make_optional(newUser);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to create user: " << e.what() << std::endl;
        return std::nullopt;
    }
}

bool DatabaseAccess::insertSessionToken(int userId, const std::string &token, const std::string &expirationDateTime, const std::string &tokenRefresh, const std::string &expirationDateTimeRefresh)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        SQLite::Statement query(*db, "INSERT INTO UserSession (user_id, token, expiration, tokenRefresh, expirationRefresh) VALUES (?, ?, ?, ?, ?)");
        query.bind(1, userId);
        query.bind(2, token);
        query.bind(3, expirationDateTime);
        query.bind(4, tokenRefresh);
        query.bind(5, expirationDateTimeRefresh);
        query.exec();
        transaction.commit();
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception in insertSessionToken: " << e.what() << std::endl;
    }
    return false;
}

std::optional<UserSession> DatabaseAccess::getSessionByToken(const std::string &token, bool isTokenRefresh)
{
    std::string key = isTokenRefresh ? "tokenRefresh" : "token";
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT * FROM UserSession WHERE " + key + " = ?");
        query.bind(1, token);

        if (query.executeStep())
        {
            UserSession session;
            session.userId = query.getColumn(0).getInt();
            session.token = query.getColumn(1).getText();
            session.expiration = query.getColumn(2).getText();
            session.tokenRefresh = query.getColumn(3).getText();
            session.expirationRefresh = query.getColumn(4).getText();
            return session;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception in getSessionByToken: " << e.what() << std::endl;
    }
    return std::nullopt;
}

bool DatabaseAccess::updateSessionToken(const UserSession &session)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        SQLite::Statement query(*db, "UPDATE UserSession SET token = ?, expiration = ?, tokenRefresh = ?, expirationRefresh = ? WHERE user_id = ?");
        query.bind(1, session.token);
        query.bind(2, session.expiration);
        query.bind(3, session.tokenRefresh);
        query.bind(4, session.expirationRefresh);
        query.bind(5, session.userId);
        query.exec();

        transaction.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool DatabaseAccess::deleteSessionByToken(const std::string &token)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);
        SQLite::Statement query(*db, "DELETE FROM UserSession WHERE token = ?");
        query.bind(1, token);
        query.exec();
        transaction.commit();
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to delete session token: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseAccess::invalidateUserRefreshTokens(int userId) {
    try {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        // Prepare a SQL statement to delete the user's refresh tokens
        SQLite::Statement query(*db, "DELETE FROM UserSession WHERE user_id = ?");
        query.bind(1, userId);

        // Execute the query
        query.exec();
        transaction.commit();

        return true;
    } catch (const std::exception &e) {
        std::cerr << "Database exception in invalidateUserRefreshTokens: " << e.what() << std::endl;
        return false;
    }
}

std::optional<Card> DatabaseAccess::insertCard(const Card &card)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        SQLite::Statement query(*db, "INSERT INTO Cards (userId, folderId, term, translation) VALUES (?, ?, ?, ?)");
        query.bind(1, card.userId);
        query.bind(2, card.folderId);
        query.bind(3, card.term);
        query.bind(4, card.translation);
        query.exec();

        int lastId = db->getLastInsertRowid();
        transaction.commit();

        Card newCard = card;
        newCard.id = lastId;
        return std::make_optional(newCard);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<Card> DatabaseAccess::getCard(int cardId, int userId, int folderId)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT * FROM Cards WHERE id = ? AND userId = ? AND folderId = ?");

        query.bind(1, cardId);
        query.bind(2, userId);
        query.bind(3, folderId);

        if (query.executeStep())
        {
            Card card;
            card.id = query.getColumn(0);
            card.userId = query.getColumn(1);
            card.folderId = query.getColumn(2);
            card.term = query.getColumn(3).getText();
            card.translation = query.getColumn(4).getText();
            return std::make_optional(card);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
    };

    return std::nullopt;
}

std::vector<Card> DatabaseAccess::getAllCards(int userId, int folderId)
{
    std::vector<Card> cards;

    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT * FROM Cards WHERE userId = ? AND folderId = ?");

        query.bind(1, userId);
        query.bind(2, folderId);

        while (query.executeStep())
        {
            Card card;
            card.id = query.getColumn(0).getInt();
            card.userId = query.getColumn(1);
            card.folderId = query.getColumn(2).getInt();
            card.term = query.getColumn(3).getText();
            card.translation = query.getColumn(4).getText();
            cards.push_back(card);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
    }

    return cards;
}

bool DatabaseAccess::updateCard(const Card &card)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        SQLite::Statement query(*db, "UPDATE Cards SET folderId = ?, term = ?, translation = ? WHERE id = ? AND userId = ? AND folderId = ?");
        query.bind(1, card.folderId);
        query.bind(2, card.term);
        query.bind(3, card.translation);
        query.bind(4, card.id);
        query.bind(5, card.userId);
        query.bind(6, card.folderId);
        query.exec();

        transaction.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool DatabaseAccess::deleteCard(int cardId, int userId, int folderId)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);
        SQLite::Statement query(*db, "DELETE FROM Cards WHERE id = ? AND userId = ? AND folderId = ?");
        query.bind(1, cardId);
        query.bind(2, userId);
        query.bind(3, folderId);
        query.exec();
        transaction.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
        return false;
    }
    return true;
}

std::optional<Folder> DatabaseAccess::insertFolder(const Folder &folder)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        SQLite::Statement query(*db, "INSERT INTO Folders (userId, name) VALUES (?, ?)");
        query.bind(1, folder.userId);
        query.bind(2, folder.name);
        query.exec();

        int lastId = db->getLastInsertRowid();
        transaction.commit();

        Folder newFolder = folder;
        newFolder.id = lastId;
        return std::make_optional(newFolder);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::optional<Folder> DatabaseAccess::getFolder(int folderId, int userId)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT * FROM Folders WHERE id = ? AND userId = ?");
        query.bind(1, folderId);
        query.bind(2, userId);

        if (query.executeStep())
        {
            Folder folder;
            folder.id = query.getColumn(0);
            folder.userId = query.getColumn(1);
            folder.name = query.getColumn(2).getText();
            return std::make_optional(folder);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
    }

    return std::nullopt;
}

bool DatabaseAccess::updateFolder(const Folder &folder)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        SQLite::Statement query(*db, "UPDATE Folders SET name = ? WHERE id = ? AND userId = ?");
        query.bind(1, folder.name);
        query.bind(2, folder.id);
        query.bind(3, folder.userId);
        query.exec();

        transaction.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool DatabaseAccess::deleteFolder(int folderId, int userId)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        // First, delete all cards associated with the folder
        SQLite::Statement deleteCardsQuery(*db, "DELETE FROM Cards WHERE folderId = ? AND userId = ?");
        deleteCardsQuery.bind(1, folderId);
        deleteCardsQuery.bind(2, userId);
        deleteCardsQuery.exec();

        // Then, delete the folder
        SQLite::Statement deleteFolderQuery(*db, "DELETE FROM Folders WHERE id = ? AND userId = ?");
        deleteFolderQuery.bind(1, folderId);
        deleteFolderQuery.bind(2, userId);
        deleteFolderQuery.exec();

        transaction.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
        return false;
    }

    return true;
}

std::vector<Folder> DatabaseAccess::getAllFolders(int userId)
{
    std::vector<Folder> folders;

    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT * FROM Folders WHERE userId = ?");
        query.bind(1, userId);

        while (query.executeStep())
        {
            Folder folder;
            folder.id = query.getColumn(0).getInt();
            folder.userId = query.getColumn(1);
            folder.name = query.getColumn(2).getText();
            folders.push_back(folder);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database exception: " << e.what() << std::endl;
    }

    return folders;
}
