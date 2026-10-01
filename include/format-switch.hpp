#pragma once
#include "line.hpp"
#include <cstdint>

class FormatSwitch : public Line {
    private:
        std::string text = "";
    public:
        FormatSwitch(std::string text);
        std::string textToDraw() override;
        uint8_t getColor() override;
        LineType type() override;
        bool isBold() override;
};
