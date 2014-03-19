CC=gcc
CFLAGS=-W -Wall -pedantic
LDFLAGS=-lgmp

all:
	make exec
	echo "\n\n!!!!!!!!!!!!!!!!!\nFaire 'make exec' pour compiler AlgoNaif et AlgoBooth.\nFaire 'make naif' pour compiler MultNaif (programme généré par AlgoNaif).\nFaire 'make booth' pour compiler MultBooth (programme généré par AlgoBooth).\nFaire 'make gen' pour compiler MultNaif et MultBooth.\n!!!!!!!!!!!!!!!!!\n\n"
	make naif
	make booth	
  
exec: AlgoNaif.o AlgoBooth.o
	$(CC) -o AlgoNaif AlgoNaif.o $(LDFLAGS)
	$(CC) -o AlgoBooth AlgoBooth.o $(LDFLAGS)	

AlgoNaif.o: AlgoNaif.c
	$(CC) -o AlgoNaif.o -c AlgoNaif.c $(CFLAGS)

AlgoBooth.o: AlgoBooth.c
	$(CC) -o AlgoBooth.o -c AlgoBooth.c $(CFLAGS)
	
gen: naif booth
				
naif: MultNaif.o
	$(CC) -o MultNaif MultNaif.o $(LDFLAGS)	
	
booth: MultBooth.o
	$(CC) -o MultBooth MultBooth.o $(LDFLAGS)
	
MultNaif.o: MultNaif.c
	$(CC) -o MultNaif.o -c MultNaif.c $(CFLAGS)

MultBooth.o: MultBooth.c
	$(CC) -o MultBooth.o -c MultBooth.c $(CFLAGS)
	
clean:
	rm -f *.c~ *.h~ *.o AlgoNaif AlgoBooth MultNaif MultBooth MultNaif.c MultBooth.c makefile~ README~
