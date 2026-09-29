#include <catch2/catch_test_macros.hpp>
#include "../include/render.hpp"
#include <ncurses.h>

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
