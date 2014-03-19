#include <stdlib.h>
#include "generate.h"

/*---------------------------------------------------------------------------*/

/* Point d'entrée appelé par main */
void mult_naif(mpz_t M);

/* Alloue le tableau bits et y écrit le développement binaire de M */
/* Renvoie le nombre de bits du développement */
int M_to_bit_array(mpz_t M,char **bits);

/* Génère la fonction de multiplication par M */
void generate_mult(char *bits, int num_bits);


/*---------------------------------------------------------------------------*/

void (*mult_by_m)(mpz_t M) = &mult_naif;
const char *algo = "naïf";
char *FilePath = "MultNaif.c";

/*---------------------------------------------------------------------------*/
/* Point d'entrée appelé par main                                            */
/*---------------------------------------------------------------------------*/
void mult_naif(mpz_t M)
{
  FILE *File;
  char *bits=NULL;
  int num_bits;
  
  /* On récupère l'écriture binaire de M et le nombre de bits */
  num_bits = M_to_bit_array(M, &bits);

  /* Si M = 0 */
  if(mpz_cmp_ui(M,0)==0) 
    add_operation_int(SETUI,label_var(1),NULL,0);   
  /* Si M = 1 */
  else if(mpz_cmp_ui(M,1)==0) 
    add_operation_str(SET,label_var(1),NULL,label_var(0));   
  else
    generate_mult(bits, num_bits); 
  
  /* On génère le programme */
  File = fopen(FilePath,"wt");
  generate(File, M, algo);
  fclose(File); 
}

/*---------------------------------------------------------------------------*/
/* Alloue le tableau bits et y écrit le développement binaire de M           */
/* Renvoie le nombre de bits du développement                                */
/*---------------------------------------------------------------------------*/
int M_to_bit_array(mpz_t M, char **bits)
{
  *bits = mpz_get_str(*bits, 2, M);
  return strlen(*bits);
}

/*---------------------------------------------------------------------------*/
/* Génère la fonction de multiplication par M                                */
/*---------------------------------------------------------------------------*/
void generate_mult(char *bits, int num_bits)
{
  int last_i,i;
  char *x   = label_var(0);
  char *res = label_var(1);  

  last_i=0;
  i=1;  /* on multiplie par au moins 2 et le premier bits est non nul */
  while(i<num_bits)
  {
    /* on cherche le prochain 1 */
    while(i<num_bits && bits[i]=='0')
      i++;
    
    /* on quitte la boucle */ 
    if(i==num_bits) break;
        
    /* on décale de i-last_i et ajoute 'x' à 'res' */
    add_operation_int(SHF,res,last_i==0?x:res,i-last_i);    
    add_operation_str(ADD,res,res,x);                     

    last_i=i;
    i++;  
  }

  /* en fin de génération, il ne faut pas oublier le dernier décalage */
  /* qui peut ne pas avoir été fait si l'écriture binaire de M fini par des 0 */
  if(i-last_i-1 != 0)
    add_operation_int(SHF,res,res,i-last_i-1);
}
