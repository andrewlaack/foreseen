// Testing pure functions

#include <rapidcheck.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <optional>
#include <rapidcheck/Check.h>
#include <utility>
#include "../include/browser.hpp"
#include "../include/utils.hpp"

TEST_CASE("Test cli input handling") {

    SECTION("CLI inputs maintain arbitrary schemas") {
        std::unordered_map<std::string, std::string> expectations;
        expectations["https://github.com"] = "https://github.com";          
        expectations["whatever://github.com"] = "whatever://github.com";   
        expectations["about://github.com"] = "about://github.com"; 
        expectations["gopher://github.com"] = "gopher://github.com";
        expectations["gemini://github.com"] = "gemini://github.com";
        expectations["file:///home/whatever"] = "file:///home/whatever";
        for(auto& expect : expectations) {
            REQUIRE(handleDestinationResolution(expect.first, true).destination == expect.second);
            REQUIRE(handleDestinationResolution(expect.first, true).t == STRING_DESTINATION);
        }
    }

    SECTION("Resolve relative file paths via the CLI if they exist") {
        std::unordered_map<std::string, std::string> expectations;
        std::string cwd = std::filesystem::current_path();
        expectations["tests/sites/basic.gmi"] = "file://" + cwd + "/tests/sites/basic.gmi";
        expectations["tests/sites/basic_2.gmi"] = "file://" + cwd + "/tests/sites/basic_2.gmi";
        expectations[cwd + "/tests/sites/basic_2.gmi"] = "file://" + cwd + "/tests/sites/basic_2.gmi";
        for(auto& expect : expectations) {
            REQUIRE(handleDestinationResolution(expect.first, true).destination == expect.second);
            REQUIRE(handleDestinationResolution(expect.first, true).t == STRING_DESTINATION);
        }
    }

    SECTION("CLI inputs without schema that aren't files") {
        std::unordered_map<std::string, std::string> expectations;
        expectations["tlgs.one"] = "gemini://tlgs.one";
        expectations["laack.co"] = "gemini://laack.co";
        expectations["blog.laack.co/pygame-vs-raylib.gmi"] = "gemini://blog.laack.co/pygame-vs-raylib.gmi";
        for(auto& expect : expectations) {
            REQUIRE(handleDestinationResolution(expect.first, true).destination == expect.second);
            REQUIRE(handleDestinationResolution(expect.first, true).t == STRING_DESTINATION);
        }
    }


}

TEST_CASE("Verify urls with numeric prefix are resolved correctly") {
    REQUIRE(handleDestinationResolution("123movies.com",false).t == STRING_DESTINATION);
    REQUIRE(handleDestinationResolution("123movies.com",false).destination == "gemini://123movies.com");
}

TEST_CASE("Inputs with spaces outside of the cli are always searched") {
    auto d1 = handleDestinationResolution("what :// :// test", false);
    REQUIRE(d1.destination == "gemini://tlgs.one/search?what%20%3A%2F%2F%20%3A%2F%2F%20test");
    rc::check("Inputs with spaces outside of the cli are always searched",
            [](const std::string& st) {
                auto dest = handleDestinationResolution(st, false);
                if(st.find(' ') != std::string::npos) {
                    RC_ASSERT(dest.destination.find("tlgs.one") != std::string::npos);
                }
            });
}


TEST_CASE("Test user input handling for destinations") {

    SECTION("Convert numbers to link numbers") {
        for(int i = 0; i < 10000; ++i) {
            REQUIRE(handleDestinationResolution(std::to_string(i), false).linkNumber == i);
            REQUIRE(handleDestinationResolution(std::to_string(i), false).t == NUMBER_DESTINATION);
        }
    }

    SECTION("Transparently resolve gemini:// prefixed urls") {
        REQUIRE(handleDestinationResolution("gemini://test.com", false).t == STRING_DESTINATION);
        REQUIRE(handleDestinationResolution("gemini://test.com", false).destination == "gemini://test.com");
    }

    SECTION("Convert input with spaces into query") {
        REQUIRE(handleDestinationResolution("what is the capital of scotland?", false).t == STRING_DESTINATION);
        REQUIRE(handleDestinationResolution("what is the capital of scotland?", false).destination == "gemini://tlgs.one/search?" + urlEncode("what is the capital of scotland?"));
    }

    SECTION("Convert {domain} without scheme -> gemini://{domain}") {
        REQUIRE(handleDestinationResolution("laack.co", false).t == STRING_DESTINATION);
        REQUIRE(handleDestinationResolution("laack.co", false).destination == "gemini://laack.co");
    }

    SECTION("Transparently resolve file names") {
        REQUIRE(handleDestinationResolution("file:///test/whatever", false).t == STRING_DESTINATION);
        REQUIRE(handleDestinationResolution("file:///test/whatever", false).destination == "file:///test/whatever");
    }

    SECTION("Convert empty string to no-destination") {
        REQUIRE(handleDestinationResolution("", false).t == NO_DESTINATION);
    }

    SECTION("Handle arbitrary string inputs for CLI input") {
        rc::check("Handle arbitrary string inputs for CLI input",
                [](const std::string& st) {
                    auto dest = handleDestinationResolution(st, true);
                    RC_ASSERT(dest.t != NUMBER_DESTINATION);
                    switch (dest.t) {
                        case NO_DESTINATION:
                            break;
                        case STRING_DESTINATION:
                            break;
                        case NUMBER_DESTINATION:
                            break;
                    }
                });
    }

    SECTION("Handle arbitrary string inputs for non-CLI input") {
        rc::check("Handle arbitrary string inputs for non-CLI input",
                [](const std::string& st) {
                    auto dest = handleDestinationResolution(st, false);

                    if(dest.t == STRING_DESTINATION) {
                        RC_ASSERT(dest.destination != "");
                    }
                });
    }


}


