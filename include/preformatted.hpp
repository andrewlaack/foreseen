#pragma once
#include "line.hpp"
#include <cstdint>

class Preformatted : public Line {
    private:
        std::string text = "";
    public:
        Preformatted(std::string text);
        std::string textToDraw() override;
        uint8_t getColor() override;
        LineType type() override;
        bool isBold() override;
};
