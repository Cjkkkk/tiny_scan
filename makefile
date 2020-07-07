CC=g++
std=c++11
src=./src
build=./build
test=./test
include=./src

scan: main.o utils.o task.o
	$(CC) -std=$(std) $(build)/main.o $(build)/utils.o $(build)/task.o -o $(build)/$@

test: test.o utils.o task.o
	$(CC) -std=$(std) $(build)/test.o $(build)/utils.o $(build)/task.o -o $(build)/$@

main.o: $(src)/main.cpp
	$(CC) -std=$(std) -c $< -o $(build)/$@ -I $(include)

utils.o: $(src)/utils.cpp $(src)/utils.hpp
	$(CC) -std=$(std) -c $< -o $(build)/$@ -I $(include)

task.o: $(src)/task.cpp $(src)/task.hpp
	$(CC) -std=$(std) -c $< -o $(build)/$@ -I $(include)

test.o: $(test)/test.cpp
	$(CC) -std=$(std) -c $< -o $(build)/$@ -I $(include)

.PHONY: clean
clean:
	rm $(build)/* mr_*