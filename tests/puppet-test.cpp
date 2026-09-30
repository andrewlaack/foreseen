#include <catch2/catch_test_macros.hpp>
#include <rapidcheck.h>
#include "../include/render.hpp"
#include <ncurses.h>
#include <rapidcheck/Check.h>
#include <rapidcheck/Log.h>
#include <string>

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
    Browser b {};
    DrawState ds {};
    REQUIRE_FALSE(mainLoop(ds, b, 'q'));
}

TEST_CASE("RESIZE falls through and returns") {
    Browser b {};
    DrawState ds {};
    REQUIRE(mainLoop(ds, b, KEY_RESIZE));
}

TEST_CASE("Open page handler returns") {
    Browser b {};
    DrawState ds {};
    REQUIRE(mainLoop(ds, b, 'o'));
}

TEST_CASE("Open page and navigate to site") {
    Browser b {};
    DrawState ds {};
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 'l'));
    REQUIRE(mainLoop(ds, b, 'a'));
    REQUIRE(mainLoop(ds, b, 'a'));
    REQUIRE(mainLoop(ds, b, 'c'));
    REQUIRE(mainLoop(ds, b, 'k'));
    REQUIRE(mainLoop(ds, b, '.'));
    REQUIRE(mainLoop(ds, b, 'c'));
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, '\n'));
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
}

TEST_CASE("Open page and search the web") {
    Browser b {};
    DrawState ds {};
    REQUIRE(mainLoop(ds, b, 'o'));

    std::string rnd = genRandom(10);
    INFO(rnd);
    for(auto& ch : rnd) {
        REQUIRE(mainLoop(ds, b, ch));
    }
    REQUIRE(mainLoop(ds, b, '\n'));
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search?" + urlEncode(rnd));
}

TEST_CASE("Open page after backspacing character") {
    Browser b {};
    DrawState ds {};
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 'l'));
    REQUIRE(mainLoop(ds, b, 'a'));
    REQUIRE(mainLoop(ds, b, 'a'));
    REQUIRE(mainLoop(ds, b, 'a'));
    REQUIRE(mainLoop(ds, b, KEY_BACKSPACE));
    REQUIRE(mainLoop(ds, b, 'c'));
    REQUIRE(mainLoop(ds, b, 'k'));
    REQUIRE(mainLoop(ds, b, '.'));
    REQUIRE(mainLoop(ds, b, 'c'));
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, '\n'));
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
}

TEST_CASE("Cancel 'o' menu by pressing escape") {
    Browser b {};
    DrawState ds {};
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 'l'));
    REQUIRE(mainLoop(ds, b, 'a'));
    REQUIRE(mainLoop(ds, b, 'a'));
    REQUIRE(mainLoop(ds, b, 'a'));
    REQUIRE(mainLoop(ds, b, KEY_BACKSPACE));
    REQUIRE(mainLoop(ds, b, 'c'));
    REQUIRE(mainLoop(ds, b, 'k'));
    REQUIRE(mainLoop(ds, b, '.'));
    REQUIRE(mainLoop(ds, b, 'c'));
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 27)); // escape
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "about://newtab");
}

TEST_CASE("Handle redirects") {
    Browser b {};
    DrawState ds {};
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 'c'));
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 'n'));
    REQUIRE(mainLoop(ds, b, 'm'));
    REQUIRE(mainLoop(ds, b, 'a'));
    REQUIRE(mainLoop(ds, b, 'n'));
    REQUIRE(mainLoop(ds, b, '.'));
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 'r'));
    REQUIRE(mainLoop(ds, b, 'g'));
    REQUIRE(mainLoop(ds, b, '\n'));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://conman.org");

    REQUIRE(mainLoop(ds, b, 'y')); // follow redirect
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://gemini.conman.org/");

}
TEST_CASE("Handle redirect rejection") {
    Browser b {};
    DrawState ds {};
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 'c'));
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 'n'));
    REQUIRE(mainLoop(ds, b, 'm'));
    REQUIRE(mainLoop(ds, b, 'a'));
    REQUIRE(mainLoop(ds, b, 'n'));
    REQUIRE(mainLoop(ds, b, '.'));
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 'r'));
    REQUIRE(mainLoop(ds, b, 'g'));
    REQUIRE(mainLoop(ds, b, '\n'));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://conman.org");

    REQUIRE(mainLoop(ds, b, 'n')); // follow redirect
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "about://newtab");
}

