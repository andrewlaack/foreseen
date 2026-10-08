#include "../include/utils.hpp"

#include <fcntl.h>
#include <spawn.h>
#include <sys/wait.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../include/errors.hpp"
#include "../include/format-switch.hpp"
#include "../include/heading.hpp"
#include "../include/link.hpp"
#include "../include/list-item.hpp"
#include "../include/plaintext.hpp"
#include "../include/preformatted.hpp"
#include "../include/quote.hpp"
#include "../include/shared.hpp"

extern char** environ;

void openUrl(const std::string& url) {
    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    // we don't want the stdout mucking up our terminal.
    posix_spawn_file_actions_addopen(&fa, 1, "/dev/null", O_WRONLY, 0);
    posix_spawn_file_actions_adddup2(&fa, 1, 2);

    pid_t pid;
    char* argv[] = {(char*)openUnknownScheme, (char*)url.c_str(), nullptr};
    if (posix_spawnp(&pid, openUnknownScheme, &fa, nullptr, argv, environ) ==
        0) {
        waitpid(pid, nullptr, 0);
    }
    posix_spawn_file_actions_destroy(&fa);
}

bool isPrefixed(std::string input, std::string prefix) {
    return std::string(input).find(prefix) == 0;
}

std::string readFileToString(std::string filePath) {
    auto in = std::ifstream(filePath);
    if (in.fail()) {
        throw FileReadError{};
    }
    std::ostringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

std::string stripLeadingWhiteSpace(std::string& input) {
    std::size_t x = 0;

    while (x < input.size() && isWhiteSpace(input, x)) {
        x += 1;
    }
    return input.substr(x);
}

bool isWhiteSpace(std::string& line, int idx) {
    if (idx < (int)line.size() && idx >= 0) {
        return line[idx] == ' ' || line[idx] == '\t';
    }
    return false;
}

std::vector<std::string> stringToList(std::string input) {
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

Line* lineToLine(std::string input, std::optional<uri> prior, int linkCount,
                 bool isPreformatted) {
    if (input.substr(0, 3) == "```") {
        FormatSwitch* fs = new FormatSwitch{input};
        return fs;
    }

    if (!isPreformatted) {
        if (input.substr(0, 2) == "=>") {
            Link* ln = new Link{input, prior, linkCount};
            return ln;
        }
        if ((input.substr(0, 1) == "#") || (input.substr(0, 2) == "##") ||
            (input.substr(0, 3) == "###")) {
            Heading* hd = new Heading{input};
            return hd;
        }

        if (input.substr(0, 1) == ">") {
            Quote* qt = new Quote{input};
            return qt;
        }
        if (input.substr(0, 2) == "* ") {
            ListItem* li = new ListItem{input};
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

    for (std::string::const_iterator i = value.begin(), n = value.end(); i != n;
         ++i) {
        std::string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char)c);
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
        "* (q|C-c) -> quit\n"
        "* b -> back a page\n"
        "* f -> forward a page\n"
        "* o -> show url entry / link selection\n"
        "* C-d -> move half a page down\n"
        "* C-u -> move half a page up\n"
        "* g -> go to the top of the page\n"
        "* G -> go to the bottom of the page\n"
        "* d -> download current page\n"
        "* e -> open current page in your preferred text editor\n"
        "* (r | C-r) -> refresh page\n";
    return st;
}

int u8len(unsigned char c) {
    if (c < 0x80) {
        return 1;
    }
    if ((c >> 5) == 0x6) {
        return 2;
    }
    if ((c >> 4) == 0xE) {
        return 3;
    }
    if ((c >> 3) == 0x1E) {
        return 4;
    }
    return 1;
}

int u8width(const std::string& s, std::size_t i, int len) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    if (len == 1 && c < 0x80) {
        return (c >= 0x20 && c != 0x7F) ? 1 : 0;
    }

    std::mbstate_t st{};
    wchar_t wc;
    if (std::mbrtowc(&wc, s.data() + i, len, &st) != (std::size_t)len) {
        return 1;
    }
    int w = wcwidth(wc);
    if (w < 0) {
        return 0;
    }
    return w;
}

// WIDTH IS INCLUSIVE
// WE ASSUME NO WIDER CHARS (E.G. replace tabs with spaces.)

std::vector<std::pair<std::string, TextRender>> breakLines(
    std::vector<std::pair<std::string, TextRender>>& strLs, int width,
    int cols) {
    std::vector<std::pair<std::string, TextRender>> res{};

    int leftPadAmount = std::max(0, (cols - width) / 2);
    std::string leftPadStr(leftPadAmount, ' ');

    if (width <= 0) {
        return res;
    }

    res.reserve(strLs.size());

    for (std::size_t i = 0; i < strLs.size(); ++i) {
        const std::string& cstr = strLs[i].first;

        if (!strLs[i].second.shouldFold) {
            int limit = cols - leftPadAmount;
            int w = 0;
            std::size_t end = 0;
            while (end < cstr.size()) {
                int len = std::min(u8len(cstr[end]), (int)(cstr.size() - end));
                int cw = u8width(cstr, end, len);
                if (w + cw > limit) {
                    break;
                }
                w += cw;
                end += len;
            }
            std::string rs =
                cstr.substr(0, end);  // otherwise there's some funkiness at the
                                      // end due to how ncurses renders stuff.
            res.push_back(std::pair<std::string, TextRender>{leftPadStr + rs,
                                                             strLs[i].second});
            continue;
        }

        std::string current = "";
        int curWidth = 0;
        int lastSpace = -1;
        int lastSpaceWidth = 0;

        for (int x = 0; x < (int)cstr.size();) {
            if (cstr[x] == '\n') {
                res.push_back(std::pair<std::string, TextRender>{
                    leftPadStr + current, strLs[i].second});
                current = "";
                curWidth = 0;
                lastSpace = -1;
                ++x;
                continue;
            }

            int len = std::min(u8len(cstr[x]), (int)cstr.size() - x);
            int w = u8width(cstr, x, len);

            if (w > 0 && curWidth > 0 && curWidth + w > width) {
                std::string toPush = current;

                if (lastSpace != -1) {
                    toPush = current.substr(0, lastSpace + 1);
                    current = current.substr(lastSpace + 1);
                    curWidth = curWidth - lastSpaceWidth;
                } else {
                    current = "";
                    curWidth = 0;
                }

                res.push_back(std::pair<std::string, TextRender>{
                    leftPadStr + toPush, strLs[i].second});
                lastSpace = -1;
            }

            current.append(cstr, x, len);
            curWidth += w;
            if (cstr[x] == ' ') {
                lastSpace = current.size() - 1;
                lastSpaceWidth = curWidth;
            }
            x += len;
        }
        if (current.size() > 0) {
            res.push_back(std::pair<std::string, TextRender>{
                leftPadStr + current, strLs[i].second});
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
    base =
        base.substr(base.find(':') + 1);  // works for file:/// and gemini:///

    while (base.size() > 0 && base[0] == '/') {
        base = base.substr(1);
    }

    std::string cleaned =
        std::regex_replace(base, std::regex("[^[:alnum:]._-]"), "_");
    return cleaned;
}

std::filesystem::path getHome() {
    std::string home = std::getenv("HOME");
    if (home == "") {
        throw std::runtime_error{"$HOME not set."};
    }
    if (!std::filesystem::exists(home)) {
        throw std::runtime_error{"$HOME directory doesn't exist..."};
    }

    return home;
}

Destination handleDestinationResolution(std::string destination, bool isCli) {
    Destination ret{};

    if (destination == "") {
        ret.t = NO_DESTINATION;
        return ret;
    }

    if (isCli) {
        if (std::filesystem::exists(destination)) {
            std::string path = "file://" +
                               std::filesystem::current_path().string() + "/" +
                               destination;

            if (destination.find('/') == std::size_t(0)) {
                path = "file://" + destination;
            }

            ret.destination = path;
            ret.t = STRING_DESTINATION;
            return ret;

        } else {
            std::string inputString = destination;
            if (inputString.find("gemini://") == 0) {
                ret.destination = destination;
                ret.t = STRING_DESTINATION;
                return ret;
            } else if (inputString.find(':') == std::string::npos) {
                ret.destination = std::string{"gemini://"} + destination;
                ret.t = STRING_DESTINATION;
                return ret;

            } else {
                ret.destination = destination;
                ret.t = STRING_DESTINATION;
                return ret;
            }
        }
    } else {
        try {
            std::size_t pos = 0;
            int dest = std::stoi(destination, &pos);
            if (pos == destination.size() && dest >= 0) {
                ret.linkNumber = dest;
                ret.t = NUMBER_DESTINATION;
                return ret;
            } else {
                throw std::invalid_argument("Unable to convert fully");
            }
        } catch (...) {
            if (destination.find(' ') == std::string::npos &&
                (destination.rfind("localhost:", 0) == 0 ||
                 destination.rfind("localhost/", 0) == 0 ||
                 destination == "localhost")) {
                destination = "gemini://" + destination;
            } else if (destination.find("://") == std::string::npos ||
                       destination.find(' ') != std::string::npos) {
                if (destination.find('.') != std::string::npos &&
                    destination.find(' ') == std::string::npos) {
                    destination = "gemini://" + destination;
                } else {
                    destination =
                        DEFAULT_SEARCH_ENGINE + urlEncode(destination);
                }
            }

            ret.destination = destination;
            ret.t = STRING_DESTINATION;
            return ret;
        }
    }
    throw std::logic_error("Unexpected input.");
}

void sanitizeCharactersToDraw(
    std::vector<std::pair<std::string, TextRender>>& strLs) {
    for (std::size_t i = 0; i < strLs.size(); ++i) {
        std::string& s = strLs[i].first;
        std::string out;
        out.reserve(s.size());

        for (int c : s)
            if (c == '\r' || c == '\v' || c == '\b' || c == '\f' || c == '\a' ||
                c == '\0') {
                continue;
            } else if (c == '\t') {
                out += "    ";  // \t is a larger character and fucks with
                                // breaklines.
            } else {
                out += c;
            }

        strLs[i].first = out;
    }
    return;
}

bool isSendableIfGeminiUrl(const uri& u) {
    return u.get_scheme() != "gemini" || u.to_string().size() <= 1024;
}

std::string getEditor() {
    const char* editor = std::getenv("EDITOR");
    if (editor) {
        return std::string{editor};
    } else {
        // if editor isn't set, default to nano
        return "nano";
    }
}
