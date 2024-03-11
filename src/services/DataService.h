#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>
#include "DatabaseAccess.h"

using json = nlohmann::json;

class DataService
{
public:
    DataService(DatabaseAccess &dbAccess) : dbAccess(dbAccess) {}

    std::optional<json> getAllCardsInFolder(std::string clientId, int folerId);
    std::optional<json> getAllFolders(std::string clientId);
    std::optional<json> getCardInFolder(int cardId, std::string clientId, int folerId);
    std::optional<json> getFolder(int folderId, std::string clientId);
    std::optional<json> insertCardInFolder(const std::string &cardData, std::string clientId, int folerId);
    std::optional<json> insertFolder(const std::string &folderData, std::string clientId);
    bool updateCardInFolder(int cardId, const std::string &cardData, std::string clientId, int folerId);
    bool updateFolder(int folderId, const std::string &folderData, std::string clientId);
    bool deleteCardInFolder(int cardId, std::string clientId, int folerId);
    bool deleteFolder(int folderId, std::string clientId);

private:
    std::optional<Card> convertToCard(const std::string &cardData, int folderId);
    std::optional<Folder> convertToFolder(const std::string &folderData);
    nlohmann::json convertCardToJson(const Card &card);
    nlohmann::json convertFolderToJson(const Folder &card);
    DatabaseAccess &dbAccess;
};