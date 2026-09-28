#include "../include/list-item.hpp"
#include "../include/utils.hpp"

ListItem::ListItem(std::string input) {
    textToRender = input.substr(1);
    textToRender = stripLeadingWhiteSpace(textToRender);
    textToRender = "* " + textToRender;
}

std::string ListItem::textToDraw() {
    return textToRender + "\n";
}

int ListItem::getColor() {
    return 15;
}

LineType ListItem::type() {
    return LIST_ITEM;
}

bool ListItem::isBold() {
    return false;
}
