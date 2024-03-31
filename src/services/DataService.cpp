#include "DataService.h"

std::optional<json> DataService::createUserSession(const User &user)
{
    std::string accessToken = Utility::generateSessionToken();
    std::string refreshToken = Utility::generateSessionToken();
    std::string accessExpirationDateTime = Utility::calculateExpirationDateTime(2);
    std::string refreshExpirationDateTime = Utility::calculateExpirationDateTime(24 * 60 * 30);

    // Insert both tokens into the database with their expiration times
    bool tokensInserted = dbAccess.insertSessionToken(user.id, accessToken, accessExpirationDateTime, refreshToken, refreshExpirationDateTime);
    if (!tokensInserted)
    {
        return std::nullopt; // Handle token insertion failure
    }

    json response;
    response["accessToken"] = accessToken;
    response["accessTokenExpires"] = accessExpirationDateTime;
    response["refreshToken"] = refreshToken;
    response["refreshTokenExpires"] = refreshExpirationDateTime;
    return response;
}

std::optional<UserSession> DataService::getSessionByToken(const std::string &token)
{
    return dbAccess.getSessionByToken(token);
}

std::optional<json> DataService::refreshUserSession(const std::string &refreshToken)
{
    auto sessionOpt = dbAccess.getSessionByToken(refreshToken, true);

    if (!sessionOpt.has_value() || Utility::isExpiredTime(sessionOpt.value().expirationRefresh))
    {
        return std::nullopt; // Refresh token is invalid or expired
    }

    auto session = sessionOpt.value();

    // Generate new tokens
    session.token = Utility::generateSessionToken();
    session.expiration = Utility::calculateExpirationDateTime(2);
    session.tokenRefresh = Utility::generateSessionToken();
    session.expirationRefresh = Utility::calculateExpirationDateTime(24 * 30 * 60);

    // Update the session in the database with new tokens and their expiration times
    bool updateSuccess = dbAccess.updateSessionToken(session);
    if (!updateSuccess)
    {
        return std::nullopt; // Failed to update the session
    }

    // Prepare and return the response with new tokens
    json response;
    response["accessToken"] = session.token;
    response["accessTokenExpires"] = session.expiration;
    response["refreshToken"] = session.tokenRefresh;
    response["refreshTokenExpires"] =  session.expirationRefresh;
    return response;
}

std::optional<json> DataService::loginUser(const std::string &userData)
{
    json data = json::parse(userData);
    std::string username = data["username"];
    std::string password = data["password"];

    if (username.length() > maxUsernameLength || password.length() > maxPasswordLength)
    {
        return std::nullopt;
    }

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

    // Invalidate any existing refresh tokens for the user as a security measure
    dbAccess.invalidateUserRefreshTokens(userOpt.value().id);

    return createUserSession(userOpt.value());
}

std::optional<json> DataService::signupUser(const std::string &userData)
{
    json dataJson = json::parse(userData);
    std::string username = dataJson["username"];
    std::string password = dataJson["password"];

    if (username.length() > maxUsernameLength || password.length() > maxPasswordLength)
    {
        return std::nullopt;
    }

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
            cardsJson.push_back(convertCardToJson(card));
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
            foldersJson.push_back(convertFolderToJson(folder));
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

        std::string term = j.at("term").get<std::string>();
        std::string translation = j.at("translation").get<std::string>();

        if (term.length() > maxTermLength || translation.length() > maxTranslationLength)
        {
            return std::nullopt; // TODO handle errors message to server
        }

        Card res{};
        res.term = term;
        res.translation = translation;
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
        std::string name = j.at("name").get<std::string>();
        if (name.length() > maxFolderNameLength)
        {
            return std::nullopt; // TODO handle errors message to server
        }
        Folder res{};
        res.name = name;
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