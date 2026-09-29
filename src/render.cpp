#include "../include/browser.hpp"
#include "../include/utils.hpp"
#include "../include/render.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <ncurses.h>
#include <locale.h>
#include <string>
#include <unctrl.h>
#include <utility>
#include <vector>

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

int lowestPos(std::vector<std::pair<std::string, TextRender>>& strLs) {
    return (strLs.size() - (LINES - 2)) + 1; // this gives us two new lines at the end because the last line should contain a \n.
}

bool isValidUserInput(int uinput) {
    if ((uinput >= 0x20 && uinput <= 0x7E)) {
        return true;
    }
    return false;
}
void initColors() {
    if(has_colors()) {
        start_color();
        use_default_colors();
        for (int c = 0; c < COLORS && c+1 < COLOR_PAIRS; ++c) {
            init_pair(c + 1, c, -1);
        }
    }
}

void drawInputBox(std::string text, std::string userInput) {

    move(LINES/2-1, COLS/4);

    attron(COLOR_PAIR(COLOR_CYAN+1));
    for(int i = 0; i < COLS/2; ++i) {
        addstr("-");
    }

    move(LINES/2 + 1, COLS/4);
    for(int i = 0; i < COLS/2; ++i) {
        addstr("-");
    }
    attroff(COLOR_PAIR(COLOR_CYAN+1));

    int userInputSize = userInput.size();
    int width = COLS/2;

    if((int)text.size() >= width) {
        text = text.substr(0,width-6) + "...: ";
    }

    int textSize = text.size();

    assert(textSize < width);

    std::string userTextToRender = userInput;

    int delta = width - (textSize + userInputSize);

    if(delta < 0) {
        userTextToRender = userInput.substr(delta*-1, userInput.size());
    } else {
        while(delta != 0) {
            userTextToRender.append(" "); // this makes sure the background doesn't leak through.
            delta -= 1;
        }
    }

    move(LINES/2, COLS/4);
    addstr(text.c_str());
    addstr(userTextToRender.c_str());

}
uint64_t getCurrentTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void draw(DrawState& ds) {
    if(COLS < 20) {
        erase();
        addstr("Screen width too small.");
        return;
    }
    if(LINES < 3) {
        erase();
        addstr("Screen height too small.");
        return;
    }

    auto* cs = ds.bPtr->getCurrentSite();

    if (!(cs != nullptr && (cs->getStatusCode() < 20 || cs->getStatusCode() > 29))) {
        ds.prior = ds.bPtr->renderSite();
    }

    auto current = breakLines(ds.prior, std::min(COLS, maxWidth), COLS);
    sanitizeCharactersToDraw(current);
    if(ds.toLowest) {
        ds.y = lowestPos(current);
        ds.toLowest = false;
    }

    ds.y = std::max(0,std::min(ds.y,lowestPos(current)));

    erase();
    move(0,(COLS / 2) - ((int)ds.header.size() / 2) );

    attron(A_BOLD);

    if((int)ds.header.size() < COLS) {
        addstr(ds.header.c_str());
    }  else {
        addstr((ds.header.substr(0,COLS-3) + "...").c_str());
    }

    attroff(A_BOLD);

    addstr(std::string(COLS, ' ').c_str());

    for(int i = ds.y; i - ds.y < LINES - 2 && i < (int)current.size(); ++i) {
        move(i - ds.y + 2, 0);

        attron(COLOR_PAIR(current[i].second.color + 1));
        if(current[i].second.isBold) {
            attron(A_BOLD);
            addstr(current[i].first.c_str());
            attroff(A_BOLD);
        }
        else {
            addstr(current[i].first.c_str());
        }
        attroff(COLOR_PAIR(current[i].second.color + 1));
    }

    if (ds.handleRedirect) {
        std::string toShow = "Redirect to \"" + ds.metaLine + "\" (y/n): ";
        drawInputBox(toShow, ds.redirInput);
    }

    if(ds.handleInput) {
        if(ds.metaLine != "") {
            drawInputBox(ds.metaLine + ": ", ds.userInput);
        } else {
            drawInputBox("Input: ", ds.userInput);

        }
    }

    if (ds.handleOpenOther) {
        drawInputBox("Destination / Link Number: ", ds.openOtherInput);
    }

    if(ds.issueText != "") {
        uint64_t now = getCurrentTime();
        if(ds.timeToClearIssueText <= now) {
            ds.issueText = "";
        } else {
            move(0,0);
            attron(A_BOLD);
            attron(COLOR_PAIR(COLOR_RED+1));
            addstr(ds.issueText.c_str());
            attroff(COLOR_PAIR(COLOR_RED+1));
            attroff(A_BOLD);
        }

    }

    refresh();

}

void openPageHandler(DrawState& ds, int sel) {

    if(sel == KEY_BACKSPACE) {
        if(ds.openOtherInput.size() > 0) {
            ds.openOtherInput = ds.openOtherInput.substr(0,ds.openOtherInput.size() - 1);
        }
        draw(ds);
    }

    if(sel ==  27) {
        ds.openOtherInput = "";
        ds.handleOpenOther = false;
        return;
    }

    if(sel == '\n' || sel == KEY_ENTER) {
        ds.handleOpenOther = false;
        return;
    }

    if(isValidUserInput(sel)) {
        ds.openOtherInput += std::string {(char)sel};
    }

    draw(ds);
}

