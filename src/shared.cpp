#include "../include/shared.hpp"

std::string DEFAULT_SEARCH_ENGINE = "gemini://tlgs.one/search";
int SITE_CACHE_LIMIT = 10;
int THREAD_NUM = 4;
int MAX_WIDTH = 80;
int CACHE_SIZE = 50;

uint8_t COLOR_LINK = 159;
uint8_t COLOR_PREFORMATTED = 201;
uint8_t COLOR_QUOTE = 5;

uint8_t COLOR_H3 = 6;
uint8_t COLOR_H2 = 2;
uint8_t COLOR_H1 = 1;

uint8_t COLOR_LIST_ITEM = 15;
uint8_t COLOR_FORMAT_SWITCH = 7;
uint8_t COLOR_PLAINTEXT = 15;

int RESPONSE_SIZE_LIMIT_MB = 5;     // This will truncate at the specified size.
