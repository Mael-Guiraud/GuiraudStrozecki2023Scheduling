run: compile 
	./test
	gnuplot success.gplt

compile: greedyRandStar.c
	gcc -o test -O2 -Wall greedyRandStar.c 

valgrind: 
	gcc -o test -g -Wall greedyRandStar.c 
	valgrind ./test
	
clean:
	rm -f test


