#pragma once

#include <unordered_map>
#include <string>
#include <optional>
#include "handlers/CardHandler.h"
#include "handlers/FolderHandler.h"
#include "services/DataService.h"
#include "utils/HttpResponseBuilder.h"
#include "utils/Types.h"

class ServerLogic
{
public:
    ServerLogic(DataService &dataService) : dataService(dataService) {}

    std::shared_ptr<http::response<http::string_body>> handleRequest(http::request<http::string_body> &request);

private:
    std::unique_ptr<IEntityHandler> getHandlerForType(DataType type);
    DataType determineDataType(const std::string &url);
    static std::optional<std::string> extractClientId(const http::request<http::string_body> &request);

    DataService &dataService;
};
