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