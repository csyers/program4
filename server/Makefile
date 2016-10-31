all: myfrmd

myfrmd: myfrmd.c
	gcc myfrmd.c -Wall -lmhash -lssl -o myfrmd

.PHONY: clean

clean:
	-@rm -rf myfrmd 2>/dev/null
