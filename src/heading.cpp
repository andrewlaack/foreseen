#include "../include/heading.hpp"
#include "../include/utils.hpp"
#include "../include/shared.hpp"
#include <cstdint>
#include <ncurses.h>
#include <stdexcept>

Heading::Heading(std::string text) : actualText(text) {

    // TODO: Actually handle whitespace correctly here.
    if(text.substr(0,3) == "###") {
        headingLevel = 3;
        toDraw = actualText.substr(3);
    }

    else if(text.substr(0,2) == "##") {
        headingLevel = 2;
        toDraw = actualText.substr(2);
    }
    else if(text.substr(0,1) == "#") {
        headingLevel = 1;
        toDraw = actualText.substr(1);
    }

    toDraw = stripLeadingWhiteSpace(toDraw);
}

std::string Heading::textToDraw() {
    switch (headingLevel) {
        case 3:
            return "### " + toDraw + "\n";
        case 2:
            return "## " + toDraw + "\n";
        case 1:
            return "# " + toDraw + "\n";
    }
    throw std::runtime_error("This is an invalid program state.");
}
uint8_t Heading::getColor() {
    switch (headingLevel) {
        case 3:
            return COLOR_H3;
        case 2:
            return COLOR_H2;
        case 1:
            return COLOR_H1;
    }
    throw std::runtime_error("This is an invalid program state.");
}
LineType Heading::type() {
    switch (headingLevel) {
        case 3:
            return H3;
        case 2:
            return H2;
        case 1:
            return H1;
    }
    throw std::runtime_error("This is an invalid program state.");
}
bool Heading::isBold() {
    return true;
}
