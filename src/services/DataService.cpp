#include "DataService.h"

std::optional<json> DataService::createUserSession(const User &user)
{
    std::string sessionToken = Utility::generateSessionToken();
    std::string expirationDateTime = Utility::calculateExpirationDateTime(24);
    bool tokenInserted = dbAccess.insertSessionToken(user.id, sessionToken, expirationDateTime);
    if (!tokenInserted)
    {
        return std::nullopt; // Handle token insertion failure
    }
    json response;
    response["token"] = sessionToken;
    response["expires"] = expirationDateTime;
    return response;
}

std::optional<UserSession> DataService::getSessionByToken(const std::string &token)
{
    return dbAccess.getSessionByToken(token);
}

std::optional<json> DataService::loginUser(const std::string &userData)
{
    json data = json::parse(userData);
    std::string username = data["username"];
    std::string password = data["password"];

    auto userOpt = dbAccess.getUserByUsername(username);
    if (!userOpt.has_value())
    {
        // Handle user not found
        return std::nullopt;
    }

    // TODO Use hash function later
    // std::string hashedPassword = Utility::hashPassword(password); Use hash function later
    std::string hashedPassword = password;

    if (hashedPassword != userOpt.value().passwordHash)
    {
        return std::nullopt; // Passwords do not match
    }

    return createUserSession(userOpt.value());
}

std::optional<json> DataService::signupUser(const std::string &userData)
{
    json dataJson = json::parse(userData);
    std::string username = dataJson["username"];
    std::string password = dataJson["password"];

    if (dbAccess.getUserByUsername(username).has_value())
    {
        return std::nullopt; // Username already exists
    }

    // TODO Use hash function later
    // std::string hashedPassword = Utility::hashPassword(password); Use hash function later
    std::string hashedPassword = password;

    auto newUserOpt = dbAccess.createUser(username, hashedPassword);
    if (!newUserOpt.has_value())
    {
        // Handle user creation failure
        return std::nullopt;
    }

    return createUserSession(newUserOpt.value());
}

bool DataService::deleteSessionByToken(const std::string &token)
{
    return dbAccess.deleteSessionByToken(token);
}

std::optional<json> DataService::getAllCardsInFolder(int userId, int folderId)
{
    auto cards = dbAccess.getAllCards(userId, folderId);
    if (!cards.empty())
    {
        nlohmann::json cardsJson = nlohmann::json::array();
        for (const auto &card : cards)
        {
            cardsJson.push_back({{"id", card.id}, {"term", card.term}, {"translation", card.translation}});
        }
        return cardsJson;
    }
    return std::nullopt;
}

std::optional<json> DataService::getAllFolders(int userId)
{
    auto folders = dbAccess.getAllFolders(userId);
    if (!folders.empty())
    {
        nlohmann::json foldersJson = nlohmann::json::array();
        for (const auto &folder : folders)
        {
            foldersJson.push_back({{"id", folder.id}, {"name", folder.name}});
        }
        return foldersJson;
    }
    return std::nullopt;
}

std::optional<json> DataService::getCardInFolder(int cardId, int userId, int folerId)
{
    auto cardOpt = dbAccess.getCard(cardId, userId, folerId);
    if (cardOpt.has_value())
    {
        return convertCardToJson(cardOpt.value());
    }
    return std::nullopt;
}

std::optional<json> DataService::getFolder(int folderId, int userId)
{
    auto folderOpt = dbAccess.getFolder(folderId, userId);
    if (folderOpt.has_value())
    {
        return convertFolderToJson(folderOpt.value());
    }
    return std::nullopt;
}

std::optional<json> DataService::insertCardInFolder(const std::string &cardData, int userId, int folderId)
{
    auto cardToInsertOpt = convertToCard(cardData, folderId);
    if (!cardToInsertOpt.has_value())
    {
        return std::nullopt;
    }
    cardToInsertOpt.value().userId = userId;
    auto cardOpt = dbAccess.insertCard(cardToInsertOpt.value());
    if (cardOpt.has_value())
    {
        return convertCardToJson(cardOpt.value());
    }
    return std::nullopt;
}

std::optional<json> DataService::insertFolder(const std::string &folderData, int userId)
{
    auto folderToInsertOpt = convertToFolder(folderData);
    if (!folderToInsertOpt.has_value())
    {
        return std::nullopt;
    }
    folderToInsertOpt.value().userId = userId;
    auto folderOpt = dbAccess.insertFolder(folderToInsertOpt.value());
    if (folderOpt.has_value())
    {
        return convertFolderToJson(folderOpt.value());
    }
    return std::nullopt;
}

bool DataService::updateCardInFolder(int cardId, const std::string &cardData, int userId, int folderId)
{
    auto cardToUpdateOpt = convertToCard(cardData, folderId);
    if (cardToUpdateOpt.has_value())
    {
        cardToUpdateOpt.value().id = cardId;
        cardToUpdateOpt.value().userId = userId;
        return dbAccess.updateCard(cardToUpdateOpt.value());
    }
    return false;
}

bool DataService::updateFolder(int folderId, const std::string &folderData, int userId)
{
    auto folderToUpdateOpt = convertToFolder(folderData);
    if (folderToUpdateOpt.has_value())
    {
        folderToUpdateOpt.value().id = folderId;
        folderToUpdateOpt.value().userId = userId;
        return dbAccess.updateFolder(folderToUpdateOpt.value());
    }
    return false;
}

bool DataService::deleteCardInFolder(int cardId, int userId, int folerId)
{
    return dbAccess.deleteCard(cardId, userId, folerId);
}

bool DataService::deleteFolder(int folderId, int userId)
{
    return dbAccess.deleteFolder(folderId, userId);
}

std::optional<Card> DataService::convertToCard(const std::string &cardData, int folderId)
{
    try
    {
        auto j = json::parse(cardData);

        Card res{};
        res.term = j["term"];
        res.translation = j["translation"];
        res.folderId = folderId;
        return res;
    }
    catch (const json::parse_error &e)
    {
        return std::nullopt;
    }
}

std::optional<Folder> DataService::convertToFolder(const std::string &folderData)
{
    try
    {
        auto j = json::parse(folderData);

        Folder res{};
        res.name = j["name"];
        return res;
    }
    catch (const json::parse_error &e)
    {
        return std::nullopt;
    }
}

json DataService::convertCardToJson(const Card &card)
{
    json cardJson;
    cardJson["id"] = card.id;
    cardJson["folderId"] = card.folderId;
    cardJson["term"] = card.term;
    cardJson["translation"] = card.translation;

    return cardJson;
}

json DataService::convertFolderToJson(const Folder &folder)
{
    json folderJson;
    folderJson["id"] = folder.id;
    folderJson["name"] = folder.name;

    return folderJson;
}