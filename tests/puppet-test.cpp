#include <ncurses.h>
#include <rapidcheck.h>
#include <rapidcheck/Check.h>
#include <rapidcheck/Log.h>

#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "../include/render.hpp"
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
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;
    REQUIRE_FALSE(mainLoop(ds, b, 'q', 0, 0));
    delete bPtr;
}

TEST_CASE("RESIZE falls through and returns") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;
    REQUIRE(mainLoop(ds, b, KEY_RESIZE, 0, 0));
    delete bPtr;
}

TEST_CASE("Redirect to broken doesn't deadlock") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;
    std::string dest = "ogemini://localhost/cgi-bin/redirect-to-broken.py";
    for (auto& ch : dest) {
        REQUIRE(mainLoop(ds, b, ch, 100, 100));
    }

    for (int i = 0; i < 100; ++i) {
        REQUIRE(mainLoop(ds, b, 'y', 100, 100));
    }

    REQUIRE(mainLoop(ds, b, 'n', 100, 100));
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "about://newtab");
    delete bPtr;
}

TEST_CASE("Open page handler returns") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;
    REQUIRE(mainLoop(ds, b, 'o', 0, 0));
    delete bPtr;
}

TEST_CASE("Cancel 'o' menu by pressing escape") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;
    REQUIRE(mainLoop(ds, b, 'o', 0, 0));
    REQUIRE(mainLoop(ds, b, 'l', 0, 0));
    REQUIRE(mainLoop(ds, b, 'a', 0, 0));
    REQUIRE(mainLoop(ds, b, 'a', 0, 0));
    REQUIRE(mainLoop(ds, b, 'a', 0, 0));
    REQUIRE(mainLoop(ds, b, KEY_BACKSPACE, 0, 0));
    REQUIRE(mainLoop(ds, b, 'c', 0, 0));
    REQUIRE(mainLoop(ds, b, 'k', 0, 0));
    REQUIRE(mainLoop(ds, b, '.', 0, 0));
    REQUIRE(mainLoop(ds, b, 'c', 0, 0));
    REQUIRE(mainLoop(ds, b, 'o', 0, 0));
    REQUIRE(mainLoop(ds, b, 27, 0, 0));  // escape
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "about://newtab");
    delete bPtr;
}

TEST_CASE("Handle redirects") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;
    std::string st = "ogemini://localhost/cgi-bin/redirect.py";
    for (auto& ch : st) {
        REQUIRE(mainLoop(ds, b, ch, 0, 0));
    }

    REQUIRE(mainLoop(ds, b, '\n', 0, 0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "gemini://localhost/cgi-bin/redirect.py");

    REQUIRE(mainLoop(ds, b, 'y', 0, 0));  // follow redirect
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "gemini://localhost");
    delete bPtr;
}

TEST_CASE("Cancel input") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;

    std::string st = "ogemini://localhost/cgi-bin/search.py";
    for (auto& ch : st) {
        REQUIRE(mainLoop(ds, b, ch, 0, 0));
    }

    REQUIRE(mainLoop(ds, b, '\n', 0, 0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "gemini://localhost/cgi-bin/search.py");

    std::string s2 = "test search query";
    for (auto& ch : s2) {
        REQUIRE(mainLoop(ds, b, ch, 0, 0));
    }

    REQUIRE(mainLoop(ds, b, 27, 0, 0));  // escape

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "about://newtab");
    delete bPtr;
}

TEST_CASE("Handle random status codes") {
    for (int i = 0; i < 100; ++i) {
        Browser* bPtr = new Browser{};
        Browser& b = *bPtr;
        DrawState ds{};
        ds.bPtr = bPtr;

        std::string st = "ogemini://localhost/cgi-bin/rnd.py";
        for (auto& ch : st) {
            REQUIRE(mainLoop(ds, b, ch, 0, 0));
        }
        REQUIRE(mainLoop(ds, b, '\n', 0, 0));
        delete bPtr;
    }
}

TEST_CASE("Follow non-existent link") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;

    std::string st = "ogemini://localhost/cgi-bin/";
    for (auto& ch : st) {
        REQUIRE(mainLoop(ds, b, ch, 0, 0));
    }
    REQUIRE(mainLoop(ds, b, '\n', 0, 0));

    REQUIRE(mainLoop(ds, b, 'o', 0, 0));
    REQUIRE(mainLoop(ds, b, '9', 0, 0));
    REQUIRE(mainLoop(ds, b, '9', 0, 0));
    REQUIRE(mainLoop(ds, b, '9', 0, 0));
    REQUIRE(mainLoop(ds, b, '\n', 0, 0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "gemini://localhost/cgi-bin/");

    delete bPtr;
}

TEST_CASE("Handle input status codes") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;

    std::string st = "ogemini://localhost/cgi-bin/search.py";
    for (auto& ch : st) {
        REQUIRE(mainLoop(ds, b, ch, 0, 0));
    }

    REQUIRE(mainLoop(ds, b, '\n', 0, 0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "gemini://localhost/cgi-bin/search.py");

    std::string s2 = "test search query";
    for (auto& ch : s2) {
        REQUIRE(mainLoop(ds, b, ch, 0, 0));
    }

    REQUIRE(mainLoop(ds, b, 127, 0, 0));
    REQUIRE(mainLoop(ds, b, 'y', 0, 0));

    REQUIRE(mainLoop(ds, b, '\n', 0, 0));

    REQUIRE(mainLoop(ds, b, 'n', 0, 0));  // don't follow redirect
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "gemini://localhost/cgi-bin/search.py?" + urlEncode(s2));
    delete bPtr;
}

