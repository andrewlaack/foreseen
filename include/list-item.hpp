#pragma once
#include "line.hpp"
#include <cstdint>

class ListItem : public Line {
    private:
        std::string textToRender;
    public:
        ListItem(std::string text);
        std::string textToDraw() override;
        uint8_t getColor() override;
        LineType type() override;
        bool isBold() override;
};
