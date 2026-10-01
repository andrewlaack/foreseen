#include "../include/plaintext.hpp"
#include <cstdint>

Plaintext::Plaintext(std::string input) : text(input) {}

std::string Plaintext::textToDraw() {
    return text + "\n";
}

uint8_t Plaintext::getColor() {
    return 15;
}

LineType Plaintext::type() {
    return PLAINTEXT;
}

bool Plaintext::isBold() {
    return false;
}
