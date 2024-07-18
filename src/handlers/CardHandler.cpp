#include "CardHandler.h"

void CardHandler::handleGetRequest(const http::request<http::string_body> &request,
                                   http::response<http::string_body> &response,
                                   int userId)
{
    auto folderIdOpt = Utility::extractQueryParameter(request.target().to_string(), "folderId");
    if (!folderIdOpt)
    {
        HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::NoFolderId, http::status::bad_request);
        return;
    }

    auto cardIdOpt = Utility::extractIdFromURL(request.target().to_string(), "cards");
    if (cardIdOpt)
    {
        // Get a specific card within a folder
        auto card = dataService->getCardInFolder(cardIdOpt.value(), userId, folderIdOpt.value());
        if (card)
        {
            HttpResponseBuilder::buildJsonResponseForData(response, card.value());
        }
        else
        {
            HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::CardNotFound, http::status::not_found);
        }
    }
    else
    {
        // Get all cards within a folder
        auto cardsOpt = dataService->getAllCardsInFolder(userId, folderIdOpt.value());
        if (cardsOpt.has_value())
        {
            HttpResponseBuilder::buildJsonResponseForData(response, cardsOpt.value());
        }
        else
        {
            HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::FolderIsEmpty, http::status::not_found);
        }
    }
}

void CardHandler::handlePostRequest(const http::request<http::string_body> &request,
                                    http::response<http::string_body> &response,
                                    int userId)
{
    auto folderIdOpt = Utility::extractQueryParameter(request.target().to_string(), "folderId");
    if (!folderIdOpt)
    {
        HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::NoFolderId, http::status::bad_request);
        return;
    }

    auto createResultOpt = dataService->insertCardInFolder(request.body(), userId, folderIdOpt.value());
    if (createResultOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForData(response, createResultOpt.value());
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::FailedToCreateCard, http::status::internal_server_error);
    }
}

void CardHandler::handlePutRequest(const http::request<http::string_body> &request,
                                   http::response<http::string_body> &response,
                                   int userId)
{
    std::string url = request.target().to_string();
    auto cardIdOpt = Utility::extractIdFromURL(url, "cards");
    auto folderIdOpt = Utility::extractQueryParameter(url, "folderId");

    if (!cardIdOpt.has_value() || !folderIdOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::InvalidRequestParameters, http::status::bad_request);
        return;
    }

    bool updateSuccess = dataService->updateCardInFolder(cardIdOpt.value(), request.body(), userId, folderIdOpt.value());

    if (updateSuccess)
    {
        nlohmann::json successJson;
        successJson["message"] = "Card updated with ID: " + std::to_string(cardIdOpt.value());
        HttpResponseBuilder::buildJsonResponseForData(response, successJson);
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::CardNotFound, http::status::not_found);
    }
}

void CardHandler::handleDeleteRequest(const http::request<http::string_body> &request,
                                      http::response<http::string_body> &response,
                                      int userId)
{
    std::string url = request.target().to_string();
    auto cardIdOpt = Utility::extractIdFromURL(url, "cards");
    auto folderIdOpt = Utility::extractQueryParameter(url, "folderId");

    if (!cardIdOpt.has_value() || !folderIdOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::InvalidRequestParameters, http::status::bad_request);
        return;
    }

    bool deletionSuccess = dataService->deleteCardInFolder(cardIdOpt.value(), userId, folderIdOpt.value());

    if (deletionSuccess)
    {
        nlohmann::json successJson;
        successJson["message"] = "Card deleted successfully from folder";
        HttpResponseBuilder::buildJsonResponseForData(response, successJson);
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::CardNotFound, http::status::not_found);
    }
}
