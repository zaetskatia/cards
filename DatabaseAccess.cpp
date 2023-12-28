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
    // SQL statement to create the Cards table if it doesn't exist
    const char *createTableSql = R"(
        CREATE TABLE IF NOT EXISTS Cards (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            clientId TEXT,
            folderId INTEGER,
            term TEXT,
            translation TEXT
        )
    )";

    try
    {
        SQLite::Statement query(*db, createTableSql);
        query.exec(); // Execute the SQL statement to create the table
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

std::optional<Card> DatabaseAccess::getCard(int cardId, std::string clientId)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT * FROM Cards WHERE id = ? AND clientId = ?");
        query.bind(1, cardId);
        query.bind(2, clientId);

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

    return std::nullopt; // Return an empty Card if not found or in case of error
}

std::vector<Card> DatabaseAccess::getAllCards(std::string clientId)
{
    std::vector<Card> cards;

    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Statement query(*db, "SELECT * FROM Cards WHERE clientId = ?");
        query.bind(1, clientId);

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
        // Handle or log the error as needed
    }

    return cards;
}

bool DatabaseAccess::updateCard(const Card &card)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);

        SQLite::Statement query(*db, "UPDATE Cards SET folderId = ?, term = ?, translation = ? WHERE id = ? AND clientId = ?");
        query.bind(1, card.folderId);
        query.bind(2, card.term);
        query.bind(3, card.translation);
        query.bind(4, card.id);
        query.bind(4, card.clientId);
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

bool DatabaseAccess::deleteCard(int cardId, std::string clientId)
{
    try
    {
        std::lock_guard<std::mutex> guard(dbMutex);
        SQLite::Transaction transaction(*db);
        SQLite::Statement query(*db, "DELETE FROM Cards WHERE id = ? AND clientId = ?");
        query.bind(1, cardId);
        query.bind(2, clientId);
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
