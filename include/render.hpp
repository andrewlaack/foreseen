#include <ncurses.h>
#include <unctrl.h>

#include <cassert>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "../include/browser.hpp"
#include "../include/utils.hpp"

#pragma once

struct DrawState {
    std::vector<std::pair<std::string, TextRender>> prior;
    std::vector<std::pair<std::string, TextRender>> broken;
    bool mustReRender = true;
    Browser* bPtr;
    std::string header;
    int y;
    bool handleInput;
    bool handleOpenOther;
    bool handleRedirect;
    bool reBreak = true;
    std::string issueText;
    uint64_t timeToClearIssueText;
    uint64_t timeToClearBottomText;
    std::string bottomText;
    std::string redirInput;
    std::string openOtherInput;
    std::string userInput;
    std::string metaLine;
    bool toLowest = false;
    int columns;
    int lines;
};

bool mainLoop(DrawState& ds, Browser& b, int input, int cols, int lines);
void tryVisitSite(DrawState& ds, Browser& b, std::string site);
