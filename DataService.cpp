#include "DataService.h"

std::optional<json> DataService::getAllCards(std::string clientId)
{
    auto cards = dbAccess.getAllCards(clientId);
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

std::optional<json> DataService::getCard(int cardId, std::string clientId)
{
    auto cardOpt = dbAccess.getCard(cardId, clientId);
    if (cardOpt.has_value())
    {
        return convertCardToJson(cardOpt.value());
    }
    return std::nullopt;
}

std::optional<json> DataService::insertCard(const std::string &cardData, std::string clientId)
{
    auto cardToInsertOpt = convertToCard(cardData);
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

bool DataService::updateCard(int cardId, const std::string &cardData, std::string clientId)
{
    auto cardToUpdateOpt = convertToCard(cardData);
    if (cardToUpdateOpt.has_value())
    {
        cardToUpdateOpt.value().id = cardId;
        cardToUpdateOpt.value().clientId = clientId;
        return dbAccess.updateCard(cardToUpdateOpt.value());
    }
    return false;
}

bool DataService::deleteCard(int cardId, std::string clientId)
{
    return dbAccess.deleteCard(cardId, clientId);
}

std::optional<Card> DataService::convertToCard(const std::string &cardData)
{
    try
    {
        auto j = json::parse(cardData);

        Card res{};
        res.term = j["term"];
        res.translation = j["translation"];
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