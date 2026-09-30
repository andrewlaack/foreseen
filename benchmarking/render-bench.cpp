#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <rapidcheck.h>
#include "../include/render.hpp"
#include <ncurses.h>
#include <rapidcheck/Check.h>
#include <rapidcheck/Log.h>
#include <string>

TEST_CASE("Benchmarking small file") {

    setenv("EDITOR", "test" , 1);

    for(int x = 8; x < 10; ++x) {
        for(int y = 8; y < 10; ++y) {
            BENCHMARK("All line types - X: " + std::to_string(x) + " Y: " + std::to_string(y)) {
                Browser* bPtr = new Browser {};
                Browser& b = *bPtr;
                DrawState ds {};
                ds.bPtr = bPtr;
                std::string st = "file:///home/andrew/gitRepos/gemini-browser/tests/sites/all_line_types.gmi";


                REQUIRE(mainLoop(ds, b, 'o',x,y));

                for(auto& ch: st) {
                    REQUIRE(mainLoop(ds, b, ch,x,y));
                }

                REQUIRE(mainLoop(ds, b, '\n',x,y));

                REQUIRE(b.getCurrentLink()->getLinkDestination().to_string() == "file:///home/andrew/gitRepos/gemini-browser/tests/sites/all_line_types.gmi");
                delete bPtr;
            };
        }
    }
}

