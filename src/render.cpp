#include "../include/browser.hpp"
#include "../include/utils.hpp"
#include "../include/shared.hpp"
#include "../include/render.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <ncurses.h>
#include <string>
#include <unctrl.h>
#include <utility>
#include <vector>

#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

#ifdef DEBUG_MODE
    const char* OUT_LOCATION = "/tmp/foreseen"; // TODO: This is bad. Pass these into fn
#else
    const char* OUT_LOCATION = "";
#endif



int lowestPos(std::vector<std::pair<std::string, TextRender>>& strLs, DrawState& ds) {
    return (strLs.size() - (ds.lines - 2)) + 1; // this gives us two new lines at the end because the last line should contain a \n.
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

void drawInputBox(std::string text, std::string userInput, DrawState& ds) {

    move(ds.lines/2-1, ds.columns/4);

    attron(COLOR_PAIR(COLOR_CYAN+1));
    for(int i = 0; i < ds.columns/2; ++i) {
        addstr("-");
    }

    move(ds.lines/2 + 1, ds.columns/4);
    for(int i = 0; i < ds.columns/2; ++i) {
        addstr("-");
    }
    attroff(COLOR_PAIR(COLOR_CYAN+1));

    int userInputSize = userInput.size();
    int width = ds.columns/2;

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

    move(ds.lines/2, ds.columns/4);
    addstr(text.c_str());
    addstr(userTextToRender.c_str());

}
uint64_t getCurrentTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void draw(DrawState& ds) {
    if(ds.columns < 20) {
        erase();
        addstr("Screen width too small.");
        refresh();
        return;
    }
    if(ds.lines < 3) {
        erase();
        addstr("Screen height too small.");
        refresh();
        return;
    }

    auto* cs = ds.bPtr->getCurrentSite();

    // CS is null or has status 2X
    if (!(cs != nullptr && (cs->getStatusCode() < 20 || cs->getStatusCode() > 29))) {
        if(ds.mustReRender) {
            ds.prior = ds.bPtr->renderSite();
            sanitizeCharactersToDraw(ds.prior);
            ds.mustReRender = false;
            ds.reBreak = true;
        }
    }

    if(ds.reBreak) {
        ds.broken  = breakLines(ds.prior, std::min(ds.columns, MAX_WIDTH), ds.columns);
        ds.reBreak = false;
    }
    if(ds.toLowest) {
        ds.y = lowestPos(ds.broken, ds);
        ds.toLowest = false;
    }

    ds.y = std::max(0,std::min(ds.y,lowestPos(ds.broken,ds)));

    erase();
    move(0,(ds.columns / 2) - ((int)ds.header.size() / 2) );

    attron(A_BOLD);

    if((int)ds.header.size() < ds.columns) {
        addstr(ds.header.c_str());
    }  else {
        addstr((ds.header.substr(0,ds.columns-3) + "...").c_str());
    }

    attroff(A_BOLD);

    addstr(std::string(ds.columns, ' ').c_str());

    for(int i = ds.y; i - ds.y < ds.lines - 2 && i < (int)ds.broken.size(); ++i) {
        move(i - ds.y + 2, 0);

        attron(COLOR_PAIR(ds.broken[i].second.color + 1));
        if(ds.broken[i].second.isBold) {
            attron(A_BOLD);
            addstr(ds.broken[i].first.c_str());
            attroff(A_BOLD);
        }
        else {
            addstr(ds.broken[i].first.c_str());
        }
        attroff(COLOR_PAIR(ds.broken[i].second.color + 1));
    }

    if (ds.handleRedirect) {
        std::string toShow = "Redirect to \"" + ds.metaLine + "\" (y/n): ";
        drawInputBox(toShow, ds.redirInput, ds);
    }

    if(ds.handleInput) {
        if(ds.metaLine != "") {
            drawInputBox(ds.metaLine + ": ", ds.userInput, ds);
        } else {
            drawInputBox("Input: ", ds.userInput, ds);

        }
    }

    if (ds.handleOpenOther) {
        drawInputBox("Destination / Link Number: ", ds.openOtherInput, ds);
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

    if(sel == KEY_BACKSPACE || sel == 127 || sel == 8) {
        if(ds.openOtherInput.size() > 0) {
            ds.openOtherInput = ds.openOtherInput.substr(0,ds.openOtherInput.size() - 1);
        }
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

}

void handleRedir(DrawState& ds, int input, Browser& b) {
    if(input == 'y') {
        ds.redirInput = "y";
        draw(ds);
        ds.handleRedirect = false;
        tryVisitSite(ds, b, b.getCurrentSite()->getMeta());
        ds.mustReRender = true;
    }
    if(input == 'n') {
        ds.redirInput = "n";
        draw(ds);
        ds.handleRedirect = false;
        b.goBack();
        ds.mustReRender = true;
    }
}

void handleUserInput(DrawState& ds, Browser& b, int sel) {
    if(sel == '\n' || sel == KEY_ENTER) {

        ds.handleInput = false;

        if(ds.userInput != "") {
            tryVisitSite(ds, b, "?"+ds.userInput);
        } else {
            b.goBack();
            ds.mustReRender = true;
        }
        return;
    }
    if(sel == 27) {
        ds.handleInput = false;
        draw(ds); // this might not be totally necessary because back is generally fast, but it's not strictly
                  // guaranteed.
        b.goBack();
        ds.mustReRender = true;
        return;
    }

    if(sel == KEY_BACKSPACE || sel == 127 || sel == 8) {
        if(ds.userInput.size() > 0) {
            ds.userInput = ds.userInput.substr(0,ds.userInput.size() - 1);
        }
        return;
    }

    if(isValidUserInput(sel)) {
        ds.userInput += std::string {(char)sel};
    }
}

void tryVisitSite(DrawState& ds , Browser& b, std::string site) {
    GoToSiteResult visitSuccess = b.goToSite(site);

    if(visitSuccess == SITE_LOADED) {
        ds.mustReRender = true;
        ds.y = 0; 
    }
    else {
        ds.issueText = "Unable to access the requested site.";
        ds.timeToClearIssueText = getCurrentTime() + 1000;
    }
}

bool mainLoop(DrawState& ds, Browser& b, int input, int cols, int lines)  {

    if(lines != ds.lines || cols != ds.columns) {
        ds.reBreak = true;
    }

    ds.lines = lines;
    ds.columns = cols;

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
                        ds.mustReRender = true;
                        break;
                    case STRING_DESTINATION:
                        tryVisitSite(ds, b, destination.destination);
                        break;
                    case NO_DESTINATION:
                        break;
                }
                ds.openOtherInput = "";
            }
            mainLoop(ds, b, KEY_RESIZE, ds.columns, ds.lines);
        } else {
            draw(ds);
        }
        return true;
    } else if (ds.handleRedirect) {

        handleRedir(ds, input, b);

        if(!ds.handleRedirect) {
            mainLoop(ds, b, KEY_RESIZE, ds.columns, ds.lines);
        }  else {
            draw(ds);
        }

        return true;
    } else if (ds.handleInput) {
        handleUserInput(ds, b, input);
        if(!ds.handleInput) {
            mainLoop(ds, b, KEY_RESIZE, ds.columns, ds.lines);
        } else {
            draw(ds);
        }
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
        ds.y += ds.lines / 2;
    } else if (input == CTRL('u')) {
        ds.y -= ds.lines / 2;
    } else if (input == 'r' || input == CTRL('r')){
        b.refresh();
        ds.mustReRender = true;
    } else if(input == 'f') {
        b.goForward();
        ds.mustReRender = true;
        ds.y = 0; // todo: make this part of state somewhere.
    } else if(input == 'd') {
        // TODO: Handle outLocation == "" meaning failed
        std::string outLocation = b.tryDownloadPage(OUT_LOCATION);
    } else if(input == 'b') {
        b.goBack();
        ds.mustReRender = true;
        ds.y = 0; // todo: make this part of state somewhere.
    } else if(input == 'o') {
        ds.handleOpenOther = true;
    } else if (input == 'e'){
        std::string editor = getEditor();
        std::string dl = b.tryDownloadPage(OUT_LOCATION);
        if(dl != "") {
            def_prog_mode();
            endwin();
            std::string quoted = "'";
            for (char c : dl) {
                if(c == '\'') {
                    quoted += std::string("'\\''");
                } else {
                    quoted +=  std::string(1, c);
                }
            }
            quoted += "'";
            system((editor + " " + quoted).c_str());
            refresh();
        }
    }

    // this is the loop where we deal with redirects and stuff like that. 
    // TODO: I don't think this has to still be a loop?
    // Broadly, we are moving away from a loop based approach, sequestering them to either browser with forward / backward
    // or main.cpp / tests.

    if( (b.getCurrentSite()->getStatusCode() < 20 || b.getCurrentSite()->getStatusCode() > 29) && !ds.handleRedirect && !ds.handleInput) {
        if(b.getCurrentSite()->getStatusCode() >= 10 && b.getCurrentSite()->getStatusCode() <= 19) {

            auto* st = b.getCurrentSite();
            assert(st != nullptr);
            ds.metaLine = st->getMeta();

            ds.handleInput = true;
            ds.userInput = "";

        } else if (b.getCurrentSite()->getStatusCode() >= 30 && b.getCurrentSite()->getStatusCode() <= 39 && !ds.handleRedirect){
            auto* st = b.getCurrentSite();
            if(st != nullptr) {
                ds.metaLine = st->getMeta();
            }

            ds.redirInput = "";
            ds.handleRedirect = true;
        }
    }

    auto* clk = b.getCurrentLink();
    assert(clk != nullptr);
    ds.header = clk->getLinkDestination().to_string();

    auto* st = b.getCurrentSite();
    assert(st != nullptr);
    ds.metaLine = st->getMeta();
    

    draw(ds);
    return true;
}
