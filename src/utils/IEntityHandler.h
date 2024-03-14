#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include "Utility.h"
#include "HttpResponseBuilder.h"
#include "services/DataService.h"

namespace beast = boost::beast;
namespace http = beast::http;

class IEntityHandler
{
public:
    virtual void handleGetRequest(const http::request<http::string_body> &request,
                                  http::response<http::string_body> &response,
                                  int userId) = 0;

    virtual void handlePostRequest(const http::request<http::string_body> &request,
                                   http::response<http::string_body> &response,
                                   int userId) = 0;

    virtual void handlePutRequest(const http::request<http::string_body> &request,
                                  http::response<http::string_body> &response,
                                  int userId) = 0;

    virtual void handleDeleteRequest(const http::request<http::string_body> &request,
                                     http::response<http::string_body> &response,
                                     int userId) = 0;

    virtual ~IEntityHandler() = default;
};
