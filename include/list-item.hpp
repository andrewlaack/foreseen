#pragma once
#include "line.hpp"

class ListItem : public Line {
    private:
        std::string textToRender;
    public:
        ListItem(std::string text);
        std::string textToDraw() override;
        int getColor() override;
        LineType type() override;
        bool isBold() override;
};
