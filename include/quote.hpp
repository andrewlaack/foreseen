#pragma once
#include "line.hpp"

class Quote : public Line {
    private:
        std::string textToRender;
    public:
        Quote(std::string text);
        std::string textToDraw() override;
        int getColor() override;
        LineType type() override;
        bool isBold() override;
};
