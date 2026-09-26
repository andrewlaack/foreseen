include config.mk

debug:
	${DCOMMAND_P} src/main.cpp ${DCOMMAND_S} -o gem-browser.out

build:
	${COMMAND_P} src/main.cpp ${COMMAND_S} -o gem-browser.out

install: build
	cp gem-browser.out ${PREFIX}/bin/gem-browser

clean:
	echo "Not implemented"

browser-test:
	${TCOMMAND_P} tests/browser-test.cpp ${TCOMMAND_S} -o test.out
	./test.out
	rm test.out

fetch-test:
	${TCOMMAND_P} tests/fetch-test.cpp ${TCOMMAND_S} -o test.out
	./test.out
	rm test.out

pure-test:
	${TCOMMAND_P} tests/pure-test.cpp ${TCOMMAND_S} -o test.out
	./test.out
	rm test.out


test: pure-test browser-test fetch-test
