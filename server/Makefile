all: myfrmd

myfrmd: myfrmd.cpp util.h
	g++ -std=c++11 myfrmd.cpp util.h -Wall -lmhash -lssl -o myfrmd

.PHONY: clean

clean:
	-@rm -rf myfrmd 2>/dev/null
