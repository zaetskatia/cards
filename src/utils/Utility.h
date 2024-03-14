#pragma once

#include <string>
#include <optional>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <random>
#include <algorithm>

struct Utility
{
    static std::optional<int> extractIdFromURL(const std::string &url, const std::string &segmentName);

    static std::optional<int> extractQueryParameter(const std::string &query, const std::string &paramName);

    static std::string calculateExpirationDateTime(int hours);

    // TODO later use token generation from lib
    static std::string generateSessionToken();

    static std::string timePointToString(const std::chrono::system_clock::time_point &time_point);

    static std::chrono::system_clock::time_point stringToTimePoint(const std::string &time_str);
};