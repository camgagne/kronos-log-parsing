CC = g++
CFLAGS = --std=c++20 -Wall -Werror -pedantic -g
LIB = -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system -lboost_unit_test_framework

all: ps7

ps7: main.o
	$(CC) $(CFLAGS) -o ps7 main.o $(LIB)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

clean:
	rm -f ps7 main.o

lint:
	cpplint *.cpp *.hpp
