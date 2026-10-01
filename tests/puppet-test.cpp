#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <filesystem>
#include <rapidcheck.h>
#include "../include/render.hpp"
#include <ncurses.h>
#include <rapidcheck/Check.h>
#include <rapidcheck/Log.h>
#include <string>
#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif


std::string genRandom(const int len) {
    static const char alpha[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alpha[rand() % (sizeof(alpha) - 1)];
    }
    
    return tmp_s;
}

std::string genAlNumNlSp(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz\n ";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}


// Puppet entire browser

TEST_CASE("q quits session immediately") {
    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;
    DrawState ds {};
    ds.bPtr = bPtr;
    REQUIRE_FALSE(mainLoop(ds, b, 'q',0,0));
    delete bPtr;
}

TEST_CASE("RESIZE falls through and returns") {
    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;
    DrawState ds {};
    ds.bPtr = bPtr;
    REQUIRE(mainLoop(ds, b, KEY_RESIZE,0,0));
    delete bPtr;
}

TEST_CASE("Open page handler returns") {
    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;
    DrawState ds {};
    ds.bPtr = bPtr;
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    delete bPtr;
}

TEST_CASE("Open page and navigate to site") {

    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;
    DrawState ds {};
    ds.bPtr = bPtr;
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'l',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, 'c',0,0));
    REQUIRE(mainLoop(ds, b, 'k',0,0));
    REQUIRE(mainLoop(ds, b, '.',0,0));
    REQUIRE(mainLoop(ds, b, 'c',0,0));
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, '\n',0,0));
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
    delete bPtr;
}

TEST_CASE("Open page and search the web") {
    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;
    DrawState ds {};
    ds.bPtr = bPtr;
    REQUIRE(mainLoop(ds, b, 'o',0,0));

    std::string rnd = genRandom(10);
    INFO(rnd);
    for(auto& ch : rnd) {
        REQUIRE(mainLoop(ds, b, ch,0,0));
    }
    REQUIRE(mainLoop(ds, b, '\n',0,0));
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search?" + urlEncode(rnd));
    delete bPtr;
}

TEST_CASE("Open page after backspacing character") {
    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;
    DrawState ds {};
    ds.bPtr = bPtr;
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'l',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, KEY_BACKSPACE,0,0));
    REQUIRE(mainLoop(ds, b, 'c',0,0));
    REQUIRE(mainLoop(ds, b, 'k',0,0));
    REQUIRE(mainLoop(ds, b, '.',0,0));
    REQUIRE(mainLoop(ds, b, 'c',0,0));
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, '\n',0,0));
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
    delete bPtr;
}

TEST_CASE("Cancel 'o' menu by pressing escape") {
    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;
    DrawState ds {};
    ds.bPtr = bPtr;
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'l',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, KEY_BACKSPACE,0,0));
    REQUIRE(mainLoop(ds, b, 'c',0,0));
    REQUIRE(mainLoop(ds, b, 'k',0,0));
    REQUIRE(mainLoop(ds, b, '.',0,0));
    REQUIRE(mainLoop(ds, b, 'c',0,0));
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 27,0,0)); // escape
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "about://newtab");
    delete bPtr;
}

TEST_CASE("Handle redirects") {
    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;
    DrawState ds {};
    ds.bPtr = bPtr;
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'c',0,0));
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'n',0,0));
    REQUIRE(mainLoop(ds, b, 'm',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, 'n',0,0));
    REQUIRE(mainLoop(ds, b, '.',0,0));
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'r',0,0));
    REQUIRE(mainLoop(ds, b, 'g',0,0));
    REQUIRE(mainLoop(ds, b, '\n',0,0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://conman.org");

    REQUIRE(mainLoop(ds, b, 'y',0,0)); // follow redirect
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://gemini.conman.org/");
    delete bPtr;

}
TEST_CASE("Handle redirect rejection") {
    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;
    DrawState ds {};
    ds.bPtr = bPtr;
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'c',0,0));
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'n',0,0));
    REQUIRE(mainLoop(ds, b, 'm',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, 'n',0,0));
    REQUIRE(mainLoop(ds, b, '.',0,0));
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'r',0,0));
    REQUIRE(mainLoop(ds, b, 'g',0,0));
    REQUIRE(mainLoop(ds, b, '\n',0,0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://conman.org");

    REQUIRE(mainLoop(ds, b, 'n',0,0)); // follow redirect
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "about://newtab");
    delete bPtr;
}

