run: compile 
	./greedyRandStar 
	python3 plot.py

compile: greedyRandStar.c
	gcc -o greedyRandStar -O2 -Wall greedyRandStar.c 

valgrind:
	gcc -o test -g -Wall greedyRandStar.c 
	valgrind ./greedyRandStar
	
clean:
	rm -f greedyRandStar
	rm -f *.data
	rm -f *.pdf
	rm -f plot.py


