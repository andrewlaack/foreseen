#include "../include/browser.hpp"
#include "../include/utils.hpp"
#include "../include/render.hpp"
#include <cassert>
#include <csignal>
#include <ncurses.h>
#include <locale.h>
#include <string>
#include <unctrl.h>

int main(int argc, char** argv) {

    std::signal(SIGPIPE, SIG_IGN); // need this in case of swapping network connections bc that shouldn't kill the whole process.

    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;

    DrawState ds {};
    ds.bPtr = bPtr;

    setlocale(LC_CTYPE, ""); // emojis and such

    initscr();
    set_escdelay(25);
    noecho(); // don't echo user inputs
    cbreak(); // make C-c and C-z work
    curs_set(0); // hide cursor
	keypad(stdscr,TRUE);

    initColors();

    std::string destination = "";

    if(argc > 1) {
        destination = argv[1];
    }

    Destination cliDestination = handleDestinationResolution(destination, true);

    if(cliDestination.t != NO_DESTINATION) {
        assert(cliDestination.t == STRING_DESTINATION); // we don't allow numeric link following on startup.
        tryVisitSite(ds,b, cliDestination.destination);
    }

    int input = 0;

    while(true) {
        bool continueExecution = mainLoop(ds,b,input, COLS,LINES);
        if(!continueExecution) {
            break;
        }
        input = getch();
    }

    endwin();

    delete bPtr;
}
