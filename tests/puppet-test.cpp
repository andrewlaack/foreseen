#include <catch2/catch_test_macros.hpp>
#include "../include/utils.hpp"
#include "../include/render.hpp"
#include <filesystem>
#include <ncurses.h>

#include "../include/gemini-client.hpp"
#include "../include/link.hpp"

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



//TEST_CASE("Puppet browser can traverse to a public site") {
//    Browser b {};
//    DrawState ds {};
//    std::string st = "olaack.co\n";
//
//    for(auto& ch : st) {
//        mainLoop(ds, b, ch);
//    }
//    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://laack.co");
//}
