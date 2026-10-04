#include "../include/link.hpp"

#include <ncurses.h>

#include <cstdint>
#include <optional>
#include <string>

#include "../include/shared.hpp"
#include "../include/utils.hpp"
#include "../vendor/uri.hpp"

Link::Link(std::string text, std::optional<uri> prior, int linkNumber) {
    this->linkNumber = linkNumber;

    if (text.substr(0, 2) != "=>") {
        linkDestination = parseDestination(text, prior);
        return;
    }

    std::string afterPrefix = text;

    afterPrefix = text.substr(2, text.size());

    int highest = -1;

    for (int i = 0; i < (int)afterPrefix.size(); ++i) {
        if (!(isWhiteSpace(afterPrefix, i))) {
            break;
        }
        highest = i;
    }

    std::string afterWhite =
        afterPrefix.substr(highest + 1, afterPrefix.size());

    int nextWhite = std::min(afterWhite.find(' '), afterWhite.find('\t'));

    if (nextWhite != -1) {
        linkText = afterWhite.substr(nextWhite + 1);

        std::string destination = afterWhite.substr(0, nextWhite);

        linkDestination = parseDestination(destination, prior);

    } else {
        linkDestination = parseDestination(afterWhite, prior);
    }

    if (linkText != std::nullopt) {
        std::string tmp = linkText.value();
        std::string rdlt = stripLeadingWhiteSpace(tmp);

        renderedText = "[" + std::to_string(linkNumber) + "] " + rdlt;
    } else {
        std::string tmp = linkDestination.to_string();
        std::string rdlt = stripLeadingWhiteSpace(tmp);
        renderedText = "[" + std::to_string(linkNumber) + "] " +
                       linkDestination.to_string();
    }
}

uri Link::parseDestination(std::string destination, std::optional<uri> prior) {
    // TODO: Not sure if this is right w/ how file paths work.

    // TODO: There's a mess here to untangle. Like I think there might be some
    // relative links that'll get fucked by this. despite  that, this is a
    // requirement to get search parameters working correctly which are probably
    // more important.

    if (destination.find(":") == std::string::npos || destination[0] == '?') {
        if (prior != std::nullopt) {
            // query parameter special casing
            if (destination.substr(0, 1) == "?") {
                std::string encoded = urlEncode(destination.substr(1));
                return uri{prior.value().to_string() + "?" + encoded};
            }

            std::string base = prior->get_scheme() + "://" + prior->get_host();

            if (destination[0] == '/') {
                destination = base + destination;
            } else {
                std::string path = prior->get_path();
                if (path[0] != '/') {
                    path = "/" + path;
                }
                destination =
                    base + path.substr(0, path.rfind('/') + 1) + destination;
            }
        }
    }
    try {
        return uri{destination};
    } catch (...) {
        invalid = true;
        return uri{"gemini://broken-link.com"};  // TODO: Handle this better
    }
}

uri Link::getLinkDestination() { return linkDestination; }
std::optional<std::string> Link::getLinkText() { return linkText; }

std::string Link::textToDraw() { return renderedText + "\n"; }

uint8_t Link::getColor() { return COLOR_LINK; }

LineType Link::type() { return LINK; }

bool Link::isBold() { return false; }
