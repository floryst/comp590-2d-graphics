CXX = g++ 

all: test0 test1 test2 test3

test0 : test0.cpp GContext0.cpp
	$(CXX) -g test0.cpp GContext0.cpp -o test0

test1 : test1.cpp GContext0.cpp
	$(CXX) -g test1.cpp GContext0.cpp -o test1

test2 : test2.cpp GContext0.cpp
	$(CXX) -g test2.cpp GContext0.cpp -o test2

test3 : test3.cpp GContext0.cpp
	$(CXX) -g test3.cpp GContext0.cpp -o test3

clean:
	rm test0
	rm test1
	rm test2
	rm test3
