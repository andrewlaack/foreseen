#pragma once
#include <cstdint>

#include "line.hpp"

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