TEST_CASE("Enter an input") {
    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;
    DrawState ds {};
    ds.bPtr = bPtr;

    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 't',0,0));
    REQUIRE(mainLoop(ds, b, 'l',0,0));
    REQUIRE(mainLoop(ds, b, 'g',0,0));
    REQUIRE(mainLoop(ds, b, 's',0,0));
    REQUIRE(mainLoop(ds, b, '.',0,0));
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'n',0,0));
    REQUIRE(mainLoop(ds, b, 'e',0,0));
    REQUIRE(mainLoop(ds, b, '\n',0,0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one");

    REQUIRE(mainLoop(ds,b,'o',0,0));
    REQUIRE(mainLoop(ds,b,'2',0,0));
    REQUIRE(mainLoop(ds, b, '\n',0,0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search");

    std::string rnd = genRandom(10);

    INFO(rnd);

    for(auto& ch : rnd) {
        REQUIRE(mainLoop(ds, b, ch,0,0));
    }

    REQUIRE(mainLoop(ds, b, '\n',0,0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search?" + urlEncode(rnd));
    delete bPtr;
}

TEST_CASE("Enter an input and then cancel") {
    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;
    DrawState ds {};
    ds.bPtr = bPtr;

    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 't',0,0));
    REQUIRE(mainLoop(ds, b, 'l',0,0));
    REQUIRE(mainLoop(ds, b, 'g',0,0));
    REQUIRE(mainLoop(ds, b, 's',0,0));
    REQUIRE(mainLoop(ds, b, '.',0,0));
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'n',0,0));
    REQUIRE(mainLoop(ds, b, 'e',0,0));
    REQUIRE(mainLoop(ds, b, '\n',0,0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one");

    REQUIRE(mainLoop(ds,b,'o',0,0));
    REQUIRE(mainLoop(ds,b,'2',0,0));
    REQUIRE(mainLoop(ds, b, '\n',0,0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search");

    std::string rnd = genRandom(10);

    INFO(rnd);

    for(auto& ch : rnd) {
        REQUIRE(mainLoop(ds, b, ch,0,0));
    }

    REQUIRE(mainLoop(ds, b, 27,0,0)); // escape

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one");
    delete bPtr;
}


TEST_CASE("Only return false on 'q' entry when not using input boxes, never crash, junk inputs") {
    setenv("EDITOR", "test" , 1);

    rc::check("Never return false / fail except with input 'q'", [] (std::string st) {
        Browser* bPtr = new Browser {};
        Browser& b = *bPtr;
        DrawState ds {};
        ds.bPtr = bPtr;
        RC_LOG(st);
        for(auto& cur : st) {
            if(cur == 'q') {
                if(!ds.handleInput && !ds.handleOpenOther && !ds.handleRedirect) {
                    REQUIRE_FALSE(mainLoop(ds,b,cur,0,0));
                } else {
                    REQUIRE(mainLoop(ds,b,cur,0,0));
                }
            } else {
                REQUIRE(mainLoop(ds,b,cur,0,0));
            }
        }
        delete bPtr;
    });
}

TEST_CASE("Only return false on 'q' entry when not using input boxes, alphanumeric inputs") {
    setenv("EDITOR", "test" , 1);

    for(int i = 0; i < 10; ++i) {
        Browser* bPtr = new Browser {};
        Browser& b = *bPtr;

        DrawState ds {};
        ds.bPtr = bPtr;
        std::string st = genAlNumNlSp(rand()%100);
        INFO(st);
        for(auto& cur : st) {
            if(cur == 'q') {
                if(!ds.handleInput && !ds.handleOpenOther && !ds.handleRedirect) {
                    REQUIRE_FALSE(mainLoop(ds,b,cur,0,0));
                } else {
                    REQUIRE(mainLoop(ds,b,cur,0,0));
                }
            } else {
                REQUIRE(mainLoop(ds,b,cur,0,0));
            }
        }
        delete bPtr;

    }
}


TEST_CASE("Open file in editor creates file") {

    setenv("EDITOR", "test" , 1);

    std::filesystem::remove("/tmp/foreseen/laack.co");

    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;

    DrawState ds {};
    ds.bPtr = bPtr;

    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, 'l',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, 'a',0,0));
    REQUIRE(mainLoop(ds, b, 'c',0,0));
    REQUIRE(mainLoop(ds, b, 'k',0,0));
    REQUIRE(mainLoop(ds, b, '.',0,0));
    REQUIRE(mainLoop(ds, b, 'c',0,0));
    REQUIRE(mainLoop(ds, b, 'o',0,0));
    REQUIRE(mainLoop(ds, b, '\n',0,0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");

    REQUIRE(mainLoop(ds,b,'e',0,0));
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");

    REQUIRE(std::filesystem::exists("/tmp/foreseen/laack.co"));
    REQUIRE(std::filesystem::remove("/tmp/foreseen/laack.co"));
    delete bPtr;

}

// If re-render was true this would mean we have to refresh on the next iteration, but we'd have to send a key to do that
// which would be weird ux

TEST_CASE("Re-render and re-break are never true outside of the main loop.") {

    setenv("EDITOR", "test" , 1);

    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;

    DrawState ds {};
    ds.bPtr = bPtr;

    REQUIRE(mainLoop(ds, b, 'o',100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, 'l',100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, 'a',100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, 'a',100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, 'c',100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, 'k',100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, '.',100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, 'c',100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, 'o',100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, '\n',100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);
    
    REQUIRE(mainLoop(ds, b, KEY_UP,100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, KEY_DOWN,100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, CTRL('d'),100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, CTRL('u'),100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, KEY_RESIZE, 120,110));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);

    REQUIRE(mainLoop(ds, b, 'o',100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);
    REQUIRE(ds.handleOpenOther);

    REQUIRE(mainLoop(ds, b, 27,100,100));
    REQUIRE_FALSE(ds.mustReRender);
    REQUIRE_FALSE(ds.reBreak);
    REQUIRE_FALSE(ds.handleOpenOther);

    delete bPtr;
}




TEST_CASE("Basic usage with specified line and col count") {

    setenv("EDITOR", "test" , 1);

    Browser* bPtr = new Browser {};
    Browser& b = *bPtr;

    DrawState ds {};
    ds.bPtr = bPtr;

    REQUIRE(mainLoop(ds, b, 'o',100,100));
    REQUIRE(mainLoop(ds, b, 'l',100,100));
    REQUIRE(mainLoop(ds, b, 'a',100,100));
    REQUIRE(mainLoop(ds, b, 'a',100,100));
    REQUIRE(mainLoop(ds, b, 'c',100,100));
    REQUIRE(mainLoop(ds, b, 'k',100,100));
    REQUIRE(mainLoop(ds, b, '.',100,100));
    REQUIRE(mainLoop(ds, b, 'c',100,100));
    REQUIRE(mainLoop(ds, b, 'o',100,100));
    REQUIRE(mainLoop(ds, b, '\n',100,100));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
    delete bPtr;
}

TEST_CASE("Basic usage with small screen sizes") {

    setenv("EDITOR", "test" , 1);

    for(int i = 0; i < 10; ++i) {
        int x = rand() % 25;
        int y = rand() % 10;
        Browser* bPtr = new Browser {};
        Browser& b = *bPtr;
        DrawState ds {};
        ds.bPtr = bPtr;

        std::string cwd = std::filesystem::current_path();
        std::string st = "file://" + cwd + "/tests/sites/all_line_types.gmi";

        REQUIRE(mainLoop(ds, b, 'o',x,y));

        for(auto& ch: st) {
            REQUIRE(mainLoop(ds, b, ch,x,y));
        }

        REQUIRE(mainLoop(ds, b, '\n',x,y));

        REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == st);
        delete bPtr;
    }
}

