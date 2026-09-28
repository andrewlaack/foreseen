include config.mk

debug:
	${DCOMMAND_P} src/main.cpp ${DCOMMAND_S} -o gem-browser.out

build:
	${COMMAND_P} src/main.cpp ${COMMAND_S} -o gem-browser.out

install: build
	cp gem-browser.out ${PREFIX}/bin/gem-browser

clean:
	rm -rf test*.out gem-browser.out

browser-test:
	${TCOMMAND_P} tests/browser-test.cpp ${TCOMMAND_S} -o test1.out
	./test1.out
	rm test1.out

fetch-test:
	${TCOMMAND_P} tests/fetch-test.cpp ${TCOMMAND_S} -o test2.out
	./test2.out
	rm test2.out

pure-test:
	${TCOMMAND_P} tests/pure-test.cpp ${TCOMMAND_S} -o test3.out
	./test3.out
	rm test3.out

# NOTE: This is purposely not included in the make test command
mem-leak-test:
	${STCOMMAND_P} tests/mem-leak-test.cpp ${COMMAND_S} -o test4.out
	valgrind --tool=memcheck ./test4.out
	rm test4.out

crash-test:
	${STCOMMAND_P} tests/mem-leak-test.cpp ${COMMAND_S} -o test5.out
	./test5.out
	rm test5.out

test: pure-test browser-test fetch-test
