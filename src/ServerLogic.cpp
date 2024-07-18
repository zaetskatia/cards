#include <iostream>
#include <optional>
#include <jwt-cpp/jwt.h>
#include "ServerLogic.h"

std::shared_ptr<http::response<http::string_body>> ServerLogic::handleRequest(http::request<http::string_body> &request)
{
    auto response = std::make_shared<http::response<http::string_body>>();
    response->version(request.version());
    response->set(http::field::server, "My HTTP Cards Server");

    std::string url = request.target().to_string();

    if (url.find("/google_signin") != std::string::npos)
    {
        handleGoogleSignIn(request, *response);
        response->prepare_payload();
        return response;
    }
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
        HttpResponseBuilder::buildJsonResponseForError(*response, ErrorCode::TokenIsNotExist, http::status::unauthorized);
        response->prepare_payload();
        return response;
    }
    auto userIdPair = validateTokenAndGetUserId(toketOpt.value());
    if (userIdPair.first != ErrorCode::None)
    {
        // Handle authentication failure
        HttpResponseBuilder::buildJsonResponseForError(*response, userIdPair.first, http::status::unauthorized);
        response->prepare_payload();
        return response;
    }

    if (url.find("/logout") != std::string::npos)
    {
        bool isLogout = dataService.deleteSessionByToken(toketOpt.value());

        isLogout ? HttpResponseBuilder::buildJsonResponseForData(*response, "Loged out") : HttpResponseBuilder::buildJsonResponseForError(*response, ErrorCode::LogoutError);

        response->prepare_payload();
        return response;
    }

    DataType type = determineDataType(url);
    auto handler = getHandlerForType(type);

    int userId = userIdPair.second;

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
            HttpResponseBuilder::buildJsonResponseForError(*response, ErrorCode::MethodNotAllowed, http::status::method_not_allowed);
        }

        response->prepare_payload();
        return response;
    }
    catch (const std::exception &e)
    {
        HttpResponseBuilder::buildJsonResponseForError(*response, ErrorCode::Undefined, http::status::internal_server_error);
        response->prepare_payload();
        return response;
    }
}

void ServerLogic::handleGoogleSignIn(http::request<http::string_body> &request, http::response<http::string_body> &response)
{
    auto idTokenOpt = getTokenFromRequest(request);
    if (!idTokenOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::TokenIsNotExist, http::status::bad_request);
        return;
    }

    // TODO add user info get method
    auto userInfoOpt = verifyGoogleToken(idTokenOpt.value());
    if (!userInfoOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::TokenIsInvalid, http::status::unauthorized);
        return;
    }

    // Process user info, check if user exists, create new user if not
    auto userSessionResultPair = dataService.createOrUpdateGoogleUserSession(userInfoOpt.value());
    if (userSessionResultPair.first == ErrorCode::None)
    {
        HttpResponseBuilder::buildJsonResponseForData(response, userSessionResultPair.second);
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, userSessionResultPair.first, http::status::internal_server_error);
    }
}

void ServerLogic::handleLoginRequest(http::request<http::string_body> &request, http::response<http::string_body> &response)
{
    auto loginResultPair = dataService.loginUser(request.body());

    if (loginResultPair.first == ErrorCode::None)
    {
        HttpResponseBuilder::buildJsonResponseForData(response, loginResultPair.second);
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, loginResultPair.first, http::status::unauthorized);
    }
}

void ServerLogic::handleSignupRequest(http::request<http::string_body> &request, http::response<http::string_body> &response)
{
    auto signupResultPair = dataService.signupUser(request.body());

    if (signupResultPair.first == ErrorCode::None)
    {
        HttpResponseBuilder::buildJsonResponseForData(response, signupResultPair.second);
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, signupResultPair.first, http::status::unauthorized);
    }
}

void ServerLogic::handleTokenRefreshRequest(http::request<http::string_body> &request, http::response<http::string_body> &response)
{
    auto refreshTokenOpt = getTokenFromRequest(request);
    if (!refreshTokenOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::TokenIsNotExist, http::status::unauthorized);
        return;
    }

    auto newSessionOpt = dataService.refreshUserSession(refreshTokenOpt.value());
    if (!newSessionOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForError(response, ErrorCode::TokenExpired, http::status::unauthorized);
        return;
    }

    HttpResponseBuilder::buildJsonResponseForData(response, newSessionOpt.value());
}

std::pair<ErrorCode, int> ServerLogic::validateTokenAndGetUserId(const std::string &token)
{
    auto session = dataService.getSessionByToken(token);
    if (!session.has_value())
    {
        // Token does not exist
        return std::make_pair(ErrorCode::TokenIsNotExist, 0);
    }
    if (Utility::isExpiredTime(session.value().expiration))
    {
        return std::make_pair(ErrorCode::TokenExpired, 0);
    }

    return std::make_pair(ErrorCode::None, session.value().userId);
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

std::optional<GoogleUserInfo> ServerLogic::verifyGoogleToken(const std::string &idToken)
{
    try
    {
        // Verify the token signature and claims
        auto decoded = jwt::decode(idToken);

        // Verify the issuer and audience
        auto issuer = decoded.get_issuer();
        auto audience = decoded.get_audience();

        if (issuer != "https://accounts.google.com" && issuer != "accounts.google.com")
        {
            std::cerr << "Invalid issuer." << std::endl;
            return std::nullopt;
        }

        bool audienceContainsPrefix = false;
        for (const auto &aud : audience)
        {
            if (aud.find("apps.googleusercontent.com") != std::string::npos)
            {
                audienceContainsPrefix = true;
                break; // Stop searching once we find a match
            }
        }

        if (!audienceContainsPrefix)
        {
            std::cerr << "Invalid audience." << std::endl;
            return std::nullopt;
        }

        // Extract user information
        GoogleUserInfo userInfo;
        userInfo.googleId = decoded.get_payload_claim("sub").as_string();
        userInfo.name = userInfo.googleId; //as name is unique now //decoded.get_payload_claim("name").as_string();

        return userInfo;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to verify Google ID Token: " << e.what() << std::endl;
        return std::nullopt;
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