#include "../include/preformatted.hpp"

#include <ncurses.h>

#include <cstdint>

#include "../include/shared.hpp"

Preformatted::Preformatted(std::string input) : text(input) {}

std::string Preformatted::textToDraw() { return text + "\n"; }

uint8_t Preformatted::getColor() { return COLOR_PREFORMATTED; }

LineType Preformatted::type() { return PREFORMATTED; }

bool Preformatted::isBold() { return false; }
