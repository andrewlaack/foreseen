#include "../include/heading.hpp"
#include "../include/errors.hpp"
#include "../include/utils.hpp"
#include <ncurses.h>

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
            return "### " + toDraw;
        case 2:
            return "## " + toDraw;
        case 1:
            return "# " + toDraw;
        default:
            throw NotImplemented();
    }
}
int Heading::getColor() {
    switch (headingLevel) {
        case 3:
            return COLOR_CYAN;
        case 2:
            return COLOR_GREEN;
        case 1:
            return COLOR_RED;
        default:
            throw NotImplemented();
    }
}
LineType Heading::type() {
    switch (headingLevel) {
        case 3:
            return H3;
        case 2:
            return H2;
        case 1:
            return H1;
        default:
            throw NotImplemented();
    }
}
bool Heading::isBold() {
    return true;
}
