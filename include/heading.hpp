#pragma once

#include <optional>
#include <string>
#include "../vendor/uri.hpp"
#include "line.hpp"

class Heading : public Line {
        int headingLevel = 1;
        std::string actualText;
        std::string toDraw;
    public:
        Heading(std::string text);
        std::string textToDraw() override;
        int getColor() override;
        LineType type() override;
        bool isBold() override;
};
