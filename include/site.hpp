#pragma once

#include <cstdint>
#include <string>

class Site{
    private:
        std::string body;
        std::string header;
        bool unreachable = false;
    public:
        Site(std::string header, std::string body);
        std::string getBody();
        uint32_t getStatusCode();
        std::string getHeader();
        std::string getMeta();
        void setUnreachable();
        bool getUnreachable();
};
