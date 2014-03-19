#include <stdio.h>
#include <gmp.h>

extern void (*MultByM)(mpz_t /*res*/, mpz_t /*x*/);

int main(int argc, char** argv)
{
  mpz_t x;
  mpz_t res;

  /* On vérifie que l'utilisateur a entré un nombre */
  if(argc!=2)
  {
    printf("La syntaxe est './exec x' où exec est le programme et x est le nombre à multiplier.\n");
    return 1;
  }

  mpz_init(x);
  /* On vérifie que le nombre est valide */
  if(mpz_set_str(x,argv[1],10)==-1)
  {
    printf("L'entier indiqué n'est pas valide.\n");
    mpz_clear(x);
    return 1;
  }

  mpz_init(res);

  MultByM(res,x);
  mpz_out_str(stdout,10,res);
  printf("\n");

  mpz_clear(x);
  mpz_clear(res);
  return 0;
}
