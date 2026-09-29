#include <catch2/catch_test_macros.hpp>
#include <experimental/filesystem>
#include <rapidcheck.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <filesystem>
#include <rapidcheck/Assertions.h>
#include <utility>
#include "../include/browser.hpp"
#include "../include/utils.hpp"


std::string genRandom(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}


// TODO: Refactor this to use $HOME
TEST_CASE( "Basic rendering of all line types" ) {
    Browser b{};
    b.goToSite("file:///home/andrew/gitRepos/gemini-browser/tests/sites/all_line_types.gmi", true);
    REQUIRE(b.getCurrentSite()->getBody() == readFileToString("tests/sites/all_line_types.gmi"));
    auto strLs = b.renderSite();
    auto res = breakLines(strLs, 80,200);
    // TODO: Make this bound tighter by computing left pad amount. 
    for(auto& st: res) {
        if(st.second.shouldFold) {
            REQUIRE(st.first.size() <= 200);
        }
    }
}


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


TEST_CASE("Test browser doesn't crash when accessing sites with specified port number that is not responsive.") {
    Browser b{};
    bool res = b.goToSite("gemini://laack.co:3847");
    REQUIRE(!res);
    b.goForward();
    b.goBack();
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "about://newtab");
}


TEST_CASE("Test browser can access sites with specified (default) port number.") {
    Browser b{};
    b.goToSite("gemini://laack.co:1965");
    REQUIRE(b.getCurrentSite()->getMeta() == "text/gemini;lang=en-US");
}


// TODO:  Test invalid meta lines (like the case where weird stuff is sent from server)
TEST_CASE("Test browser returns proper meta for visited sites.") {
    Browser b{};
    b.goToSite("gemini://laack.co");
    // meta strips status.
    REQUIRE(b.getCurrentSite()->getMeta() == "text/gemini;lang=en-US");
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
    std::string destination = b.tryDownloadPage();
    std::string out = readFileToString(destination);
    REQUIRE(out == b.getCurrentSite()->getBody());
    REQUIRE(std::filesystem::remove(destination));
}


TEST_CASE("Test downloading to specific location") {
    std::filesystem::path cwd = std::filesystem::current_path();

    Browser b{};
    b.goToSite("gemini://blog.laack.co");
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://blog.laack.co");
    std::string destination = b.tryDownloadPage(cwd / "out.gmi");
    std::string out = readFileToString(destination);
    REQUIRE(out == b.getCurrentSite()->getBody());
    REQUIRE(destination == cwd / "out.gmi");
    REQUIRE(std::filesystem::remove(destination));
}


TEST_CASE("Test going forwards doesn't break when accessing an invalid site prior.") {
    Browser b{};
    bool res = b.goToSite("gemini://aroisetnatsr.aoirseaorstie");
    REQUIRE(!res);
    b.goForward();
}

TEST_CASE("Test going backwards doesn't break when accessing an invalid site prior.") {
    Browser b{};
    bool res = b.goToSite("gemini://aroisetnatsr.aoirseaorstie");
    REQUIRE(!res);
    b.goBack();
}

TEST_CASE("Test encoding allows relative linking with : in parameter") {
    Browser b{};
    b.goToSite("gemini://tlgs.one/search");
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search");
    b.goToSite("?https://test.com");
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "gemini://tlgs.one/search?https%3A%2F%2Ftest.com");
}

TEST_CASE("Sanitize characters to draw  tests") {
    Browser b {};
    bool res = b.goToSite("file:///home/andrew/gitRepos/gemini-browser/tests/sites/line-return.gmi");

    REQUIRE(res);
    REQUIRE(b.getCurrentSite()->getStatusCode() >= 20);
    REQUIRE(b.getCurrentSite()->getStatusCode() <= 29);

    auto ls = b.renderSite();
    sanitizeCharactersToDraw(ls);
    for(auto& line : ls) {
        REQUIRE(line.first.find('\r') == std::string::npos);
    }
}

TEST_CASE("Never crash from weird user inputs") {
    for(int i  =  0; i < 5; ++i) {
        Browser b {};

        std::vector<std::string> ls {};
        for(int x = 0; x < 10; ++x) {
            ls.push_back(genRandom(rand() % 5000));
        }

        for(auto& str : ls) {
            auto res = handleDestinationResolution(str, false);
            if(res.t == STRING_DESTINATION) {
                b.goToSite(res.destination);
            } else if (res.t == NUMBER_DESTINATION) {
                b.followLinkNumber(res.linkNumber);
            }
            b.refresh();
            b.goBack();
            REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "about://newtab");
        }
    }
}
