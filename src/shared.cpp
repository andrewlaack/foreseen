#include "../include/shared.hpp"

// =============================================================================
// CLI OPTIONS
// =============================================================================

// These are the defaults for CLI options. When options are specified from the
// CLI they will supersede these values.

// When using the 'o' search menu, this will be infered as the base url of the
// search.

// TODO: Must this be url encoded?
std::string DEFAULT_SEARCH_ENGINE = "gemini://tlgs.one/search";

// This is the maximum number of links to prefetch per page.

int SITE_CACHE_LIMIT = 10;

// The prefetch cache and history cache may both have CACHE_SIZE sites within
// their respective caches at any given time.

// TODO: given this definition; check to make sure eviction happens prior to
// insertion to maintain size invariant at all times.

int CACHE_SIZE = 50;

// =============================================================================
// COLORS
// =============================================================================

// Colors vary by $TERM. See ncurses and terminfo docs for details. The default
// constants assume a 256-color terminal.

uint8_t COLOR_LINK = 159;
uint8_t COLOR_PREFORMATTED = 201;
uint8_t COLOR_QUOTE = 5;

uint8_t COLOR_LIST_ITEM = 15;
uint8_t COLOR_FORMAT_SWITCH = 7;
uint8_t COLOR_PLAINTEXT = 15;

uint8_t COLOR_H3 = 6;
uint8_t COLOR_H2 = 2;
uint8_t COLOR_H1 = 1;

// =============================================================================
// OTHER
// =============================================================================

// This limits the size of gemini protocol responses to at most
// RESPONSE_SIZE_LIMIT_MB + 4kb, and at least RESPONSE_SIZE_LIMIT_MB.

int RESPONSE_SIZE_LIMIT_MB = 5;

// This specifies the base command to execute to handle unknown schemes (e.g.
// http). The command will be executed along with the uri being followed as the
// first positional argument.

#ifdef DEBUG_MODE
char openUnknownScheme[] = "echo";
#else
char openUnknownScheme[] = "xdg-open";
#endif

// This is the thread pool size for prefetching. Each thread is given at most
// SITE_CACHE_LIMIT links to fetch when visiting a site. If all threads are in
// use and a site is visited, prefetching is skipped.

int THREAD_NUM = 4;

// This is the maximum width text may be before it is forcibly wrapped.

int MAX_WIDTH = 80;
