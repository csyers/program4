all: myfrmd

myfrmd: myfrmd.cpp
	g++ -std=c++11 myfrmd.cpp -Wall -lmhash -lssl -o myfrmd

.PHONY: clean

clean:
	-@rm -rf myfrmd 2>/dev/null
