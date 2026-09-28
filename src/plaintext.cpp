#include "../include/plaintext.hpp"

Plaintext::Plaintext(std::string input) : text(input) {}

std::string Plaintext::textToDraw() {
    return text + "\n";
}

int Plaintext::getColor() {
    return 15;
}

LineType Plaintext::type() {
    return PLAINTEXT;
}

bool Plaintext::isBold() {
    return false;
}
