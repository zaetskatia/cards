#pragma once

#include <string>
#include <optional>

struct Utility
{
    static std::optional<int> extractIdFromURL(const std::string &url, const std::string &segmentName);

    static std::optional<int> extractQueryParameter(const std::string &query, const std::string &paramName);
};