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

    std::optional<json> getAllCards(std::string clientId);
    std::optional<json> getCard(int cardId, std::string clientId);
    std::optional<json> insertCard(const std::string &cardData, std::string clientId);
    bool updateCard(int cardId, const std::string &cardData, std::string clientId);
    bool deleteCard(int cardId, std::string clientId);

private:
    std::optional<Card> convertToCard(const std::string &cardData);
    nlohmann::json convertCardToJson(const Card &card);
    DatabaseAccess &dbAccess;
};