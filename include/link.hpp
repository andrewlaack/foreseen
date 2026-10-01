#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include "../vendor/uri.hpp"
#include "line.hpp"

class Link : public Line {
    private:
        bool invalid = false;
        int linkNumber = -1;
        std::string renderedText;
        uri linkDestination = uri("gemini://example.com");
        std::optional<std::string> linkText;
        uri parseDestination(std::string destination, std::optional<uri> prior);
    public:
        Link(std::string text, std::optional<uri> prior = std::nullopt, int linkNumber = -1);
        uri getLinkDestination();
        std::optional<std::string> getLinkText();
        std::string textToDraw() override;
        uint8_t getColor() override;
        bool isBold() override;
        LineType type() override;
};
