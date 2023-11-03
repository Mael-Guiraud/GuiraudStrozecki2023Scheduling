OS := $(shell uname)
ifeq ($(OS),Darwin)
  CC = gcc-7
else
  CC = gcc
endif


all: compil run plot


compil:
	$(CC) longsmessages.c -fopenmp -o longsmess -O3 -g

run: longsmess
	./longsmess

plot: plot.py
	python3 plot.py