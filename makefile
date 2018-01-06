
cpp = g++

all:
	rm -f $@
	make fifthpowers

fifthpowers:
	$(cpp) -o $@ 2-fifth_powers.cpp
