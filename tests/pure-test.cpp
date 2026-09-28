// Testing pure functions
#include <catch2/catch_test_macros.hpp>
#include <climits>
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
        REQUIRE(handleDestinationResolution(expect.first, true).destination == expect.second);
        REQUIRE(handleDestinationResolution(expect.first, true).t == STRING_DESTINATION);
    }

}

TEST_CASE("Edge cases for input handling") {
    REQUIRE(handleDestinationResolution("123movies.com",false).t == STRING_DESTINATION);
    REQUIRE(handleDestinationResolution("123movies.com",false).destination == "gemini://123movies.com");
}

TEST_CASE("Test user input handling for destinations") {

    for(int i = 0; i < 10000; ++i) {
        REQUIRE(handleDestinationResolution(std::to_string(i), false).linkNumber == i);
        REQUIRE(handleDestinationResolution(std::to_string(i), false).t == NUMBER_DESTINATION);
    }

    REQUIRE(handleDestinationResolution("gemini://test.com", false).t == STRING_DESTINATION);
    REQUIRE(handleDestinationResolution("what is the capital of scotland?", false).t == STRING_DESTINATION);
    REQUIRE(handleDestinationResolution("laack.co", false).t == STRING_DESTINATION);
    REQUIRE(handleDestinationResolution("file:///test/whatever", false).t == STRING_DESTINATION);

    REQUIRE(handleDestinationResolution("gemini://test.com", false).destination == "gemini://test.com");
    REQUIRE(handleDestinationResolution("what is the capital of scotland?", false).destination == "gemini://tlgs.one/search?" + urlEncode("what is the capital of scotland?"));
    REQUIRE(handleDestinationResolution("laack.co", false).destination == "gemini://laack.co");

    REQUIRE(handleDestinationResolution("file:///test/whatever", false).destination == "file:///test/whatever");
    REQUIRE(handleDestinationResolution("", false).t == NO_DESTINATION);
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

TEST_CASE("Test line parsing handles whitespace correctly") {

    std::unordered_map<std::string, LineType> prefixes {};

    prefixes["*"]  = LIST_ITEM;
    prefixes[">"] = QUOTE;
    prefixes["#"] = H1;
    prefixes["##"] = H2;
    prefixes["###"] = H3;

    std::vector<std::string> options{" ", "\t"};

    for(auto& prefix: prefixes) {
        for(int x = 0; x < 100; ++x) {

            std::string acc = options[rand() % 2];

            for(int i = 0; i < 1000; ++i) {
                std::string check = prefix.first +  acc + "test line content";
                auto ln = lineToLine(check,  std::nullopt, 1, false);
                REQUIRE(ln->type() == prefix.second);
                REQUIRE(ln->textToDraw() == prefix.first + " test line content");
                acc += options[rand() % 2];
            }
        }
    }

    std::string acc = options[rand() % 2];

    // TODO: Should add tests for non-human readable links as well. 
    for(int x = 0; x < 10; ++x) {
        for(int i = 0; i < 10; ++i) {
            std::string check = "=>" + acc + "gemini://laack.co" + acc + "link human text";
            auto ln = lineToLine(check,  std::nullopt, 1, false);
            REQUIRE(ln->type() == LINK);
            REQUIRE(ln->textToDraw() == "[1] link human text");
            acc += options[rand() % 2];
        }
    }
}

TEST_CASE("Proper whitespace compliance") {
    auto res1 = lineToLine("",  std::nullopt, -1, false);
    REQUIRE(res1->type() == PLAINTEXT);

    auto res2 = lineToLine("",  std::nullopt, -1, true);
    REQUIRE(res2->type() == PREFORMATTED);

    auto res3 = lineToLine("=>gemini://laack.co",  std::nullopt, 1, false);
    REQUIRE(res3->type() == PLAINTEXT);

    auto res4 = lineToLine("```",  std::nullopt, 1, false);
    REQUIRE(res4->type() == FORMAT_SWITCH);

    auto res5 = lineToLine("#H1 Heading",  std::nullopt, 1, false);
    REQUIRE(res5->type() == PLAINTEXT);

    auto res6 = lineToLine("##H2 Heading",  std::nullopt, 1, false);
    REQUIRE(res6->type() == PLAINTEXT);

    auto res7 = lineToLine("###H3 Heading",  std::nullopt, 1, false);
    REQUIRE(res7->type() == PLAINTEXT);

    auto res8 = lineToLine(">test quote",  std::nullopt, 1, false);
    REQUIRE(res8->type() == PLAINTEXT);

    auto res9 = lineToLine("*test li",  std::nullopt, 1, false);
    REQUIRE(res9->type() == PLAINTEXT);


}

TEST_CASE("Crash test") {
    auto res1 = lineToLine("",  std::nullopt, -1, false);
    REQUIRE(res1->type() == PLAINTEXT);

    auto res2 = lineToLine("",  std::nullopt, -1, true);
    REQUIRE(res2->type() == PREFORMATTED);

    auto res3 = lineToLine("=>",  std::nullopt, 1, false);
    REQUIRE(res3->type() == PLAINTEXT);

    auto res4 = lineToLine("```",  std::nullopt, 1, false);
    REQUIRE(res4->type() == FORMAT_SWITCH);

    auto res5 = lineToLine("#",  std::nullopt, 1, false);
    REQUIRE(res5->type() == PLAINTEXT);

    auto res6 = lineToLine("##",  std::nullopt, 1, false);
    REQUIRE(res6->type() == PLAINTEXT);

    auto res7 = lineToLine("###",  std::nullopt, 1, false);
    REQUIRE(res7->type() == PLAINTEXT);

    auto res8 = lineToLine(">",  std::nullopt, 1, false);
    REQUIRE(res8->type() == PLAINTEXT);

    auto res9 = lineToLine("*",  std::nullopt, 1, false);
    REQUIRE(res9->type() == PLAINTEXT);
}

static std::string linecharset = "#>=abc##  de_-><0193248$#)(&@)(*&$#^*&#^%^&*&fghijklmnopqrstuvwxyz #````ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

TEST_CASE("Crash test rng") {
    for(int x = 0; x < 10000; ++x) {
        std::string strRnd = "";
        for(int i = 0; i < 1000; ++i) {
            strRnd += linecharset[rand() % linecharset.length()];
            auto* ln = lineToLine(strRnd, std::nullopt, 1, false);
        }
    }
    for(int x = 0; x < 10000; ++x) {
        std::string strRnd = "";
        for(int i = 0; i < 1000; ++i) {
            strRnd += linecharset[rand() % linecharset.length()];
            auto* ln = lineToLine(strRnd, std::nullopt, 1, true);
        }
    }

}
