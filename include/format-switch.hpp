#pragma once
#include <cstdint>

#include "line.hpp"

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
