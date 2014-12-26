all: lalrparser
lalrparser: lalrparser.h lalrparser.cpp
	g++ -std=c++11 -o lalrparser lalrparser.cpp -O3
test: lalrparser.h rules.h test.cpp
	g++ -std=c++11 -o test test.cpp -O3
clean:
	rm -f lalrparser
