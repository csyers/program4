all: myfrm

myfrm: myfrm.c
	gcc myfrm.c -Wall -lmhash -lssl -o myfrm

.PHONY: clean

clean:
	-@rm -rf myfrm 2>/dev/null
