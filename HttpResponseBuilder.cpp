
#include "HttpResponseBuilder.h"

void HttpResponseBuilder::buildJsonResponseForData(http::response<http::string_body>& response, const json &data, http::status status)
{
    nlohmann::json responseJson = {
        {"status", "success"},
        {"data", data}
        };
        
        return buildJsonResponse(response, responseJson, status);
}

void HttpResponseBuilder::buildJsonResponseForError(http::response<http::string_body>& response, const std::string &errorMessage, http::status status)
{
    nlohmann::json responseJson = {
            {"status", "error"},
            {"message", errorMessage}
        };

        return buildJsonResponse(response, responseJson, status);
}

void HttpResponseBuilder::buildJsonResponse(http::response<http::string_body>& response, const json &jsonResponse, http::status status)
{
    response.result(status);
    response.set(http::field::content_type, "application/json");
    response.body() = jsonResponse.dump();

}
