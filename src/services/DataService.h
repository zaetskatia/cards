#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include "DatabaseAccess.h"
#include "utils/Utility.h"
#include "utils/Types.h"

using json = nlohmann::json;

class DataService
{
public:
    DataService(DatabaseAccess &dbAccess) : dbAccess(dbAccess) {}

    std::optional<json> loginUser(const std::string &userData);
    std::optional<json> signupUser(const std::string &userData);

    std::optional<UserSession> getSessionByToken(const std::string &token);
    bool deleteSessionByToken(const std::string &token);
    std::optional<json> refreshUserSession(const std::string &refreshToken);

    std::optional<json> getAllCardsInFolder(int userId, int folerId);
    std::optional<json> getAllFolders(int userId);
    std::optional<json> getCardInFolder(int cardId, int userId, int folerId);
    std::optional<json> getFolder(int folderId, int userId);
    std::optional<json> insertCardInFolder(const std::string &cardData, int userId, int folerId);
    std::optional<json> insertFolder(const std::string &folderData, int userId);
    bool updateCardInFolder(int cardId, const std::string &cardData, int userId, int folerId);
    bool updateFolder(int folderId, const std::string &folderData, int userId);
    bool deleteCardInFolder(int cardId, int userId, int folerId);
    bool deleteFolder(int folderId, int userId);

private:
    std::optional<json> createUserSession(const User &user);
    std::optional<Card> convertToCard(const std::string &cardData, int folderId);
    std::optional<Folder> convertToFolder(const std::string &folderData);
    nlohmann::json convertCardToJson(const Card &card);
    nlohmann::json convertFolderToJson(const Folder &card);
    DatabaseAccess &dbAccess;
};