enum Direction {
    FORWARD,
    BACKWARD
};

Direction handleRedir(DrawState ds) {

    ds.handleRedirect = true;
    ds.redirInput = "";
    draw(ds);

    while(true) {
        // TODO: Remove this getch invocation and make this entire file non-blocking
        int input = getch();
        if(input == 'y') {
            ds.redirInput = "y";
            draw(ds);
            ds.handleRedirect = false;
            return FORWARD;
        }
        if(input == 'n') {
            ds.redirInput = "n";
            draw(ds);
            ds.handleRedirect = false;
            return BACKWARD;
        }
        draw(ds);
    }
}

std::string handleUserInput(DrawState ds) {

    ds.handleInput = true;
    draw(ds);
    std::string acc = "";

    while(true) {

        // TODO: Remove this getch invocation and make this entire file non-blocking
        int sel = getch();
        if(sel == '\n' || sel == KEY_ENTER) {
            break;
        }
        if(sel ==  27) {
            acc = "";
            ds.userInput = acc;
            break;
        }
        if(sel == KEY_BACKSPACE) {
            if(acc.size() > 0) {
                acc = acc.substr(0,acc.size() - 1);
            }
            ds.userInput = acc;
            draw(ds);
            continue;
        }

        if(isValidUserInput(sel)) {
            acc += std::string {(char)sel};
        }
        ds.userInput = acc;
        draw(ds);
    }

    ds.handleInput = false;
    draw(ds);
    return "?" + acc;
}

void tryVisitSite(DrawState& ds , Browser& b, std::string site) {
    bool visitSuccess = b.goToSite(site);
    if(!visitSuccess) {
        ds.issueText = "Unable to access the requested site.";
        ds.timeToClearIssueText = getCurrentTime() + 1000;
    } else {
        ds.y = 0; 
    }
}

bool mainLoop(DrawState& ds, Browser& b, int input)  {

    if(ds.handleOpenOther) {
        openPageHandler(ds,input);
        if(!ds.handleOpenOther) {
            draw(ds);
            if(ds.openOtherInput != "") {
                Destination destination = handleDestinationResolution(ds.openOtherInput , false); 
                bool res;
                switch(destination.t) {
                    case NUMBER_DESTINATION:
                        res = b.followLinkNumber(destination.linkNumber);
                        if(res) {
                            ds.y = 0;
                        }
                        break;
                    case STRING_DESTINATION:
                        tryVisitSite(ds, b, destination.destination);
                        break;
                    case NO_DESTINATION:
                        break;
                }
                ds.openOtherInput = "";
            }
        } else {
            auto* clk = b.getCurrentLink();
            ds.header = clk->getLinkDestination().to_string();
            draw(ds);
            return true;
        }
        auto* clk = b.getCurrentLink();
        ds.header = clk->getLinkDestination().to_string();
        draw(ds);
        return true;
    }


    if(input == 'q') {
        return false;
    }

    if(input == KEY_DOWN) {
        ds.y += 1;
    } else if (input == KEY_UP){
        ds.y -= 1;
    } else if (input == 'g'){
        ds.y = 0;
    } else if (input == 'G'){
        ds.toLowest = true;
    } else if (input == CTRL('d')){
        ds.y += LINES / 2;
    } else if (input == CTRL('u')) {
        ds.y -= LINES / 2;
    } else if (input == 'r' || input == CTRL('r')){
        b.refresh();
    } else if(input == 'f') {
        b.goForward();
        ds.y = 0; // todo: make this part of state somewhere.
    } else if(input == 'd') {
        // TODO: Handle outLocation == "" meaning failed
        std::string outLocation = b.tryDownloadPage();
    } else if(input == 'b') {
        b.goBack();
        ds.y = 0; // todo: make this part of state somewhere.

    } else if(input == 'o') {
        ds.handleOpenOther = true;
        mainLoop(ds, b, KEY_RESIZE);
    }

    // this is the loop where we deal with redirects and stuff like that. 
    while(b.getCurrentSite()->getStatusCode() < 20 || b.getCurrentSite()->getStatusCode() > 29) {
        if(b.getCurrentSite()->getStatusCode() >= 10 && b.getCurrentSite()->getStatusCode() <= 19) {
            auto* st = b.getCurrentSite();
            if(st != nullptr) {
                ds.metaLine = st->getMeta();
            }

            std::string inputQuery = handleUserInput(ds);
            if(inputQuery != "?") { // TODO: Better handling
                tryVisitSite(ds, b,inputQuery);
            } else {
                b.goBack();
            }

        } else if (b.getCurrentSite()->getStatusCode() >= 30 && b.getCurrentSite()->getStatusCode() <= 39){
            
            auto* st = b.getCurrentSite();
            if(st != nullptr) {
                ds.metaLine = st->getMeta();
            }

            Direction dir = handleRedir(ds);
            if(dir == BACKWARD) {
                b.goBack();
            } else {
                tryVisitSite(ds, b, b.getCurrentSite()->getMeta());
            }
        } else { // this should handle invalid status codes
            b.goBack();
        }
    }

    auto* clk = b.getCurrentLink();
    if(clk != nullptr) {
        ds.header = clk->getLinkDestination().to_string();
    } else {
        ds.header = "foreseen";
    }

    auto* st = b.getCurrentSite();
    if(st != nullptr) {
        ds.metaLine = st->getMeta();
    }

    draw(ds);
    refresh();
    return true;
}
