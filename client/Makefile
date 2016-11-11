all: myfrm

myfrm: myfrm.cpp util.h
	g++ -std=c++11 myfrm.cpp util.h -Wall -lmhash -lssl -o myfrm

.PHONY: clean

clean:
	-@rm -rf myfrm 2>/dev/null
