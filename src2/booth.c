#include <stdlib.h>
#include "generate.h"

/*---------------------------------------------------------------------------*/

/* Point d'entrée appelé par main */
void mult_booth(mpz_t M);

/* Alloue le tableau bits et y écrit le développement binaire de M */
/* Renvoie le nombre de bits du développement                      */
int M_to_bit_array(mpz_t M,char **bits);

/* convertit l'entier binaire réprésenté par la chaine 'bits' */
/* en codage de Booth modifié */
int M_to_Booth(char *bits, int num_bits, int **codage);

/* renvoie un chiffre du codage */
int booth_code(char b0, char b1, char b2);

/* Génère la fonction de multiplication par M */
void generate_mult(int *booth_code, int len_code);


/*---------------------------------------------------------------------------*/

void (*mult_by_m)(mpz_t M) = &mult_booth;
const char *algo = "de Booth modifié";
char *FilePath = "MultBooth.c";

/*---------------------------------------------------------------------------*/
/* Point d'entrée appelé par main                                            */
/*---------------------------------------------------------------------------*/
void mult_booth(mpz_t M)
{
  FILE *File;
  
  /* Si M = 0 */
  if(mpz_cmp_ui(M,0)==0) 
    add_operation_int(SETUI,label_var(1),NULL,0);   
  /* Si M = 1 */
  else if(mpz_cmp_ui(M,1)==0) 
    add_operation_str(SET,label_var(1),NULL,label_var(0));   
  else
  {
    char *bits=NULL;
    int *booth_code;
    int num_bits;
    int len_code;

    /* On récupère l'écriture binaire de M et le nombre de bits */
    num_bits = M_to_bit_array(M, &bits);

    /* on calcule le codage de booth de M */
    len_code = M_to_Booth(bits, num_bits, &booth_code);
  
    generate_mult(booth_code, len_code);  
  }
  
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
/* convertit l'entier binaire réprésenté par la chaine 'bits'                */
/* en codage de Booth modifié                                                */
/*---------------------------------------------------------------------------*/
int M_to_Booth(char *bits, int num_bits, int **codage)
{
  int i;  
  int i0,i1,i2;    
  int m0,m1,m2;  
  int len = (num_bits/2 + 1)*2;
  *codage = malloc(sizeof(int) * len);

  for(i=0;i<len;i+=2)
  {
    i0 = num_bits-1-i+1;
    i1 = num_bits-1-i;
    i2 = num_bits-1-i-1;        
    m0 = (i0<0 || num_bits<=i0)?'0':bits[i0];
    m1 = (i1<0 || num_bits<=i1)?'0':bits[i1];
    m2 = (i2<0 || num_bits<=i2)?'0':bits[i2];
    
    switch(booth_code(m0,m1,m2))
    {
      case 0:
      (*codage)[len-i-1] = 0;
      (*codage)[len-i-2] = 0;      
      break;
      case 1:
      (*codage)[len-i-1] = 1;
      (*codage)[len-i-2] = 0;      
      break;
      case -1:
      (*codage)[len-i-1] = -1;
      (*codage)[len-i-2] = 0;      
      break;
      case 2:
      (*codage)[len-i-1] = 0;
      (*codage)[len-i-2] = 1;      
      break;
      case -2:
      (*codage)[len-i-1] = 0;
      (*codage)[len-i-2] = -1;      
      break;
    }
  }
  return len;
}

/*---------------------------------------------------------------------------*/
/* renvoie un chiffre du codage                                              */
/*---------------------------------------------------------------------------*/
int booth_code(char b0, char b1, char b2)
{
  return (b0=='0'?0:1)+(b1=='0'?0:1)-2*(b2=='0'?0:1);
}

/*---------------------------------------------------------------------------*/
/* Génère la fonction de multiplication par M                                */
/*---------------------------------------------------------------------------*/
void generate_mult(int *booth_code, int len_code)
{
  int first_i,last_i,i;
  char *x   = label_var(0);
  char *res = label_var(1);

  /* on cherche le premier bit de poids fort non nul */
  i=0;  
  while(i<len_code && booth_code[i]==0)
    i++;
  first_i=i;  
  last_i=i;
  i++;
  
  while(i<len_code)
  {
    /* on cherche le prochain 1 */
    while(i<len_code && booth_code[i]==0)
      i++;

    /* on quitte la boucle */  
    if(i==len_code) break;

    /* on décale de i-last_i */
    add_operation_int(SHF,res,last_i==first_i?x:res,i-last_i);     

    /* on ajoute 'x' à 'res' */      
    if(booth_code[i]<0)
      add_operation_str(SUB,res,res,x);
    else      
      add_operation_str(ADD,res,res,x);      
    
    last_i=i;    
    i++;      
  }

  /* en fin de génération, il ne faut pas oublier le dernier décalage */
  /* qui peut ne pas avoir été fait si l'écriture binaire de M fini par des 0 */
  if(i-last_i-1 != 0)
    /* on décale de i-last_i-1 */
    add_operation_int(SHF,res,last_i==first_i?x:res,i-last_i-1);
}
