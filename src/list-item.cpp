#include "../include/list-item.hpp"
#include "../include/utils.hpp"
#include "../include/shared.hpp"
#include <cstdint>

ListItem::ListItem(std::string input) {

//	list-item        = "*" SP text-line
// notice that a space is required along with a *. There'd be no reason to remove
// them and then add them back here, so we just pass everything through.

    textToRender = input;
}

std::string ListItem::textToDraw() {
    return textToRender + "\n";
}

uint8_t ListItem::getColor() {
    return COLOR_LIST_ITEM;
}

LineType ListItem::type() {
    return LIST_ITEM;
}

bool ListItem::isBold() {
    return false;
}
