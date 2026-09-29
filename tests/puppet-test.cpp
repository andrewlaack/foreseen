#include <catch2/catch_test_macros.hpp>
#include "../include/render.hpp"
#include <ncurses.h>

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

