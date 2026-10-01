#pragma once
#include "line.hpp"
#include <cstdint>

class Quote : public Line {
    private:
        std::string textToRender;
    public:
        Quote(std::string text);
        std::string textToDraw() override;
        uint8_t getColor() override;
        LineType type() override;
        bool isBold() override;
};
