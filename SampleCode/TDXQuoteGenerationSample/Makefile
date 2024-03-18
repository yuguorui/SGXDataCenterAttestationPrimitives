main: main.c
	gcc main.c -ltdx_attest -o main

clean:
	rm -f main quote.dat report.dat ./dist/*.rpm

rpm:
	./dist/build.sh

.PHONY: clean
