#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <rapidcheck.h>
#include "../include/render.hpp"
#include <ncurses.h>
#include <rapidcheck/Check.h>
#include <rapidcheck/Log.h>
#include <string>

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
TEST_CASE("Benchmark web browsing") {
    setenv("EDITOR", "test" , 1);

    BENCHMARK("Access my website") {
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
    };
}

TEST_CASE("Benchmarking huge file scrolling") {
    setenv("EDITOR", "test" , 1);
    writeStringToFile(genAlNumNlSp(10000000), "tests/example.out");
    BENCHMARK("5 passes per iteration, 10m character file, load, scroll down 1000x, scroll up 1000x with 100x100 emulated terminal size") {
            for(int i = 0; i < 5; ++i) {
                Browser* bPtr = new Browser {};
                Browser& b = *bPtr;
                DrawState ds {};
                ds.bPtr = bPtr;
                std::string cwd = std::filesystem::current_path();
                std::string st = "file://" + cwd + "/tests/example.out";

                REQUIRE(mainLoop(ds, b, 'o',100,100));
                for(auto& ch: st) {
                    REQUIRE(mainLoop(ds, b, ch,100,100));
                }
                REQUIRE(mainLoop(ds, b, '\n',100,100));
                REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == st);

                for(int i = 0; i < 1000; ++i) {
                    REQUIRE(mainLoop(ds, b, KEY_DOWN,100,100));
                }
                for(int i = 0; i < 1000; ++i) {
                    REQUIRE(mainLoop(ds, b, KEY_UP,100,100));
                }
            delete bPtr;
        }
    };
}


TEST_CASE("Benchmarking large file scrolling", "[.]") {
    setenv("EDITOR", "test" , 1);
    writeStringToFile(genAlNumNlSp(100000), "tests/example.out");
    BENCHMARK("100k character file, load, scroll down 100x, scroll up 100x with 100x100 emulated terminal size") {
        Browser* bPtr = new Browser {};
        Browser& b = *bPtr;
        DrawState ds {};
        ds.bPtr = bPtr;
        std::string cwd = std::filesystem::current_path();
        std::string st = "file://" + cwd + "/tests/example.out";

        REQUIRE(mainLoop(ds, b, 'o',100,100));
        for(auto& ch: st) {
            REQUIRE(mainLoop(ds, b, ch,100,100));
        }
        REQUIRE(mainLoop(ds, b, '\n',100,100));
        REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == st);

        for(int i = 0; i < 100; ++i) {
            REQUIRE(mainLoop(ds, b, KEY_DOWN,100,100));
        }
        for(int i = 0; i < 100; ++i) {
            REQUIRE(mainLoop(ds, b, KEY_UP,100,100));
        }

        delete bPtr;
    };
}


TEST_CASE("Benchmarking small file") {

    setenv("EDITOR", "test" , 1);

    for(int x = 8; x < 10; ++x) {
        for(int y = 8; y < 10; ++y) {
            BENCHMARK("All line types - X: " + std::to_string(x) + " Y: " + std::to_string(y)) {
                Browser* bPtr = new Browser {};
                Browser& b = *bPtr;
                DrawState ds {};
                ds.bPtr = bPtr;
                std::string cwd = std::filesystem::current_path();
                std::string st = "file://" + cwd + "/tests/all_line_types.gmi";


                REQUIRE(mainLoop(ds, b, 'o',x,y));

                for(auto& ch: st) {
                    REQUIRE(mainLoop(ds, b, ch,x,y));
                }

                REQUIRE(mainLoop(ds, b, '\n',x,y));

                REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == st);
                delete bPtr;
            };
        }
    }
}

