#pragma once

#include <stdexcept>

class FileReadError : public std::invalid_argument
{
public:
    FileReadError() : std::invalid_argument("Unable to read file") { };
};

