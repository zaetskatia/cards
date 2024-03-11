#include "DataService.h"

std::optional<json> DataService::getAllCardsInFolder(std::string clientId, int folderId)
{
    auto cards = dbAccess.getAllCards(clientId, folderId);
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

std::optional<json> DataService::getAllFolders(std::string clientId)
{
    auto folders = dbAccess.getAllFolders(clientId);
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

std::optional<json> DataService::getCardInFolder(int cardId, std::string clientId, int folerId)
{
    auto cardOpt = dbAccess.getCard(cardId, clientId, folerId);
    if (cardOpt.has_value())
    {
        return convertCardToJson(cardOpt.value());
    }
    return std::nullopt;
}

std::optional<json> DataService::getFolder(int folderId, std::string clientId)
{
    auto folderOpt = dbAccess.getFolder(folderId, clientId);
    if (folderOpt.has_value())
    {
        return convertFolderToJson(folderOpt.value());
    }
    return std::nullopt;
}

std::optional<json> DataService::insertCardInFolder(const std::string &cardData, std::string clientId, int folderId)
{
    auto cardToInsertOpt = convertToCard(cardData, folderId);
    if (!cardToInsertOpt.has_value())
    {
        return std::nullopt;
    }
    cardToInsertOpt.value().clientId = clientId;
    auto cardOpt = dbAccess.insertCard(cardToInsertOpt.value());
    if (cardOpt.has_value())
    {
        return convertCardToJson(cardOpt.value());
    }
    return std::nullopt;
}

std::optional<json> DataService::insertFolder(const std::string &folderData, std::string clientId)
{
    auto folderToInsertOpt = convertToFolder(folderData);
    if (!folderToInsertOpt.has_value())
    {
        return std::nullopt;
    }
    folderToInsertOpt.value().clientId = clientId;
    auto folderOpt = dbAccess.insertFolder(folderToInsertOpt.value());
    if (folderOpt.has_value())
    {
        return convertFolderToJson(folderOpt.value());
    }
    return std::nullopt;
}

bool DataService::updateCardInFolder(int cardId, const std::string &cardData, std::string clientId, int folderId)
{
    auto cardToUpdateOpt = convertToCard(cardData, folderId);
    if (cardToUpdateOpt.has_value())
    {
        cardToUpdateOpt.value().id = cardId;
        cardToUpdateOpt.value().clientId = clientId;
        return dbAccess.updateCard(cardToUpdateOpt.value());
    }
    return false;
}

bool DataService::updateFolder(int folderId, const std::string &folderData, std::string clientId)
{
    auto folderToUpdateOpt = convertToFolder(folderData);
    if (folderToUpdateOpt.has_value())
    {
        folderToUpdateOpt.value().id = folderId;
        folderToUpdateOpt.value().clientId = clientId;
        return dbAccess.updateFolder(folderToUpdateOpt.value());
    }
    return false;
}

bool DataService::deleteCardInFolder(int cardId, std::string clientId, int folerId)
{
    return dbAccess.deleteCard(cardId, clientId, folerId);
}

bool DataService::deleteFolder(int folderId, std::string clientId)
{
    return dbAccess.deleteFolder(folderId, clientId);
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