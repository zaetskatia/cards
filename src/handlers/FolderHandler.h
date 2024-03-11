#pragma once

#include "utils/IEntityHandler.h"

class FolderHandler : public IEntityHandler
{
public:
    FolderHandler(DataService* ds) : dataService(ds) {}
    void handleGetRequest(const http::request<http::string_body> &request,
                          http::response<http::string_body> &response,
                          const std::string &clientId) override;

    void handlePostRequest(const http::request<http::string_body> &request,
                           http::response<http::string_body> &response,
                           const std::string &clientId) override;

    void handlePutRequest(const http::request<http::string_body> &request,
                          http::response<http::string_body> &response,
                          const std::string &clientId) override;

    void handleDeleteRequest(const http::request<http::string_body> &request,
                             http::response<http::string_body> &response,
                             const std::string &clientId) override;

private:
    DataService* dataService;
};
