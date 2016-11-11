all: myfrm

myfrm: myfrm.cpp
	g++ -std=c++11 myfrm.cpp -Wall -lmhash -lssl -o myfrm

.PHONY: clean

clean:
	-@rm -rf myfrm 2>/dev/null
