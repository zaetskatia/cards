#include <iostream>
#include <optional>
#include "ServerLogic.h"

std::shared_ptr<http::response<http::string_body>> ServerLogic::handleRequest(http::request<http::string_body> &request)
{
    auto response = std::make_shared<http::response<http::string_body>>();
    response->version(request.version());
    response->set(http::field::server, "My HTTP Cards Server");

    std::string url = request.target().to_string();

    if (url.find("/signin") != std::string::npos)
    {
        handleLoginRequest(request, *response);
        response->prepare_payload();
        return response;
    }
    else if (url.find("/signup") != std::string::npos)
    {
        handleSignupRequest(request, *response);
        response->prepare_payload();
        return response;
    }
    else if (url.find("/refresh_token") != std::string::npos)
    {
        handleTokenRefreshRequest(request, *response);
        response->prepare_payload();
        return response;
    }

    auto toketOpt = getTokenFromRequest(request);
    if (!toketOpt.has_value())
    {
        // Handle authentication failure
        HttpResponseBuilder::buildJsonResponseForError(*response, "Token is not present in request", http::status::unauthorized);
        response->prepare_payload();
        return response;
    }
    auto userIdOpt = validateTokenAndGetUserId(toketOpt.value());
    if (!userIdOpt.has_value())
    {
        // Handle authentication failure
        HttpResponseBuilder::buildJsonResponseForError(*response, "Authentication failed or token expired", http::status::unauthorized);
        response->prepare_payload();
        return response;
    }

    if (url.find("/logout") != std::string::npos)
    {
        bool isLogout = dataService.deleteSessionByToken(toketOpt.value());

        isLogout ? HttpResponseBuilder::buildJsonResponseForData(*response, "Loged out") : HttpResponseBuilder::buildJsonResponseForError(*response, "Error during log out");

        response->prepare_payload();
        return response;
    }

    DataType type = determineDataType(url);
    auto handler = getHandlerForType(type);

    int userId = userIdOpt.value();

    try
    {
        switch (request.method())
        {
        case http::verb::get:
            handler->handleGetRequest(request, *response, userId);
            break;
        case http::verb::post:
            handler->handlePostRequest(request, *response, userId);
            break;
        case http::verb::put:
            handler->handlePutRequest(request, *response, userId);
            break;
        case http::verb::delete_:
            handler->handleDeleteRequest(request, *response, userId);
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

void ServerLogic::handleLoginRequest(http::request<http::string_body> &request, http::response<http::string_body> &response)
{
    auto loginResultOpt = dataService.loginUser(request.body());

    if (loginResultOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForData(response, loginResultOpt.value());
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Error during log up", http::status::unauthorized);
    }
}

void ServerLogic::handleSignupRequest(http::request<http::string_body> &request, http::response<http::string_body> &response)
{
    auto signupResultOpt = dataService.signupUser(request.body());

    if (signupResultOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForData(response, signupResultOpt.value());
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Error during sign up", http::status::unauthorized);
    }
}

void ServerLogic::handleTokenRefreshRequest(http::request<http::string_body>& request, http::response<http::string_body>& response) {
    auto refreshTokenOpt = getTokenFromRequest(request);
    if (!refreshTokenOpt.has_value()) {
        HttpResponseBuilder::buildJsonResponseForError(response, "Refresh token is missing", http::status::unauthorized);
        return;
    }

    auto newSessionOpt = dataService.refreshUserSession(refreshTokenOpt.value());
    if (!newSessionOpt.has_value()) {
        HttpResponseBuilder::buildJsonResponseForError(response, "Failed to refresh session", http::status::unauthorized);
        return;
    }

    HttpResponseBuilder::buildJsonResponseForData(response, newSessionOpt.value());
}

std::optional<int> ServerLogic::validateTokenAndGetUserId(const std::string &token)
{
    auto session = dataService.getSessionByToken(token);
    if (!session.has_value() || Utility::isExpiredTime(session.value().expiration))
    {
        // Token does not exist
        return std::nullopt;
    }
    
    return session.value().userId;
}

std::optional<std::string> ServerLogic::getTokenFromRequest(const http::request<http::string_body> &request)
{
    // Look for the Authorization header.
    auto it = request.find(http::field::authorization);
    if (it == request.end())
    {
        // Authorization header not found.
        return std::nullopt;
    }

    // Extract the value of the Authorization header.
    std::string authHeader = it->value().to_string();

    // Check if the Authorization header value starts with "Bearer ".
    std::string bearerPrefix = "Bearer ";
    if (authHeader.substr(0, bearerPrefix.length()) != bearerPrefix)
    {
        // The token is not provided in the expected format.
        return std::nullopt;
    }

    // Extract the token from the header value.
    std::string token = authHeader.substr(bearerPrefix.length());
    if (token.empty())
    {
        // Token is empty.
        return std::nullopt;
    }

    return token;
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