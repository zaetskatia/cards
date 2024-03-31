#pragma once

enum class DataType
{
    None = 0,
    Card,
    Folder
};

constexpr size_t maxTermLength = 30;
constexpr size_t maxTranslationLength = 30;
constexpr size_t maxFolderNameLength = 30;
constexpr size_t maxUsernameLength = 30;
constexpr size_t maxPasswordLength = 30;