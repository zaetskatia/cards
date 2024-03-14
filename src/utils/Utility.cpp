// Utility.cpp
#include "Utility.h"
#include <sstream>
#include <vector>
#include <algorithm>

std::optional<int> Utility::extractIdFromURL(const std::string &url, const std::string &segmentName)
{
    std::vector<std::string> segments;
    std::stringstream urlStream(url);
    std::string segment;

    while (std::getline(urlStream, segment, '/'))
    {
        segments.push_back(segment);
    }

    auto it = std::find(segments.begin(), segments.end(), segmentName);
    if (it != segments.end() && ++it != segments.end())
    {
        try
        {
            return std::stoi(*it);
        }
        catch (const std::invalid_argument &e)
        {
            // Handle invalid argument exception if conversion fails
        }
        catch (const std::out_of_range &e)
        {
            // Handle out of range exception if conversion fails
        }
    }

    return std::nullopt;
}

std::optional<int> Utility::extractQueryParameter(const std::string &query, const std::string &paramName)
{
    std::size_t paramPos = query.find(paramName + "=");
    if (paramPos != std::string::npos)
    {
        std::size_t valueStart = paramPos + paramName.length() + 1;
        std::size_t valueEnd = query.find("&", valueStart);
        std::string paramValue = query.substr(valueStart, valueEnd - valueStart);
        try
        {
            return std::stoi(paramValue);
        }
        catch (const std::invalid_argument &e)
        {
            return std::nullopt;
        }
    }
    return std::nullopt;
}

std::string Utility::calculateExpirationDateTime(int hours)
{
    auto now = std::chrono::system_clock::now();
    auto expiration = now + std::chrono::hours(hours);
    return timePointToString(expiration);
}

std::string Utility::generateSessionToken()
{
    auto randchar = []() -> char
    {
        const char charset[] =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        const size_t maxIndex = (sizeof(charset) - 1);
        return charset[rand() % maxIndex];
    };
    std::string token(64, 0);
    std::generate_n(token.begin(), 64, randchar);
    return token;
}

std::string Utility::timePointToString(const std::chrono::system_clock::time_point &time_point)
{
    auto time_t = std::chrono::system_clock::to_time_t(time_point);
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%FT%TZ"); // ISO 8601 format in UTC
    return oss.str();
}

std::chrono::system_clock::time_point Utility::stringToTimePoint(const std::string &time_str)
{
    std::tm tm = {};
    std::istringstream iss(time_str);
    iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ"); // Matching ISO 8601 format in UTC
    auto time_t = std::mktime(&tm);                  // Assumes local time. For UTC, consider a different approach or library.
    return std::chrono::system_clock::from_time_t(time_t);
}