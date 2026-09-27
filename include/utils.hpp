#pragma once
#include <optional>
#include <filesystem>
#include <string>
#include <vector>
#include "line.hpp"
#include "../vendor/uri.hpp"

struct TextRender {
    int color;
    bool isBold;
    bool shouldFold = true; // preformatted text doesn't need to be folded.
};

bool isPrefixed(std::string input, std::string prefix);
std::string readFileToString(std::string filePath);
bool isWhiteSpace(std::string& line, int idx);
std::string truncateAfter(std::string input, char truncate);
std::vector<std::string> stringToList(std::string input);
Line* lineToLine(std::string input, std::optional<uri> prior, int linkCount, bool isPreformatted);
std::string urlEncode(const std::string& value);
std::string getNewTab();
void openUrl(const std::string& url);
std::vector<std::pair<std::string, TextRender>> breakLines(std::vector<std::pair<std::string, TextRender>>& strLs, int width, int cols);

// This will over-write the existing file, if relevant. This will not create directories for you.
void writeStringToFile(std::string toWrite, std::string filePath);
std::string encodeAsFilename(uri link);
std::filesystem::path getHome();
std::optional<std::string> handleCLIDestination(std::string destination);
