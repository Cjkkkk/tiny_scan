CC=g++
std=c++11
src=./src
build=./build

scan: main.o utils.o
	$(CC) -std=$(std) $(build)/main.o $(build)/utils.o -o $(build)/$@

main.o: $(src)/main.cpp
	$(CC) -std=$(std) -c $< -o $(build)/$@

utils.o: $(src)/utils.cpp $(src)/utils.hpp
	$(CC) -std=$(std) -c $< -o $(build)/$@

.PHONY: clean
clean:
	rm $(build)/* mr_*