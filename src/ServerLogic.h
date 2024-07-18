#pragma once

#include <unordered_map>
#include <string>
#include <optional>
#include "handlers/CardHandler.h"
#include "handlers/FolderHandler.h"
#include "services/DataService.h"
#include "utils/HttpResponseBuilder.h"

class ServerLogic
{
public:
    ServerLogic(DataService &dataService) : dataService(dataService) {}

    std::shared_ptr<http::response<http::string_body>> handleRequest(http::request<http::string_body> &request);

private:
    std::unique_ptr<IEntityHandler> getHandlerForType(DataType type);
    DataType determineDataType(const std::string &url);

    void handleGoogleSignIn(http::request<http::string_body> &request, http::response<http::string_body> &response);
    void handleLoginRequest(http::request<http::string_body> &request, http::response<http::string_body> &response);
    void handleSignupRequest(http::request<http::string_body> &request, http::response<http::string_body> &response);
    void handleTokenRefreshRequest(http::request<http::string_body>& request, http::response<http::string_body>& response);

    std::pair<ErrorCode,int> validateTokenAndGetUserId(const std::string& token);
    std::optional<std::string> getTokenFromRequest(const http::request<http::string_body>& request);
    std::optional<GoogleUserInfo> verifyGoogleToken(const std::string& idToken);

    DataService &dataService;
};
