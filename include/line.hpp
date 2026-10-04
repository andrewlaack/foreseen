#pragma once

#include <cstdint>
#include <string>

enum LineType {
    LINK,
    PLAINTEXT,
    H1,
    H2,
    H3,
    FORMAT_SWITCH,
    PREFORMATTED,
    QUOTE,
    LIST_ITEM
};

class Line {
   public:
    virtual std::string textToDraw() = 0;
    virtual uint8_t getColor() = 0;
    virtual LineType type() = 0;
    virtual ~Line() = default;
    virtual bool isBold() = 0;
};
