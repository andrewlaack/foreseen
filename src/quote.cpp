#include "../include/quote.hpp"
#include "../include/utils.hpp"
#include <ncurses.h>

Quote::Quote(std::string input) {
    textToRender = input.substr(1);
    textToRender = stripLeadingWhiteSpace(textToRender);
    textToRender = "> " + textToRender;
}

std::string Quote::textToDraw() {
    return textToRender + "\n";
}

int Quote::getColor() {
    return COLOR_MAGENTA;
}

LineType Quote::type() {
    return QUOTE;
}

bool Quote::isBold() {
    return false;
}
