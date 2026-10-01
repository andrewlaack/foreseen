#include "../include/preformatted.hpp"
#include "../include/shared.hpp"
#include <cstdint>
#include <ncurses.h>

Preformatted::Preformatted(std::string input) : text(input) {}

std::string Preformatted::textToDraw() {
    return text + "\n";
}

uint8_t Preformatted::getColor() {
    return COLOR_PREFORMATTED;
}

LineType Preformatted::type() {
    return PREFORMATTED;
}

bool Preformatted::isBold() {
    return false;
}

