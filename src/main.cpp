#include "../include/browser.hpp"
#include "../include/utils.hpp"
#include "../include/render.hpp"
#include "../include/shared.hpp"
#include <cassert>
#include <csignal>
#include <ncurses.h>
#include <locale.h>
#include <string>
#include <unctrl.h>
#include "../vendor/argparse.hpp"

int main(int argc, char** argv) {

    argparse::ArgumentParser program("foreseen", "0.0.1");

    program.add_argument("-c", "--cache-pages")
    .help("max number of pages for each cache (history, prefetch)")
    .default_value(CACHE_SIZE)
    .scan<'i', int>();

    program.add_argument("-p", "--prefetch-links")
    .help("max number of links to prefetch per page (0 disables prefetching)")
    .default_value(SITE_CACHE_LIMIT)
    .scan<'i', int>();

    program.add_argument("-e", "--search-engine")
    .help("default search engine")
    .default_value(DEFAULT_SEARCH_ENGINE);

    program.add_argument("destination")
    .help("page to open")
    .nargs(argparse::nargs_pattern::optional);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    SITE_CACHE_LIMIT = program.get<int>("--prefetch-links");
    CACHE_SIZE = program.get<int>("--cache-pages");
    DEFAULT_SEARCH_ENGINE = program.get<std::string>("--search-engine");

    if(DEFAULT_SEARCH_ENGINE.substr(DEFAULT_SEARCH_ENGINE.size()-1) != "?") {
        DEFAULT_SEARCH_ENGINE += "?";
    }

    std::string destination = program.present<std::string>("destination").value_or("");

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
