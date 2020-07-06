std=c++11
cxx=g++
objects=main.o utils.o

main: $(objects)
	$(cxx) -std=$(std) $^ -o $@

main.o: main.cpp
	$(cxx) -std=$(std) -c $< -o $@

utils.o: utils.cpp utils.hpp
	$(cxx) -std=$(std) -c $< -o $@

.PHONY: clean
clean:
	rm *.o *.out