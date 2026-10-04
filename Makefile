include config.mk

# this must be started and running before running tests that do network connections
server:
	python3 -m jetforce --dir tests

debug:
	${DCOMMAND_P} src/main.cpp ${DCOMMAND_S} -o foreseen.out

build:
	${COMMAND_P} src/main.cpp ${COMMAND_S} -o foreseen.out

install: build
	cp foreseen.out ${PREFIX}/bin/foreseen
	mkdir -p ${MANPREFIX}/man1
	cp docs/foreseen.1 ${MANPREFIX}/man1/foreseen.1
	chmod 644 ${MANPREFIX}/man1/foreseen.1

clean:
	rm -rf test*.out foreseen.out
	rm -rf bench*.out
	rm -rf *.gcda *.gcno
	rm -rf coverage.info
	rm -rf coverage-html
	rm -rf *.svg
	rm -rf *.data
	rm -rf *.data.old
	rm -rf *.folded
	rm -rf *.info

browser-test:
	${TCOMMAND_P} tests/browser-test.cpp ${TCOMMAND_S} -o test1.out
	./test1.out
	rm test1.out

fetch-test:
	${TCOMMAND_P} tests/fetch-test.cpp ${TCOMMAND_S} -o test2.out
	./test2.out
	rm test2.out

puppet-test:
	${TCOMMAND_P} tests/puppet-test.cpp ${TCOMMAND_S} -o test6.out
	./test6.out
	rm test6.out

pure-test:
	${TCOMMAND_P} tests/pure-test.cpp ${TCOMMAND_S} -o test3.out
	./test3.out
	rm test3.out

# NOTE: This is purposely not included in the make test command
mem-leak-test:
	${STCOMMAND_P} tests/mem-leak-test.cpp ${COMMAND_S} -o test4.out
	valgrind --tool=memcheck ./test4.out
	rm test4.out

benchmark:
	${BCOMMAND_P} benchmarking/render-bench.cpp ${BCOMMAND_S} -o bench1.out
	perf record -g ./bench1.out
	rm bench1.out

test: pure-test browser-test fetch-test puppet-test
	lcov --capture --directory . --output-file coverage.info \
		--no-external \
		--rc geninfo_unexecuted_blocks=1 \
		--ignore-errors inconsistent,inconsistent
	genhtml coverage.info --output-directory coverage-html \
		--ignore-errors inconsistent,inconsistent
	rm -rf *.gcda *.gcno
	rm -rf coverage.info

.PHONY: test pure-test browser-test fetch-test puppet-test benchmark mem-leak-test clean install build debug server
