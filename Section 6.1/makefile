OS := $(shell uname)
ifeq ($(OS),Darwin)
  CC = gcc-7
else
  CC = gcc
endif


all: compil run gnuplot


compil:
	$(CC) longsmessages.c -fopenmp -o longsmess -O3 -g

run: longsmess
	./longsmess

gnuplot: success.gplt
	gnuplot success.gplt