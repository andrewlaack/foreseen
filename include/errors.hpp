#pragma once

#include <stdexcept>

class NotImplemented : public std::logic_error
{
public:
    NotImplemented() : std::logic_error("Function not yet implemented") { };
};


class BrowserStartError : public std::logic_error {
    public:
        BrowserStartError() : std::logic_error("An issue occurred while creating the browser") { };
};

class FileReadError : public std::invalid_argument
{
public:
    FileReadError() : std::invalid_argument("Unable to read file") { };
};

