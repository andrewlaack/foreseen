#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <utility>
#include "../include/browser.hpp"
#include "../include/utils.hpp"


TEST_CASE( "Basic navigation" ) {
    Browser b{};
    b.goToSite("file:///home/andrew/gitRepos/gemini-browser/tests/sites/basic.gmi", true);
    REQUIRE(b.getCurrentSite()->getBody() == readFileToString("tests/sites/basic.gmi"));
    b.goToSite("file:///home/andrew/gitRepos/gemini-browser/tests/sites/basic_2.gmi", true);
    REQUIRE(b.getCurrentSite()->getBody() == readFileToString("tests/sites/basic_2.gmi"));
}


TEST_CASE( "Local filesystem relative navigation" ) {

    Browser b{};
    b.goToSite("file:///home/andrew/gitRepos/gemini-browser/tests/sites/basic.gmi", true);
    REQUIRE(b.getCurrentSite()->getBody() == readFileToString("tests/sites/basic.gmi"));
    b.goToSite("basic_2.gmi", true);
    REQUIRE(b.getCurrentSite()->getBody() == readFileToString("tests/sites/basic_2.gmi"));
}

TEST_CASE("Test browser doesn't crash on invalid sites.") {
    Browser b{};
    b.goToSite("about://newtab");
    b.goToSite("gemini://this_site_doesn_t_exsist");
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "about://newtab");
}
