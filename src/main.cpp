#include "../include/browser.hpp"
#include "../include/utils.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <ncurses.h>
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

const int maxWidth = 80;

const std::string DEFAULT_SEARCH_ENGINE="gemini://tlgs.one/search?";


bool isValidUserInput(int uinput) {
    if ((uinput >= 0x20 && uinput <= 0x7E)) {
        return true;
    }
    return false;
}

// we do this because this is built against ncurses, would be nice to do away w/ this
// bc ppl use lots of emojis on gemini sites.

void removeNonAscii(std::vector<std::pair<std::string, TextRender>>& strLs) {
    for(std::size_t i = 0; i < strLs.size(); ++i) {

        std::string& s = strLs[i].first;

        std::string out;
        for (int c: s)
            if ((c >= 0x20 && c <= 0x7E) || (c == '\n')) {
                out += c;
            } else if (c == '\t') {
                out += "    "; // \t is a larger character and fucks with breaklines.
            }

        strLs[i].first = out;
    }
    return;
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
struct DrawState {
    std::vector<std::pair<std::string, TextRender>> prior;
    Browser* bPtr;
    std::string header;
    int y;
    bool handleInput;
    bool handleOpenOther;
    bool handleRedirect;
    std::string redirInput;
    std::string openOtherInput;
    std::string userInput;
    std::string metaLine;
    bool toLowest = false;
};

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

    auto& current = ds.prior;
    auto* cs = ds.bPtr->getCurrentSite();

    if(cs != nullptr && (cs->getStatusCode() < 20 || cs->getStatusCode() > 29)) {
        current = ds.prior;

    } else {
        current = ds.bPtr->renderSite();
        removeNonAscii(current);
        current = breakLines(current,std::min(COLS, maxWidth), COLS);
        ds.prior = current;
    }

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

    refresh();

}

std::string openPageHandler(DrawState ds) {

    ds.handleOpenOther = true;
    draw(ds);
    std::string acc = "";

    while(true) {
        int sel = getch();

        if(sel == KEY_BACKSPACE) {
            if(acc.size() > 0) {
                acc = acc.substr(0,acc.size() - 1);
            }
            ds.openOtherInput = acc;
            draw(ds);
            continue;
        }

        if(sel ==  27) {
            acc = "";
            break;
        }

        if(sel == '\n' || sel == KEY_ENTER) {
            break;
        }

        if(isValidUserInput(sel)) {
            acc += std::string {(char)sel};
        }

        ds.openOtherInput = acc;
        draw(ds);

    }

    ds.openOtherInput = acc;
    ds.handleOpenOther = false;
    draw(ds);



    return acc;
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


int main(int argc, char** argv) {

    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;

    DrawState ds {};
    ds.bPtr = bPtr;

    initscr();
    set_escdelay(25);
    noecho(); // don't echo user inputs
    cbreak(); // make C-c and C-z work
    curs_set(0); // hide cursor
	keypad(stdscr,TRUE);

    initColors();

    b.goToSite("about:newtab",true);

    if(argc > 1) {
        // TODO: This should have tests.

        if(std::filesystem::exists(argv[1])) {
            std::string path = "file:///" + std::filesystem::current_path().string() + "/" + argv[1];
            b.goToSite(path,true);
        }  else {
            std::string inputString = argv[1];
            if(inputString.find("gemini://") == 0) {
                b.goToSite(argv[1], true);
            } else {
                b.goToSite(std::string {"gemini://"} + argv[1], true);
            }
        }

    } else {
        b.goToSite("gemini://tlgs.one",true);
    }


    int input = 0;

    // this is the main loop.
    //
    while( input != 'q') {

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
            b.downloadPage();
        } else if(input == 'b') {
            b.goBack();
            ds.y = 0; // todo: make this part of state somewhere.

        } else if(input == 'o') {
            std::string locationToGo = openPageHandler(ds);

            // TODO: Check if this is an int and if it is try to use that link.
            // if it's not, then try to go to domain (should consider having a default search engine too.)

            try {
                b.followLinkNumber(std::stoi(locationToGo));
            } catch (...) {
                if(locationToGo != "") {
                    if(locationToGo.find(":") == std::string::npos) { // TODO: Is this how we which scheme was specified?
                        if(locationToGo.find('.') != std::string::npos && urlEncode(locationToGo) == locationToGo) { // hmm, this feels weak. Like I can't search something if I add a period?
                            locationToGo = "gemini://" + locationToGo;
                        } else {
                            locationToGo = DEFAULT_SEARCH_ENGINE + urlEncode(locationToGo);
                        }
                    }
                    b.goToSite(locationToGo, true);
                    ds.y = 0; // todo: make this part of state somewhere.

                }
            }

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
                    b.goToSite(inputQuery,true);
                    ds.y = 0; // todo: make this part of state somewhere.
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
                    b.goToSite(b.getCurrentSite()->getMeta(),true);
                    ds.y = 0; // todo: make this part of state somewhere.
                }
            }
        }

        auto* clk = b.getCurrentLink();
        if(clk != nullptr) {
            ds.header = clk->getLinkDestination().to_string();
        } else {
            ds.header = "Welcome!";
        }

        auto* st = b.getCurrentSite();
        if(st != nullptr) {
            ds.metaLine = st->getMeta();
        }

        draw(ds);
        refresh();


        input = getch();
    }

    endwin();

    delete bPtr;
}