TEST_CASE("Test width == 0 doesn't crash program") {
    std::vector<std::pair<std::string, TextRender>> strLs {};
    for(int i = 0; i < 10; ++i) {
        std::pair<std::string, TextRender> current {"this is a simple test line", TextRender{10,false}};
        strLs.push_back(current);
    }
    REQUIRE_NOTHROW(breakLines(strLs, 0,80));
}

TEST_CASE("Don't fold doesn't fold lines") {
    std::vector<std::pair<std::string, TextRender>> strLs {};
    for(int i = 0; i < 10; ++i) {
        std::pair<std::string, TextRender> current {"this is a simple test line", TextRender{10,true,false}};
        strLs.push_back(current);
    }
    auto result = breakLines(strLs, 10,80);
    REQUIRE(result.size() ==  strLs.size());
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
    SECTION("Plaintext classification") {
        auto res1 = lineToLine("",  std::nullopt, -1, false);
        REQUIRE(res1->type() == PLAINTEXT);
    }

    SECTION("Preformatted classification") {
        auto res2 = lineToLine("",  std::nullopt, -1, true);
        REQUIRE(res2->type() == PREFORMATTED);
    }

    SECTION("Link classification") {
        auto res3 = lineToLine("=> gemini://laack.co",  std::nullopt, 1, false);
        REQUIRE(res3->type() == LINK);
    }

    SECTION("Format switch classification") {
        auto res4 = lineToLine("```",  std::nullopt, 1, false);
        REQUIRE(res4->type() == FORMAT_SWITCH);
    }

    SECTION("H1 classification") {
        auto res5 = lineToLine("# H1 Heading",  std::nullopt, 1, false);
        REQUIRE(res5->type() == H1);
    }

    SECTION("H2 classification") {
        auto res6 = lineToLine("## H2 Heading",  std::nullopt, 1, false);
        REQUIRE(res6->type() == H2);
    }

    SECTION("H3 classification") {
        auto res7 = lineToLine("### H3 Heading",  std::nullopt, 1, false);
        REQUIRE(res7->type() == H3);
    }

    SECTION("Quote classification") {
        auto res8 = lineToLine("> test quote",  std::nullopt, 1, false);
        REQUIRE(res8->type() == QUOTE);
    }

    SECTION("List item classification") {
        auto res9 = lineToLine("* test li",  std::nullopt, 1, false);
        REQUIRE(res9->type() == LIST_ITEM);
    }


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
                REQUIRE(ln->textToDraw() == prefix.first + " test line content\n");
                acc += options[rand() % 2];
            }
        }
    }

    std::string acc = options[rand() % 2];

    rc::check("any non-zero number of whitespaces results in the same link text",
            [](const std::vector<bool> &l0) {
                std::string acc = "";
                if(l0.size() == 0) {
                    if(rand() % 2 == 0) {
                        acc = " ";
                    } else {
                        acc = "\t";
                    }
                }
                for(bool i : l0) {
                    if(i) {
                        acc += '\t';
                    } else {
                        acc += ' ';
                    }
                }
                std::string check = "=>" + acc + "gemini://laack.co" + acc + "link human text";
                auto ln = lineToLine(check,  std::nullopt, 1, false);
                RC_ASSERT(ln->type() == LINK);
                RC_ASSERT(ln->textToDraw() == "[1] link human text\n");
                REQUIRE(ln->type() == LINK);
                REQUIRE(ln->textToDraw() == "[1] link human text\n");
            });
}


