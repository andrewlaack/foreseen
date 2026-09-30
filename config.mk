VERSION = 10.0.0

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

LIBS = -lssl -lcrypto -lncursesw
TLIBS = -lCatch2Main -lCatch2 -lrapidcheck ${LIBS}

CC = g++

IFLAGS = -Ofast -std=c++23
DFLAGS = -O0 -fsanitize=address,undefined -g -std=c++23 -Wpedantic -Wall -Wextra -Wno-deprecated-declarations -D DEBUG_MODE

CTFLAGS = -O0 -fsanitize=undefined -g -std=c++23 -Wpedantic -Wall -Wextra -Wno-deprecated-declarations -fprofile-arcs -ftest-coverage -fPIC -D DEBUG_MODE

BASE_FILES = src/identity-manager.cpp src/quote.cpp src/list-item.cpp src/preformatted.cpp src/format-switch.cpp src/cache.cpp src/link.cpp  src/plaintext.cpp src/site.cpp src/utils.cpp src/gemini-client.cpp src/browser.cpp src/heading.cpp src/render.cpp

COMMAND_P = ${CC} ${CFLAGS} -D NDEBUG
COMMAND_S = ${BASE_FILES} ${LIBS}

TCOMMAND_P = ${CC} ${CTFLAGS} 
TCOMMAND_S = ${BASE_FILES} ${TLIBS}

DCOMMAND_P = ${CC} ${DFLAGS}
DCOMMAND_S = ${BASE_FILES} ${LIBS}

STCOMMAND_P = ${CC} ${CFLAGS} -D DEBUG_MODE
