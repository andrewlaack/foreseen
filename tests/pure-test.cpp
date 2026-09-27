// Testing pure functions
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <optional>
#include <utility>
#include "../include/browser.hpp"
#include "../include/utils.hpp"

TEST_CASE("Test cli input handling") {
    std::unordered_map<std::string, std::string> expectations;

    // Maintain arbitrary schemas
    expectations["https://github.com"] = "https://github.com";          
    expectations["whatever://github.com"] = "whatever://github.com";   
    expectations["about://github.com"] = "about://github.com"; 
    expectations["gopher://github.com"] = "gopher://github.com";
    expectations["gemini://github.com"] = "gemini://github.com";
    expectations["file:///home/whatever"] = "file:///home/whatever";

    // resolve relative file paths if they exist
    std::string cwd = std::filesystem::current_path();
    expectations["tests/sites/basic.gmi"] = "file://" + cwd + "/tests/sites/basic.gmi";
    expectations["tests/sites/basic_2.gmi"] = "file://" + cwd + "/tests/sites/basic_2.gmi";

    // resolve absolute paths correctly
    expectations[cwd + "/tests/sites/basic_2.gmi"] = "file://" + cwd + "/tests/sites/basic_2.gmi";

    // handle specified inputs without schema that aren't files
    expectations["tlgs.one"] = "gemini://tlgs.one";
    expectations["laack.co"] = "gemini://laack.co";
    expectations["blog.laack.co/pygame-vs-raylib.gmi"] = "gemini://blog.laack.co/pygame-vs-raylib.gmi";

    for(auto& expect : expectations) {
        REQUIRE(handleCLIDestination(expect.first) == expect.second);
    }

}

TEST_CASE("Test trivial line breaking") {
    std::vector<std::pair<std::string, TextRender>> strLs {};
    for(int i = 0; i < 10; ++i) {
        std::pair<std::string, TextRender> current {"this is a simple test line", TextRender{10,false}};
        strLs.push_back(current);
    }
    auto result = breakLines(strLs, 10,80);
    REQUIRE(result.size() ==  30);
}

static std::string charset = "abc defghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

TEST_CASE("Test width invariant") {
    
    std::vector<std::pair<std::string, TextRender>> strLs {};

    srand(std::time(NULL));

    for(int i = 0; i < 100; ++i) {
        std::string strRnd;
        int ub = rand() % 100;
        for(int x = 0; x < ub; ++x) {
            strRnd += charset[rand() % charset.length()];
        }
        std::pair<std::string, TextRender> current {strRnd, TextRender{10, false}};
        strLs.push_back(current);
    }

    auto result = breakLines(strLs, 80,80);

    for(auto& res: result) {
        REQUIRE(res.first.size() <= 80);
    }

}

TEST_CASE("Test lots of spaces") {
    
    std::vector<std::pair<std::string, TextRender>> strLs {};

    srand(std::time(NULL));

    for(int i = 0; i < 10; ++i) {
        std::string strRnd;
        int ub = rand() % 100;
        for(int x = 0; x < ub; ++x) {
            strRnd += ' ';
        }
        std::pair<std::string, TextRender> current {strRnd, TextRender{10, false}};
        strLs.push_back(current);
    }

    auto result = breakLines(strLs, 80,80);

    for(auto& res: result) {
        REQUIRE(res.first.size() <= 80);
    }

}

TEST_CASE("Test normal line classification") {
    auto res1 = lineToLine("",  std::nullopt, -1, false);
    REQUIRE(res1->type() == PLAINTEXT);

    auto res2 = lineToLine("",  std::nullopt, -1, true);
    REQUIRE(res2->type() == PREFORMATTED);

    auto res3 = lineToLine("=> gemini://laack.co",  std::nullopt, 1, false);
    REQUIRE(res3->type() == LINK);

    auto res4 = lineToLine("```",  std::nullopt, 1, false);
    REQUIRE(res4->type() == FORMAT_SWITCH);

    auto res5 = lineToLine("# H1 Heading",  std::nullopt, 1, false);
    REQUIRE(res5->type() == H1);

    auto res6 = lineToLine("## H2 Heading",  std::nullopt, 1, false);
    REQUIRE(res6->type() == H2);

    auto res7 = lineToLine("### H3 Heading",  std::nullopt, 1, false);
    REQUIRE(res7->type() == H3);

    auto res8 = lineToLine("> test quote",  std::nullopt, 1, false);
    REQUIRE(res8->type() == QUOTE);

    auto res9 = lineToLine("* test li",  std::nullopt, 1, false);
    REQUIRE(res9->type() == LIST_ITEM);


}

