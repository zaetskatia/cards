#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/core.hpp>
#include "utils/Types.h"

namespace http = boost::beast::http;
using json = nlohmann::json;

class HttpResponseBuilder
{
public:
    static void buildJsonResponseForData(
        http::response<http::string_body> &response,
        const json &data,
        http::status status = http::status::ok);

    static void buildJsonResponseForError(
        http::response<http::string_body> &response,
        const ErrorCode errorMessage,
        http::status status = http::status::internal_server_error);

private:
    static void buildJsonResponse(
        http::response<http::string_body> &response,
        const json &jsonResponse,
        http::status status);
};
