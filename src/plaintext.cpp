#include "../include/plaintext.hpp"

#include <cstdint>

#include "../include/shared.hpp"

Plaintext::Plaintext(std::string input) : text(input) {}

std::string Plaintext::textToDraw() { return text + "\n"; }

uint8_t Plaintext::getColor() { return COLOR_PLAINTEXT; }

LineType Plaintext::type() { return PLAINTEXT; }

bool Plaintext::isBold() { return false; }
