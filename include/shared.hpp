#pragma once

#include <string>

const std::string DEFAULT_SEARCH_ENGINE = "gemini://tlgs.one/search?";

const int SITE_CACHE_LIMIT = 10;    // number of links to prefetch per page
const int THREAD_NUM = 4;           // thread count in thread pool for pre-fetching
const int maxWidth = 80;            // max text width (left and right will be padded if COLS > maxWidth)
const int CACHE_SIZE = 500;         // max number of elements in history cache and prefetch cache (500 for both)
const int COLOR_LINK = 159;         // color used for links
const int COLOR_PREFORMATTED = 201; // color used for preformatted text regions
