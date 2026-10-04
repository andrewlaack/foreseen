#include "../include/site.hpp"

#include <algorithm>
#include <cstdint>
#include <string>

Site::Site(std::string h, std::string b) : body(b), header(h) {}

std::string Site::getHeader() { return header; }

std::string Site::getBody() { return body; }
uint32_t Site::getStatusCode() {
    try {
        uint32_t statusCode = std::stoi(header.substr(0, 2));
        return statusCode;
    } catch (...) {
        return uint32_t{41};
    }
}

std::string Site::getMeta() {
    std::string meta = header.substr(3);
    std::replace(meta.begin(), meta.end(), '\n', ' ');
    return meta;
}

void Site::setUnreachable() { unreachable = true; }

bool Site::getUnreachable() { return unreachable; }
