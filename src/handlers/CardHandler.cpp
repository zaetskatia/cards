#include "CardHandler.h"

void CardHandler::handleGetRequest(const http::request<http::string_body> &request,
                                   http::response<http::string_body> &response,
                                   const std::string &clientId)
{
    auto folderIdOpt = Utility::extractQueryParameter(request.target().to_string(), "folderId");
    if (!folderIdOpt)
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Folder ID must be provided for card operations", http::status::bad_request);
        return;
    }

    auto cardIdOpt = Utility::extractIdFromURL(request.target().to_string(), "cards");
    if (cardIdOpt)
    {
        // Get a specific card within a folder
        auto card = dataService->getCardInFolder(cardIdOpt.value(), clientId, folderIdOpt.value());
        if (card)
        {
            HttpResponseBuilder::buildJsonResponseForData(response, card.value());
        }
        else
        {
            HttpResponseBuilder::buildJsonResponseForError(response, "Card not found in the specified folder", http::status::not_found);
        }
    }
    else
    {
        // Get all cards within a folder
        auto cardsOpt = dataService->getAllCardsInFolder(clientId, folderIdOpt.value());
        if (cardsOpt.has_value())
        {
            HttpResponseBuilder::buildJsonResponseForData(response, cardsOpt.value());
        }
        else
        {
            HttpResponseBuilder::buildJsonResponseForError(response, "No cards found in the folder", http::status::not_found);
        }
    }
}

void CardHandler::handlePostRequest(const http::request<http::string_body> &request,
                                    http::response<http::string_body> &response,
                                    const std::string &clientId)
{
    auto folderIdOpt = Utility::extractQueryParameter(request.target().to_string(), "folderId");
    if (!folderIdOpt)
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Folder ID must be provided to create a card", http::status::bad_request);
        return;
    }

    auto createResultOpt = dataService->insertCardInFolder(request.body(), clientId, folderIdOpt.value());
    if (createResultOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForData(response, createResultOpt.value());
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Failed to create card in the specified folder", http::status::internal_server_error);
    }
}

void CardHandler::handlePutRequest(const http::request<http::string_body> &request,
                                   http::response<http::string_body> &response,
                                   const std::string &clientId)
{
    std::string url = request.target().to_string();
    auto cardIdOpt = Utility::extractIdFromURL(url, "cards");
    auto folderIdOpt = Utility::extractQueryParameter(url, "folderId");

    if (!cardIdOpt.has_value() || !folderIdOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Invalid request parameters", http::status::bad_request);
        return;
    }

    bool updateSuccess = dataService->updateCardInFolder(cardIdOpt.value(), request.body(), clientId, folderIdOpt.value());

    if (updateSuccess)
    {
        nlohmann::json successJson;
        successJson["message"] = "Card updated with ID: " + std::to_string(cardIdOpt.value());
        HttpResponseBuilder::buildJsonResponseForData(response, successJson);
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Card with ID: " + std::to_string(cardIdOpt.value()) + " not found in folder", http::status::not_found);
    }
}

void CardHandler::handleDeleteRequest(const http::request<http::string_body> &request,
                                      http::response<http::string_body> &response,
                                      const std::string &clientId)
{
    std::string url = request.target().to_string();
    auto cardIdOpt = Utility::extractIdFromURL(url, "cards");
    auto folderIdOpt = Utility::extractQueryParameter(url, "folderId");

    if (!cardIdOpt.has_value() || !folderIdOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Invalid request parameters", http::status::bad_request);
        return;
    }

    bool deletionSuccess = dataService->deleteCardInFolder(cardIdOpt.value(), clientId, folderIdOpt.value());

    if (deletionSuccess)
    {
        nlohmann::json successJson;
        successJson["message"] = "Card deleted successfully from folder";
        HttpResponseBuilder::buildJsonResponseForData(response, successJson);
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Card or folder not found", http::status::not_found);
    }
}
