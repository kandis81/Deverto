
cpp = g++
cflags = -ggdb -std=c++11

all:
	make fpower
	make qtest

fpower:
	$(cpp) $(cflags) -o fifthpowers 2-fifth_powers.cpp

qtest:
	$(cpp) $(cflags) -o queuetest 4-queue.cpp -lpthread

