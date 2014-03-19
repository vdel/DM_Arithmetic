#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "generate.h"

/* Ces variables sont mise à jour par l'algorithme utilisé */
/* Fonction d'appel */
extern void (*mult_by_m)(mpz_t M);
/* Nom de l'algo */
extern const char *algo;
/* Fichier de sortie */
extern char *FilePath;

int main(int argc, char** argv)
{
  int i;
  int num_ok = 0;
  int file = 0;
  mpz_t M;
  mpz_init(M); 
  
  while((i = getopt(argc, argv, "M:f:h:")) > 0)
    switch(i)
    {
      case 'M':
      if(mpz_set_str(M,optarg,0)==-1)
      {
        printf("L'entier indiqué n'est pas valide.\n");
        printf("La syntaxe est './Algo -M x' où 'Algo' est le programme et 'x' est la constante multiplicative.\n");    
        mpz_clear(M); 
        return 1;    
      }
      else
        num_ok = 1;   
      break;
      case 'f':
      if(file) free(FilePath);
      FilePath=malloc(sizeof(char)*(strlen(optarg)+1));
      strcpy(FilePath,optarg);
      file=1;
      break;
      case 'h':
      default:
      printf("La syntaxe est la suivante: ./Algo -M 'constante' -f 'fichier de sortie'\n");
      mpz_clear(M);       
      return 1;
    }
  if(!num_ok)
  {
    printf("La syntaxe est la suivante: ./Algo -M 'constante' -f 'fichier de sortie'\n");
    mpz_clear(M);     
    return 1;  
  }
  
  /* on initialise les données */
  init_data_generate("res","x");
    
  /* on fabrique le code pour la multiplication par M */
  (*mult_by_m)(M);
    
  /* on fait le ménage */
  clear_data_generate();
  mpz_clear(M);
  return 0;
}