TEST_CASE("Handle redirect rejection") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;

    std::string st = "ogemini://localhost/cgi-bin/redirect.py";
    for (auto& ch : st) {
        REQUIRE(mainLoop(ds, b, ch, 0, 0));
    }

    REQUIRE(mainLoop(ds, b, '\n', 0, 0));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "gemini://localhost/cgi-bin/redirect.py");

    REQUIRE(mainLoop(ds, b, 'n', 0, 0));  // don't follow redirect
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "about://newtab");
    delete bPtr;
}

TEST_CASE("Handle g and G navigation") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;

    std::string st = "ogemini://localhost/cgi-bin/redirect.py";
    for (auto& ch : st) {
        REQUIRE(mainLoop(ds, b, ch, 100, 100));
    }

    REQUIRE(mainLoop(ds, b, '\n', 100, 100));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "gemini://localhost/cgi-bin/redirect.py");

    REQUIRE(mainLoop(ds, b, 'n', 100, 100));  // don't follow redirect
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "about://newtab");

    REQUIRE(mainLoop(ds, b, 'g', 0, 0));
    REQUIRE(mainLoop(ds, b, 'G', 0, 0));

    REQUIRE(mainLoop(ds, b, 'g', 100, 0));
    REQUIRE(mainLoop(ds, b, 'G', 100, 0));

    REQUIRE(mainLoop(ds, b, 'g', 100, 100));
    REQUIRE(mainLoop(ds, b, 'G', 100, 100));

    REQUIRE(mainLoop(ds, b, 'g', 100, 0));
    REQUIRE(mainLoop(ds, b, 'G', 100, 0));

    delete bPtr;
}

TEST_CASE("Show issue text on non-existent resource request") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;

    std::string st =
        "ogemini://localhost/cgi-bin/this-straight-up-cant-exist.py";
    for (auto& ch : st) {
        REQUIRE(mainLoop(ds, b, ch, 100, 100));
    }

    REQUIRE(mainLoop(ds, b, '\n', 100, 100));

    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "about://newtab");

    REQUIRE(ds.issueText != "");

    REQUIRE(mainLoop(ds, b, 'o', 100, 100));

    delete bPtr;
}

TEST_CASE("Redirect on navigation by link number") {
    Browser* bPtr = new Browser{};
    Browser& b = *bPtr;
    DrawState ds{};
    ds.bPtr = bPtr;

    std::string st = "ogemini://localhost/cgi-bin/";
    for (auto& ch : st) {
        REQUIRE(mainLoop(ds, b, ch, 100, 100));
    }

    REQUIRE(mainLoop(ds, b, '\n', 100, 100));

    REQUIRE(mainLoop(ds, b, 'o', 100, 100));
    REQUIRE(mainLoop(ds, b, '2', 100, 100));
    REQUIRE(mainLoop(ds, b, '\n', 100, 100));

    REQUIRE(mainLoop(ds, b, 'y', 100, 100));
    REQUIRE(ds.issueText == "");
    REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() ==
            "gemini://localhost");

    delete bPtr;
}

TEST_CASE(
    "Only return false on 'q' entry when not using input boxes, never crash, "
    "junk inputs") {
    setenv("EDITOR", "test", 1);

    rc::check("Never return false / fail except with input 'q'",
              [](std::string st) {
                  Browser* bPtr = new Browser{};
                  Browser& b = *bPtr;
                  DrawState ds{};
                  ds.bPtr = bPtr;
                  RC_LOG(st);
                  for (auto& cur : st) {
                      if (cur == 'q') {
                          if (!ds.handleInput && !ds.handleOpenOther &&
                              !ds.handleRedirect) {
                              REQUIRE_FALSE(mainLoop(ds, b, cur, 0, 0));
                          } else {
                              REQUIRE(mainLoop(ds, b, cur, 0, 0));
                          }
                      } else {
                          REQUIRE(mainLoop(ds, b, cur, 0, 0));
                      }
                  }
                  delete bPtr;
              });
}

TEST_CASE(
    "Only return false on 'q' entry when not using input boxes, alphanumeric "
    "inputs") {
    setenv("EDITOR", "test", 1);

    for (int i = 0; i < 10; ++i) {
        Browser* bPtr = new Browser{};
        Browser& b = *bPtr;

        DrawState ds{};
        ds.bPtr = bPtr;
        std::string st = genAlNumNlSp(rand() % 100);
        INFO(st);
        for (auto& cur : st) {
            if (cur == 'q') {
                if (!ds.handleInput && !ds.handleOpenOther &&
                    !ds.handleRedirect) {
                    REQUIRE_FALSE(mainLoop(ds, b, cur, 0, 0));
                } else {
                    REQUIRE(mainLoop(ds, b, cur, 0, 0));
                }
            } else {
                REQUIRE(mainLoop(ds, b, cur, 0, 0));
            }
        }
        delete bPtr;
    }
}

TEST_CASE("Basic usage with small screen sizes") {
    setenv("EDITOR", "test", 1);

    for (int i = 0; i < 10; ++i) {
        int x = rand() % 25;
        int y = rand() % 10;
        Browser* bPtr = new Browser{};
        Browser& b = *bPtr;
        DrawState ds{};
        ds.bPtr = bPtr;

        std::string cwd = std::filesystem::current_path();
        std::string st = "file://" + cwd + "/tests/sites/all_line_types.gmi";

        REQUIRE(mainLoop(ds, b, 'o', x, y));

        for (auto& ch : st) {
            REQUIRE(mainLoop(ds, b, ch, x, y));
        }

        REQUIRE(mainLoop(ds, b, '\n', x, y));

        REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == st);
        delete bPtr;
    }
}
