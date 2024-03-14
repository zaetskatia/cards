#include "FolderHandler.h"

void FolderHandler::handleGetRequest(const http::request<http::string_body> &request,
                                     http::response<http::string_body> &response,
                                     int userId)
{
    auto folderIdOpt = Utility::extractIdFromURL(request.target().to_string(), "folders");
    if (folderIdOpt)
    {
        auto folderOpt = dataService->getFolder(folderIdOpt.value(), userId);
        if (folderOpt)
        {
            HttpResponseBuilder::buildJsonResponseForData(response, folderOpt.value());
        }
        else
        {
            HttpResponseBuilder::buildJsonResponseForError(response, "Folder not found", http::status::not_found);
        }
    }
    else
    {
        auto foldersOpt = dataService->getAllFolders(userId);
        if (foldersOpt.has_value())
        {
            HttpResponseBuilder::buildJsonResponseForData(response, foldersOpt.value());
        }
        else
        {
            HttpResponseBuilder::buildJsonResponseForError(response, "No folders found", http::status::not_found);
        }
    }
}

void FolderHandler::handlePostRequest(const http::request<http::string_body> &request,
                                      http::response<http::string_body> &response,
                                      int userId)
{
    auto createResultOpt = dataService->insertFolder(request.body(), userId);
    if (createResultOpt.has_value())
    {
        HttpResponseBuilder::buildJsonResponseForData(response, createResultOpt.value());
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Failed to create folder", http::status::internal_server_error);
    }
}

void FolderHandler::handlePutRequest(const http::request<http::string_body> &request,
                                     http::response<http::string_body> &response,
                                     int userId)
{
    auto folderIdOpt = Utility::extractIdFromURL(request.target().to_string(), "folders");
    if (folderIdOpt.has_value())
    {
        bool updateSuccess = dataService->updateFolder(folderIdOpt.value(), request.body(), userId);
        if (updateSuccess)
        {
            nlohmann::json successJson;
            successJson["message"] = "Folder updated successfully";
            HttpResponseBuilder::buildJsonResponseForData(response, successJson);
        }
        else
        {
            HttpResponseBuilder::buildJsonResponseForError(response, "Failed to update folder", http::status::not_found);
        }
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Invalid folder ID", http::status::bad_request);
    }
}

void FolderHandler::handleDeleteRequest(const http::request<http::string_body> &request,
                                        http::response<http::string_body> &response,
                                        int userId)
{
    auto folderIdOpt = Utility::extractIdFromURL(request.target().to_string(), "folders");
    if (folderIdOpt.has_value())
    {
        bool deletionSuccess = dataService->deleteFolder(folderIdOpt.value(), userId);
        if (deletionSuccess)
        {
            nlohmann::json successJson;
            successJson["message"] = "Folder deleted successfully";
            HttpResponseBuilder::buildJsonResponseForData(response, successJson);
        }
        else
        {
            HttpResponseBuilder::buildJsonResponseForError(response, "Failed to delete folder", http::status::not_found);
        }
    }
    else
    {
        HttpResponseBuilder::buildJsonResponseForError(response, "Invalid folder ID", http::status::bad_request);
    }
}
