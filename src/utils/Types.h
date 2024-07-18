#pragma once

enum class DataType
{
    None = 0,
    Card,
    Folder
};

enum class ErrorCode
{
    None = 0,
    Undefined,
    UserNotFound,
    MaxLenReached,
    PassDoNotMatch,
    TokenInsertionFailure, 
    UsernameAlreadyExists,
    UserCreationFailure,
    TokenIsNotExist,
    TokenIsInvalid,
    TokenExpired,
    LogoutError,
    MethodNotAllowed,
    NoFolderId,
    CardNotFound,
    FailedToCreateCard,
    InvalidRequestParameters,
    FolderNotFound,
    FailedToCreateFolder,
    FailedToUpdateFolder,
    InvalidFolderId,
    FailedToDeleteFolder,
    FolderIsEmpty
};

constexpr size_t maxTermLength = 30;
constexpr size_t maxTranslationLength = 30;
constexpr size_t maxFolderNameLength = 30;
constexpr size_t maxUsernameLength = 30;
constexpr size_t maxPasswordLength = 30;