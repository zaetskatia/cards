#include <iostream>
#include <optional>
#include "ServerLogic.h"

std::shared_ptr<http::response<http::string_body>> ServerLogic::handleRequest(http::request<http::string_body> &request)
{
    auto response = std::make_shared<http::response<http::string_body>>();
    response->version(request.version());
    response->set(http::field::server, "My HTTP Cards Server");

    std::string url = request.target().to_string();
    DataType type = determineDataType(url);
    auto handler = getHandlerForType(type);

    auto clientIdOpt = extractClientId(request);
    if (!clientIdOpt.has_value())
    {
        // Handle error: Client ID not provided or invalid
        HttpResponseBuilder::buildJsonResponseForError(*response, "No client id found", http::status::unauthorized);
        response->prepare_payload();
        return response;
    }

    std::string clientId = clientIdOpt.value();

    try
    {
        switch (request.method())
        {
        case http::verb::get:
            handler->handleGetRequest(request, *response, clientId);
            break;
        case http::verb::post:
            handler->handlePostRequest(request, *response, clientId);
            break;
        case http::verb::put:
            handler->handlePutRequest(request, *response, clientId);
            break;
        case http::verb::delete_:
            handler->handleDeleteRequest(request, *response, clientId);
            break;
        default:
            HttpResponseBuilder::buildJsonResponseForError(*response, "Method not allowed", http::status::method_not_allowed);
        }

        response->prepare_payload();
        return response;
    }
    catch (const std::exception &e)
    {
        HttpResponseBuilder::buildJsonResponseForError(*response, "Undefined error", http::status::internal_server_error);
        response->prepare_payload();
        return response;
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

DataType ServerLogic::determineDataType(const std::string &url)
{
    if (url.find("/cards") != std::string::npos)
    {
        return DataType::Card;
    }
    else if (url.find("/folders") != std::string::npos)
    {
        return DataType::Folder;
    }
    return DataType::None;
}

std::unique_ptr<IEntityHandler> ServerLogic::getHandlerForType(DataType type)
{
    switch (type)
    {
    case DataType::Card:
        return std::make_unique<CardHandler>(&dataService);
    case DataType::Folder:
        return std::make_unique<FolderHandler>(&dataService);
    default:
        throw std::runtime_error("Unsupported data type");
    }
}