#include "../include/browser.hpp"
#include "../include/utils.hpp"
#include <cassert>
#include <cstdint>
#include <ncurses.h>
#include <string>
#include <unctrl.h>
#include <utility>
#include <vector>

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
    std::string redirInput;
    std::string openOtherInput;
    std::string userInput;
    std::string metaLine;
    bool toLowest = false;
    int columns;
    int lines;
};

bool mainLoop(DrawState& ds, Browser& b, int input, int cols, int lines);
void tryVisitSite(DrawState& ds , Browser& b, std::string site);
