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
            clientId TEXT,
            folderId INTEGER,
            term TEXT,
            translation TEXT
        )
    )";

    const char *createFoldersTableSql = R"(
        CREATE TABLE IF NOT EXISTS Folders (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            clientId TEXT,
            name TEXT
        )
    )";

    try
    {
        SQLite::Statement cardsQuery(*db, createCardsTableSql);
        cardsQuery.exec();

        SQLite::Statement foldersQuery(*db, createFoldersTableSql);
        foldersQuery.exec();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to create table: " << e.what() << std::endl;
    }
}

std::optional<Card> DatabaseAccess::insertCard(const Card &card)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        SQLite::Statement query(*db, "INSERT INTO Cards (clientId, folderId, term, translation) VALUES (?, ?, ?, ?)");
        query.bind(1, card.clientId);
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

std::optional<Card> DatabaseAccess::getCard(int cardId, std::string clientId, int folderId)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT * FROM Cards WHERE id = ? AND clientId = ? AND folderId = ?");

        query.bind(1, cardId);
        query.bind(2, clientId);
        query.bind(3, folderId);

        if (query.executeStep())
        {
            Card card;
            card.id = query.getColumn(0);
            card.clientId = query.getColumn(1).getText();
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

std::vector<Card> DatabaseAccess::getAllCards(std::string clientId, int folderId)
{
    std::vector<Card> cards;

    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT * FROM Cards WHERE clientId = ? AND folderId = ?");

        query.bind(1, clientId);
        query.bind(2, folderId);

        while (query.executeStep())
        {
            Card card;
            card.id = query.getColumn(0).getInt();
            card.clientId = query.getColumn(1).getText();
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

        SQLite::Statement query(*db, "UPDATE Cards SET folderId = ?, term = ?, translation = ? WHERE id = ? AND clientId = ? AND folderId = ?");
        query.bind(1, card.folderId);
        query.bind(2, card.term);
        query.bind(3, card.translation);
        query.bind(4, card.id);
        query.bind(5, card.clientId);
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

bool DatabaseAccess::deleteCard(int cardId, std::string clientId, int folderId)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);
        SQLite::Statement query(*db, "DELETE FROM Cards WHERE id = ? AND clientId = ? AND folderId = ?");
        query.bind(1, cardId);
        query.bind(2, clientId);
        query.bind(2, folderId);
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

        SQLite::Statement query(*db, "INSERT INTO Folders (clientId, name) VALUES (?, ?)");
        query.bind(1, folder.clientId);
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

std::optional<Folder> DatabaseAccess::getFolder(int folderId, std::string clientId)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT * FROM Folders WHERE id = ? AND clientId = ?");
        query.bind(1, folderId);
        query.bind(2, clientId);

        if (query.executeStep())
        {
            Folder folder;
            folder.id = query.getColumn(0);
            folder.clientId = query.getColumn(1).getText();
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

        SQLite::Statement query(*db, "UPDATE Folders SET name = ? WHERE id = ? AND clientId = ?");
        query.bind(1, folder.name);
        query.bind(2, folder.id);
        query.bind(3, folder.clientId);
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

bool DatabaseAccess::deleteFolder(int folderId, std::string clientId)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        // First, delete all cards associated with the folder
        SQLite::Statement deleteCardsQuery(*db, "DELETE FROM Cards WHERE folderId = ? AND clientId = ?");
        deleteCardsQuery.bind(1, folderId);
        deleteCardsQuery.bind(2, clientId);
        deleteCardsQuery.exec();

        // Then, delete the folder
        SQLite::Statement deleteFolderQuery(*db, "DELETE FROM Folders WHERE id = ? AND clientId = ?");
        deleteFolderQuery.bind(1, folderId);
        deleteFolderQuery.bind(2, clientId);
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

std::vector<Folder> DatabaseAccess::getAllFolders(std::string clientId)
{
    std::vector<Folder> folders;

    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT * FROM Folders WHERE clientId = ?");
        query.bind(1, clientId);

        while (query.executeStep())
        {
            Folder folder;
            folder.id = query.getColumn(0).getInt();
            folder.clientId = query.getColumn(1).getText();
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
