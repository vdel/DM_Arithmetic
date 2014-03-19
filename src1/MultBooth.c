#include <stdio.h>
#include <gmp.h>

void multbyM(mpz_t res, mpz_t x);

int main(int argc, char** argv)
{
  mpz_t x;
  mpz_t res;

  /* On vérifie que l'utilisateur a entré un nombre */
  if(argc!=2)
  {
    printf("La syntaxe est './generate_AlgoNaif x' où x est le nombre à multiplier.\n");
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

  multbyM(res,x);
  mpz_out_str(stdout,10,res);
  printf("\n");

  mpz_clear(x);
  mpz_clear(res);
  return 0;
}

void multbyM(mpz_t res, mpz_t x)  /* Multiplication par 984685465 */
{
  mpz_t _2x;
  mpz_init(_2x);
  mpz_mul_ui(_2x, x, 2);

  mpz_mul_ui(res, x, 1);      /* res  = 1x */
  mpz_mul_2exp(res, res, 4);  /* res  = 16x */
  mpz_sub(res, res, x   );    /* res  = 15x */
  mpz_mul_2exp(res, res, 2);  /* res  = 60x */
  mpz_sub(res, res, x   );    /* res  = 59x */
  mpz_mul_2exp(res, res, 2);  /* res  = 236x */
  mpz_sub(res, res, x   );    /* res  = 235x */
  mpz_mul_2exp(res, res, 2);  /* res  = 940x */
  mpz_sub(res, res, x   );    /* res  = 939x */
  mpz_mul_2exp(res, res, 4);  /* res  = 15024x */
  mpz_add(res, res, x   );    /* res  = 15025x */
  mpz_mul_2exp(res, res, 4);  /* res  = 240400x */
  mpz_add(res, res, _2x );    /* res  = 240402x */
  mpz_mul_2exp(res, res, 2);  /* res  = 961608x */
  mpz_sub(res, res, x   );    /* res  = 961607x */
  mpz_mul_2exp(res, res, 4);  /* res  = 15385712x */
  mpz_sub(res, res, _2x );    /* res  = 15385710x */
  mpz_mul_2exp(res, res, 2);  /* res  = 61542840x */
  mpz_add(res, res, _2x );    /* res  = 61542842x */
  mpz_mul_2exp(res, res, 2);  /* res  = 246171368x */
  mpz_sub(res, res, _2x );    /* res  = 246171366x */
  mpz_mul_2exp(res, res, 2);  /* res  = 984685464x */
  mpz_add(res, res, x   );    /* res  = 984685465x */

  mpz_clear(_2x);
}