TEST_CASE("Enter an input") {
    Browser b {};
    DrawState ds {};

    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 't'));
    REQUIRE(mainLoop(ds, b, 'l'));
    REQUIRE(mainLoop(ds, b, 'g'));
    REQUIRE(mainLoop(ds, b, 's'));
    REQUIRE(mainLoop(ds, b, '.'));
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 'n'));
    REQUIRE(mainLoop(ds, b, 'e'));
    REQUIRE(mainLoop(ds, b, '\n'));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one");

    REQUIRE(mainLoop(ds,b,'o'));
    REQUIRE(mainLoop(ds,b,'2'));
    REQUIRE(mainLoop(ds, b, '\n'));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search");

    std::string rnd = genRandom(10);

    INFO(rnd);

    for(auto& ch : rnd) {
        REQUIRE(mainLoop(ds, b, ch));
    }

    REQUIRE(mainLoop(ds, b, '\n'));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search?" + urlEncode(rnd));
}

TEST_CASE("Enter an input and then cancel") {
    Browser b {};
    DrawState ds {};

    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 't'));
    REQUIRE(mainLoop(ds, b, 'l'));
    REQUIRE(mainLoop(ds, b, 'g'));
    REQUIRE(mainLoop(ds, b, 's'));
    REQUIRE(mainLoop(ds, b, '.'));
    REQUIRE(mainLoop(ds, b, 'o'));
    REQUIRE(mainLoop(ds, b, 'n'));
    REQUIRE(mainLoop(ds, b, 'e'));
    REQUIRE(mainLoop(ds, b, '\n'));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one");

    REQUIRE(mainLoop(ds,b,'o'));
    REQUIRE(mainLoop(ds,b,'2'));
    REQUIRE(mainLoop(ds, b, '\n'));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search");

    std::string rnd = genRandom(10);

    INFO(rnd);

    for(auto& ch : rnd) {
        REQUIRE(mainLoop(ds, b, ch));
    }

    REQUIRE(mainLoop(ds, b, 27)); // escape

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one");
}


TEST_CASE("Only return false on 'q' entry when not using input boxes, never crash, junk inputs") {
    rc::check("Never return false / fail except with input 'q'", [] (std::string st) {
        Browser b {};
        DrawState ds {};
        RC_LOG(st);
        for(auto& cur : st) {
            if(cur == 'q') {
                if(!ds.handleInput && !ds.handleOpenOther && !ds.handleRedirect) {
                    REQUIRE_FALSE(mainLoop(ds,b,cur));
                } else {
                    REQUIRE(mainLoop(ds,b,cur));
                }
            } else {
                REQUIRE(mainLoop(ds,b,cur));
            }
        }
    });
}

TEST_CASE("Only return false on 'q' entry when not using input boxes, alphanumeric inputs") {
    for(int i = 0; i < 100; ++i) {
        Browser b {};
        DrawState ds {};
        std::string st = genAlNumNlSp(rand()%1000);
        INFO(st);
        for(auto& cur : st) {
            if(cur == 'q') {
                if(!ds.handleInput && !ds.handleOpenOther && !ds.handleRedirect) {
                    REQUIRE_FALSE(mainLoop(ds,b,cur));
                } else {
                    REQUIRE(mainLoop(ds,b,cur));
                }
            } else {
                REQUIRE(mainLoop(ds,b,cur));
            }
        }
    }
}

