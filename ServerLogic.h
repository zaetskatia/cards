#pragma once

#ifndef SERVER_LOGIC_HPP
#define SERVER_LOGIC_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <unordered_map>
#include <string>
#include <optional>
#include "DataService.h"
#include "HttpResponseBuilder.h"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

class ServerLogic
{
public:
    // Constructor accepts an interface pointer
    ServerLogic(DataService &dataService) : dataService(dataService) {}

    std::shared_ptr<http::response<http::string_body>> handleRequest(http::request<http::string_body> &request);

private:
    void handlePostRequest(http::request<http::string_body> &request, http::response<http::string_body> &response, std::string clientId);
    void handleGetAllCardsRequest(http::request<http::string_body> &request,
                                  http::response<http::string_body> &response,
                                  std::string clientId);
    void handleGetRequest(http::request<http::string_body> &request, http::response<http::string_body> &response, std::string clientId);
    void handlePutRequest(http::request<http::string_body> &request, http::response<http::string_body> &response, std::string clientId);
    void handleDeleteRequest(http::request<http::string_body> &request, http::response<http::string_body> &response, std::string clientId);

    std::optional<std::string> extractClientId(const http::request<http::string_body> &request);
    std::optional<int> extractCardIdFromURL(const std::string &url);

    DataService &dataService;
};

#endif // SERVER_LOGIC_HPP
