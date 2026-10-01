#include "../include/list-item.hpp"
#include "../include/utils.hpp"
#include <cstdint>

ListItem::ListItem(std::string input) {
    textToRender = input.substr(1);
    textToRender = stripLeadingWhiteSpace(textToRender);
    textToRender = "* " + textToRender;
}

std::string ListItem::textToDraw() {
    return textToRender + "\n";
}

uint8_t ListItem::getColor() {
    return 15;
}

LineType ListItem::type() {
    return LIST_ITEM;
}

bool ListItem::isBold() {
    return false;
}
