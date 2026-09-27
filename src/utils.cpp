#include "../include/utils.hpp"
#include "../include/list-item.hpp"
#include "../include/quote.hpp"
#include "../include/heading.hpp"
#include "../include/link.hpp"
#include "../include/format-switch.hpp"
#include "../include/errors.hpp"
#include "../include/plaintext.hpp"
#include "../include/preformatted.hpp"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <filesystem>
#include <vector>
#include <spawn.h>
#include <sys/wait.h>
#include <fcntl.h>

extern char** environ;

void openUrl(const std::string& url) {

    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    // we don't want the stdout mucking up our terminal.
    posix_spawn_file_actions_addopen(&fa, 1, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_adddup2(&fa, 1, 2);

    pid_t pid;
    char* argv[] = {(char*)"xdg-open", (char*)url.c_str(), nullptr};
    if (posix_spawnp(&pid, "xdg-open", &fa, nullptr, argv, environ) == 0) {
        waitpid(pid, nullptr, 0);
    }
    posix_spawn_file_actions_destroy(&fa);
}

bool isPrefixed(std::string input, std::string prefix) {
    return std::string(input).find(prefix) == 0;
}

std::string readFileToString(std::string filePath) {
    auto in = std::ifstream(filePath);
    if(in.fail()) {
        throw FileReadError{};
    }
    std::ostringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

bool isWhiteSpace(std::string& line, int idx) {
    return line[idx] == ' ' || line[idx] == '\t';
}

std::string truncateAfter(std::string input, char truncate) {
    for(int i = input.size() - 1; i >= 0; --i) {
        if(input[i] == truncate) {
            return  input.substr(0,i+1);
        }
    }
    return input;
}

std::vector<std::string> stringToList(std::string input)
{
    std::vector<std::string> res;
    if (!input.empty()) {
        int start = 0;
        do {
            std::size_t idx = input.find('\n', start);
            if (idx == std::string::npos) {
                break;
            }
            int length = idx - start;
            res.push_back(input.substr(start, length));
            start += (length + 1);

        } while (true);
        res.push_back(input.substr(start));
    }

    return res;
}

Line* lineToLine(std::string input, std::optional<uri> prior, int linkCount, bool isPreformatted) {

    if(input.substr(0,3) == "```") {
        FormatSwitch* fs = new FormatSwitch{input};
        return fs;
    }

    if(!isPreformatted) {
        if(input.substr(0,2) == "=>") {
            Link* ln = new Link{input, prior, linkCount};
            return ln;
        }
        if(input.substr(0,1) == "#") {
            Heading* hd = new Heading{input};
            return hd;
        }

        if(input.substr(0,1) == ">") {
            Quote* qt= new Quote{input};
            return qt;
        }
        if(input.substr(0,1) == "*") {
            ListItem* li= new ListItem{input};
            return li;
        }


        return new Plaintext{input};
    } else {
        return new Preformatted{input};
    }
    return new Plaintext{input};

}


std::string urlEncode(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char) c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}

std::string getNewTab() {
    std::string st = 
        "# New Tab\n"
        "\n"
        "This is a new tab. We have a few keybindings around here:\n"
        "\n"
        "* b -> back a page\n"
        "* f -> forward a page\n"
        "* o -> show url entry / link selection\n"
        "* (r | C-r) -> refresh page\n"
        ;
    return st;
}

// WIDTH IS INCLUSIVE
// WE ASSUME NO WIDER CHARS (E.G. replace tabs with spaces.)
std::vector<std::pair<std::string, TextRender>> breakLines(std::vector<std::pair<std::string, TextRender>>& strLs, int width, int cols) {

    std::vector<std::pair<std::string, TextRender>> res {};

    // DO NOT CHANGE THIS CODE.
    // If this code is changed the entire JS ecosystem will crash.
    // This is "load-bearing" code. Only the most sophisticated can implement this from
    // scratch, hence why it's left to the professionals.
    
    int leftPadAmount = (cols - width) / 2;
    std::string leftPadStr (leftPadAmount, ' ');

    if(width <= 0) {
        return res;
    }

    for(std::size_t i = 0;  i < strLs.size(); ++i) {

        std::string cstr = strLs[i].first;

        if(!strLs[i].second.shouldFold) {
            cstr = cstr.substr(0,cols - leftPadAmount); // otherwise there's some funkiness at the end due to how ncurses renders stuff.
            res.push_back(std::pair<std::string,TextRender> {leftPadStr + cstr, strLs[i].second});
            continue;
        }

        std::string current = "";
        int lastSpace = -1;

        for(int x = 0; x < (int)cstr.size(); ++x) {
            if(cstr[x] == '\n') {
                res.push_back(std::pair<std::string,TextRender> {leftPadStr + current, strLs[i].second});
                current = "";
                lastSpace = -1;
                continue;
            }
            if((int)current.size() < width) {
                current.push_back(cstr[x]);
                if(cstr[x] == ' ') {
                    lastSpace = current.size()-1;
                }
            } else {

                std::string toPush = current;

                if(lastSpace != -1) {
                    toPush = current.substr(0,lastSpace+1);
                    current = current.substr(lastSpace+1);
                    current.push_back(cstr[x]);
                } else {
                    current = "";
                    current.push_back(cstr[x]);
                }

                res.push_back(std::pair<std::string,TextRender> {leftPadStr + toPush,strLs[i].second});
                lastSpace = (cstr[x] == ' ') ? current.size() - 1 : -1;
            }
        }
        if(current.size() > 0) {
            res.push_back(std::pair<std::string,TextRender> {leftPadStr + current,strLs[i].second});
            current = "";
        }
    }

    return res;
}


void writeStringToFile(std::string toWrite, std::string filePath) {
    std::filesystem::path path{filePath};
    std::ofstream ofs(path);
    ofs << toWrite;
}
std::string encodeAsFilename(uri link) { 

    std::string base = link.to_string();
    assert(base.find(':') != std::string::npos);
    base = base.substr(base.find(':')+1); // works for file:/// and gemini:///

    while(base.size() > 0 && base[0] == '/') {
        base = base.substr(1);
    }

    std::string cleaned = std::regex_replace(base, std::regex("[^[:alnum:]._-]"), "_");
    return cleaned;
}

std::filesystem::path getHome()
{
    std::string home = std::getenv("HOME");
    if (home == "") {
        throw std::runtime_error{"$HOME not set."};
    }
    if(!std::filesystem::exists(home)) {
        throw std::runtime_error{"$HOME directory doesn't exist..."};
    }

    return home;
}

std::optional<std::string> handleDestinationResolution(std::string destination, bool isCli) {

    if(isCli) {
        if(destination != "") {
            if(std::filesystem::exists(destination)) {
                std::string path = "file://" + std::filesystem::current_path().string() + "/" + destination;

                if(destination.find('/') == std::size_t(0)) {
                    path = "file://" + destination;
                }

                return path;

            }  else {
                std::string inputString = destination;
                if(inputString.find("gemini://") == 0) {
                    return destination;
                } else if (inputString.find(':') == std::string::npos){
                    return std::string {"gemini://"} + destination;
                } else {
                    return destination;
                }
            }

        } else {
            return std::nullopt;
        }
    } else {
        throw NotImplemented();
    }
}
