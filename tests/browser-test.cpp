#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <filesystem>
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

TEST_CASE ("Local filesystems navigation via links") {
    Browser b{};
    b.goToSite("file:///home/andrew/gitRepos/gemini-browser/tests/sites/basic.gmi", true);
    REQUIRE(b.getCurrentSite()->getBody() == readFileToString("tests/sites/basic.gmi"));
    b.followLinkNumber(1);
    REQUIRE(b.getCurrentSite()->getBody() == readFileToString("tests/sites/basic_2.gmi"));
}

TEST_CASE("Test browser doesn't crash on invalid sites.") {
    Browser b{};
    b.goToSite("gemini://this_site_doesn_t_exsist");
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "about://newtab");
    b.goBack();
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "about://newtab");
}

TEST_CASE("Test browser doesn't crash on invalid link number following, excessive back, and excessive forwards.") {

    Browser b{};
    b.goToSite("gemini://blog.laack.co/feed.xml");
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://blog.laack.co/feed.xml");
    for(int i = 0; i < 1000; ++i) {
        b.followLinkNumber(i);
    }
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://blog.laack.co/feed.xml");
    b.goBack();
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "about://newtab");
    b.goForward();
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://blog.laack.co/feed.xml");
    b.goBack();
    b.goBack();
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "about://newtab");
    b.goForward();
    b.goForward();
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://blog.laack.co/feed.xml");
}

TEST_CASE("Test page downloading.") {
    Browser b{};
    b.goToSite("gemini://blog.laack.co");
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://blog.laack.co");
    std::string destination = b.downloadPage();
    std::string out = readFileToString(destination);
    REQUIRE(out == b.getCurrentSite()->getBody());
    REQUIRE(std::filesystem::remove(destination));
}

TEST_CASE("Test encoding allows relative linking with : in parameter") {
    Browser b{};
    b.goToSite("gemini://tlgs.one/search");
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search");
    b.goToSite("?https://test.com");
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search?https%3A%2F%2Ftest.com");
}

