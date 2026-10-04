#pragma once

#include <cstdint>
#include <string>

extern std::string DEFAULT_SEARCH_ENGINE;   // default search option when search is inferred
extern int SITE_CACHE_LIMIT;                // number of links to prefetch per page
extern int THREAD_NUM;                      // thread count in thread pool for pre-fetching
extern int MAX_WIDTH;                       // max text width
extern int CACHE_SIZE;                      // max elements in history cache and max in prefetch cache
extern uint8_t COLOR_LINK;                  // color used for links
extern uint8_t COLOR_PREFORMATTED;          // color used for preformatted text regions
extern uint8_t COLOR_QUOTE;                 // color used for quote lines

extern uint8_t COLOR_H3;                    // color used for H3 lines
extern uint8_t COLOR_H2;                    // color used for H2 lines
extern uint8_t COLOR_H1;                    // color used for H1 lines

extern uint8_t COLOR_LIST_ITEM;             // color used for list items
extern uint8_t COLOR_FORMAT_SWITCH;         // color used for format switch lines
extern uint8_t COLOR_PLAINTEXT;             // color used for plaintext lines

 // We guarantee responses can be at least the size limit, and at most 4096 additional bytes
extern int RESPONSE_SIZE_LIMIT_MB;

// since the xdg-open stuff is in a seperate process nothing will go to stdout
// even in the echo case.
#ifdef DEBUG_MODE
    const char openUnknownScheme[] = "echo";
#else
    const char openUnknownScheme[] = "xdg-open";
#endif
