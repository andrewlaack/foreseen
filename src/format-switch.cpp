#include "../include/format-switch.hpp"

#include <ncurses.h>

#include <cstdint>

#include "../include/shared.hpp"

FormatSwitch::FormatSwitch(std::string input) : text(input) {}

std::string FormatSwitch::textToDraw() { return ""; }

uint8_t FormatSwitch::getColor() { return COLOR_FORMAT_SWITCH; }

LineType FormatSwitch::type() { return FORMAT_SWITCH; }

bool FormatSwitch::isBold() { return false; }
