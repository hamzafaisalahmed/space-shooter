#pragma once

#include <stdexcept>
#include <string>

class AssetLoadException : public std::runtime_error
{
public:
    AssetLoadException(const std::string &msg) : std::runtime_error(msg) {}
};

class InvalidLevelException : public std::runtime_error
{
public:
    InvalidLevelException(const std::string &msg) : std::runtime_error(msg) {}
};
