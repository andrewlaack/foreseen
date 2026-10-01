#include "../include/format-switch.hpp"
#include <cstdint>
#include <ncurses.h>

FormatSwitch::FormatSwitch(std::string input) : text(input) {}

std::string FormatSwitch::textToDraw() {
    return "";
}

uint8_t FormatSwitch::getColor() {
    return COLOR_WHITE;
}

LineType FormatSwitch::type() {
    return FORMAT_SWITCH;
}

bool FormatSwitch::isBold() {
    return false;
}

