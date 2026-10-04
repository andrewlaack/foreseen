#include "../include/quote.hpp"

#include <ncurses.h>

#include <cstdint>

#include "../include/shared.hpp"
#include "../include/utils.hpp"

Quote::Quote(std::string input) {
    textToRender = input.substr(1);
    textToRender = stripLeadingWhiteSpace(textToRender);
    textToRender = "> " + textToRender;
}

std::string Quote::textToDraw() { return textToRender + "\n"; }

uint8_t Quote::getColor() { return COLOR_QUOTE; }

LineType Quote::type() { return QUOTE; }

bool Quote::isBold() { return false; }