TEST_CASE("Proper whitespace compliance") {
    SECTION("Line types without white spacing") {
        auto res1 = lineToLine("",  std::nullopt, -1, false);
        REQUIRE(res1->type() == PLAINTEXT);

        auto res3 = lineToLine("=>gemini://laack.co",  std::nullopt, 1, false);
        REQUIRE(res3->type() == LINK);

        auto res5 = lineToLine("#H1 Heading",  std::nullopt, 1, false);
        REQUIRE(res5->type() == H1);

        auto res6 = lineToLine("##H2 Heading",  std::nullopt, 1, false);
        REQUIRE(res6->type() == H2);

        auto res7 = lineToLine("###H3 Heading",  std::nullopt, 1, false);
        REQUIRE(res7->type() == H3);

        auto res8 = lineToLine(">test quote",  std::nullopt, 1, false);
        REQUIRE(res8->type() == QUOTE);

        auto res9 = lineToLine("*test li",  std::nullopt, 1, false);
        REQUIRE(res9->type() == LIST_ITEM);
    }

    SECTION("Non-whitespace based line types don't require whitespace") {
        auto res2 = lineToLine("",  std::nullopt, -1, true);
        REQUIRE(res2->type() == PREFORMATTED);

        auto res4 = lineToLine("```",  std::nullopt, 1, false);
        REQUIRE(res4->type() == FORMAT_SWITCH);
    }


}


TEST_CASE("Line parser handles lines that are only format characters with whitespaces") {
    auto res1 = lineToLine(" ",  std::nullopt, -1, false);
    REQUIRE(res1->type() == PLAINTEXT);

    auto res2 = lineToLine(" ",  std::nullopt, -1, true);
    REQUIRE(res2->type() == PREFORMATTED);

    auto res3 = lineToLine("=> ",  std::nullopt, 1, false);
    REQUIRE(res3->type() == LINK);

    auto res4 = lineToLine("``` ",  std::nullopt, 1, false);
    REQUIRE(res4->type() == FORMAT_SWITCH);

    auto res5 = lineToLine("# ",  std::nullopt, 1, false);
    REQUIRE(res5->type() == H1);

    auto res6 = lineToLine("## ",  std::nullopt, 1, false);
    REQUIRE(res6->type() == H2);

    auto res7 = lineToLine("### ",  std::nullopt, 1, false);
    REQUIRE(res7->type() == H3);

    auto res8 = lineToLine("> ",  std::nullopt, 1, false);
    REQUIRE(res8->type() == QUOTE);

    auto res9 = lineToLine("* ",  std::nullopt, 1, false);
    REQUIRE(res9->type() == LIST_ITEM);
}

// I was wrong on this originally bc the spec states whitespaces are optional.
TEST_CASE("Line parser handles lines that are only format characters without whitespaces") {
    auto res1 = lineToLine("",  std::nullopt, -1, false);
    REQUIRE(res1->type() == PLAINTEXT);

    auto res2 = lineToLine("",  std::nullopt, -1, true);
    REQUIRE(res2->type() == PREFORMATTED);

    auto res3 = lineToLine("=>",  std::nullopt, 1, false);
    REQUIRE(res3->type() == LINK);

    auto res4 = lineToLine("```",  std::nullopt, 1, false);
    REQUIRE(res4->type() == FORMAT_SWITCH);

    auto res5 = lineToLine("#",  std::nullopt, 1, false);
    REQUIRE(res5->type() == H1);

    auto res6 = lineToLine("##",  std::nullopt, 1, false);
    REQUIRE(res6->type() == H2);

    auto res7 = lineToLine("###",  std::nullopt, 1, false);
    REQUIRE(res7->type() == H3);

    auto res8 = lineToLine(">",  std::nullopt, 1, false);
    REQUIRE(res8->type() == QUOTE);

    auto res9 = lineToLine("*",  std::nullopt, 1, false);
    REQUIRE(res9->type() == LIST_ITEM);
}

static std::string linecharset = "#>=abc##  de_-><0193248$#)(&@)(*&$#^*&#^%^&*&fghijklmnopqrstuvwxyz #````ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

TEST_CASE("Crash test random line strings") {
    rc::check("Crash test random line strings",
        [](const std::string& st) {
            auto* ln = lineToLine(st, std::nullopt, 1, false);
            delete ln;
        });
}

TEST_CASE("Test cache never exceeds CACHE_SIZE") {
    const int size = CACHE_SIZE;
    Cache c{};

    for (int i = 0; i < 10000; ++i) {
        c.addSite("gemini://" + std::to_string(i), Site("header", "body"));
        if (i >= size) {
            REQUIRE(c.getSite("gemini://" + std::to_string(i - size)) == std::nullopt);
        }
        int oldest = std::max(0, i - size + 1);
        REQUIRE(c.getSite("gemini://" + std::to_string(oldest)) != std::nullopt);
    }
}
