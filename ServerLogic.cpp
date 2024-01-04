#include <iostream>
#include <optional>
#include "ServerLogic.h"

void ServerLogic::handleGetAllCardsRequest(http::request<http::string_body> &request,
                                           http::response<http::string_body> &response,
                                           std::string clientId)
{
    auto cardsOpt = dataService.getAllCards(clientId);
    if (cardsOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForData(response, cardsOpt.value());
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "No cards found", http::status::not_found);
    }
}

void ServerLogic::handleGetRequest(http::request<http::string_body> &request, http::response<http::string_body> &response, std::string clientId)
{
    std::string url = request.target().to_string();
    auto cardIdOpt = extractCardIdFromURL(url);
    if (!cardIdOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Invalid card ID format", http::status::bad_request);
        return;
    }

    auto cardOpt = dataService.getCard(cardIdOpt.value(), clientId);
    if (cardOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForData(response, cardOpt.value());
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Card with ID: " + std::to_string(cardIdOpt.value()) + " not found", http::status::not_found);
    }
}

void ServerLogic::handlePostRequest(http::request<http::string_body> &request, http::response<http::string_body> &response, std::string clientId)
{
    auto newCardOpt = dataService.insertCard(request.body(), clientId);

    if (newCardOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForData(response, newCardOpt.value());
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Failed to create new card", http::status::internal_server_error);
    }
}

void ServerLogic::handlePutRequest(http::request<http::string_body> &request, http::response<http::string_body> &response, std::string clientId)
{
    std::string url = request.target().to_string();
    auto cardIdOpt = extractCardIdFromURL(url);
    if (!cardIdOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Invalid card ID format", http::status::bad_request);
        return;
    }

    if (dataService.updateCard(cardIdOpt.value(), request.body(), clientId))
    {
        nlohmann::json successJson;
        successJson["message"] = "Card updated with ID: " + std::to_string(cardIdOpt.value());
        HttpResponseBuilder::buildJsonResponseForData(response, successJson);
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Card with ID: " + std::to_string(cardIdOpt.value()) + " not found", http::status::not_found);
    }
}

void ServerLogic::handleDeleteRequest(http::request<http::string_body> &request, http::response<http::string_body> &response, std::string clientId)
{
    std::string url = request.target().to_string();
    auto cardIdOpt = extractCardIdFromURL(url);
    if (!cardIdOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Invalid card ID format", http::status::bad_request);
        return;
    }
    if (dataService.deleteCard(cardIdOpt.value(), clientId))
    {
        nlohmann::json successJson;
        successJson["message"] = "Card deleted with ID: " + std::to_string(cardIdOpt.value());
        HttpResponseBuilder::buildJsonResponseForData(response, successJson);
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Card with ID: " + std::to_string(cardIdOpt.value()) + " not found", http::status::not_found);
    }
}

std::optional<std::string> ServerLogic::extractClientId(const http::request<http::string_body> &request)
{
    auto it = request.find("X-Client-ID");
    if (it == request.end())
    {
        return std::nullopt; // Header not found
    }

    std::string clientIdStr = std::string(it->value().data(), it->value().size());

    if (clientIdStr.empty())
    {
        return std::nullopt; // Header value is empty
    }

    try
    {
        return clientIdStr;
    }
    catch (...)
    {
        return std::nullopt;
        ;
    }
}

std::optional<int> ServerLogic::extractCardIdFromURL(const std::string &url)
{
    std::vector<std::string> segments;
    std::stringstream urlStream(url);
    std::string segment;

    // Split the URL into segments based on '/'
    while (std::getline(urlStream, segment, '/'))
    {
        if (!segment.empty())
        {
            segments.push_back(segment);
        }
    }

    // Assuming the URL format is /cards/{cardId}
    if (segments.size() >= 2 && segments[0] == "cards")
    {
        try
        {
            return std::stoi(segments[1]);
        }
        catch (const std::invalid_argument &e)
        {
            // Handle the case where the conversion fails
            return std::nullopt;
        }
    }

    return std::nullopt; // Card ID not found or URL format is incorrect
}

std::shared_ptr<http::response<http::string_body>> ServerLogic::handleRequest(http::request<http::string_body> &request)
{
    auto response = std::make_shared<http::response<http::string_body>>();
    response->version(request.version());
    response->set(http::field::server, "My HTTP Cards Server");

    try
    {
        auto clientIdOpt = extractClientId(request);
        if (!clientIdOpt.has_value())
        {
            // Handle error: Client ID not provided or invalid
            HttpResponseBuilder::buildJsonResponseForError(*response, "No client id found", http::status::unauthorized);
            response->prepare_payload();
            return response;
        }
        std::string clientId = clientIdOpt.value();

        if (request.method() == http::verb::post)
        {
            handlePostRequest(request, *response, clientId);
        }
        else if (request.method() == http::verb::get)
        {
            std::string url = request.target().to_string();
            if (url.find("/cards/") != std::string::npos)
            {
                handleGetRequest(request, *response, clientId);
            }
            else if (url == "/cards")
            {
                handleGetAllCardsRequest(request, *response, clientId);
            }
        }
        else if (request.method() == http::verb::put)
        {
            handlePutRequest(request, *response, clientId);
        }
        else if (request.method() == http::verb::delete_)
        {
            handleDeleteRequest(request, *response, clientId);
        }
        else
        {
            // Method not supported
            HttpResponseBuilder::buildJsonResponseForError(*response, "Method not allowed", http::status::method_not_allowed);
        }

        response->prepare_payload();
        return response;
    }
    catch (const std::exception &e)
    {
        // Send an Internal Server Error response
        HttpResponseBuilder::buildJsonResponseForError(*response, "Undefined error", http::status::internal_server_error);
        response->prepare_payload();
        return response;
    }
}
