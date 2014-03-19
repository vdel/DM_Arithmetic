CC=gcc
CFLAGS=-W -Wall -pedantic
LDFLAGS=-lgmp -lm

all:
	make exec
	echo "\n\n!!!!!!!!!!!!!!!!!\nFaire 'make' (ou 'make exec') pour compiler les algorithmes demandés.\nFaire 'make gen' pour compiler les multiplications générées.\nPour compiler le programme de test associé à une multiplication spécifique seulement, faire:\n  - make naif\n  - make booth\n  - make bernstein\n  - make lefevre1\n  - make lefevre2\n!!!!!!!!!!!!!!!!!\n\n"	
  
exec: main.o generate.o naif.o booth.o bernstein.o lefevre1.o lefevre2.o
	$(CC) -o AlgoNaif main.o generate.o naif.o $(LDFLAGS)
	$(CC) -o AlgoBooth main.o generate.o booth.o $(LDFLAGS)
	$(CC) -o AlgoBernstein main.o generate.o bernstein.o $(LDFLAGS)	
	$(CC) -o AlgoLefevre1 main.o generate.o common_lefevre.o lefevre1.o $(LDFLAGS)	
	$(CC) -o AlgoLefevre2 main.o generate.o common_lefevre.o lefevre2.o $(LDFLAGS)		
	
main.o: main.c
	$(CC) -o main.o -c main.c $(CFLAGS)
	
generate.o: generate.c generate.h
	$(CC) -o generate.o -c generate.c $(CFLAGS)

naif.o: naif.c
	$(CC) -o naif.o -c naif.c $(CFLAGS)
	
booth.o: booth.c
	$(CC) -o booth.o -c booth.c $(CFLAGS)
	
bernstein.o: bernstein.c
	$(CC) -o bernstein.o -c bernstein.c $(CFLAGS)

lefevre1.o: lefevre1.c common_lefevre.o
	$(CC) -o lefevre1.o -c lefevre1.c $(CFLAGS)
	
lefevre2.o: lefevre2.c common_lefevre.o
	$(CC) -o lefevre2.o -c lefevre2.c $(CFLAGS)	

common_lefevre.o: common_lefevre.c common_lefevre.h
	$(CC) -o common_lefevre.o -c common_lefevre.c $(CFLAGS)


gen: naif booth bernstein lefevre1 lefevre2
				
test.o: test.c
	$(CC) -o test.o -c test.c $(CFLAGS)
				
naif: MultNaif.c test.o
	$(CC) -o MultNaif.o -c MultNaif.c $(CFLAGS)
	$(CC) -o MultNaif test.o MultNaif.o $(LDFLAGS)	
	
booth: MultBooth.c test.o
	$(CC) -o MultBooth.o -c MultBooth.c $(CFLAGS)
	$(CC) -o MultBooth test.o MultBooth.o $(LDFLAGS)

bernstein: MultBernstein.c test.o
	$(CC) -o MultBernstein.o -c MultBernstein.c $(CFLAGS)
	$(CC) -o MultBernstein test.o MultBernstein.o $(LDFLAGS)

lefevre1: MultLefevre1.c test.o
	$(CC) -o MultLefevre1.o -c MultLefevre1.c $(CFLAGS)
	$(CC) -o MultLefevre1 test.o MultLefevre1.o $(LDFLAGS)

lefevre2: MultLefevre2.c test.o
	$(CC) -o MultLefevre2.o -c MultLefevre2.c $(CFLAGS)
	$(CC) -o MultLefevre2 test.o MultLefevre2.o $(LDFLAGS)	
		
clean:
	rm -f *.c~ *.h~ *.o makefile~ README~
	rm -f AlgoNaif MultNaif MultNaif.c
	rm -f AlgoBooth MultBooth MultBooth.c
	rm -f AlgoBernstein MultBernstein MultBernstein.c
	rm -f AlgoLefevre1 MultLefevre1 MultLefevre1.c
	rm -f AlgoLefevre2 MultLefevre2 MultLefevre2.c
