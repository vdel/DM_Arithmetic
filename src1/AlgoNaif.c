#include <gmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Alloue le tableau bits et y écrit le développement binaire de M */
/* Renvoie le nombre de bits du développement */
int M_to_bit_array(mpz_t M,char **bits);

/* Génère l'en-tête du fichier */
void generate_header(FILE *File);

/* Génère la fonction de multiplication par M */
void generate_mult(FILE *File, char *bits, int num_bits, char *M);

/*---------------------------------------------------------------------------*/
int main(int argc, char** argv)
{
  mpz_t M;
  char *bits=NULL;
  int num_bits;
  FILE *File;
  
  /* On vérifie que l'utilisateur a entré un nombre */
  if(argc!=2)
  {
    printf("La syntaxe est './question1 M' où M est la constante multiplicative.\n");
    return 1;
  }
 
  mpz_init(M);
  /* On vérifie que le nombre est valide */
  if(mpz_set_str(M,argv[1],10)==-1)
  {
    printf("L'entier indiqué n'est pas valide.\n");
    mpz_clear(M);    
    return 1;    
  }
  
  /* On récupère l'écriture binaire de M et le nombre de bits */
  num_bits = M_to_bit_array(M, &bits);
  
  /* On génère le programme */
  File = fopen("MultNaif.c","wt");
  generate_header(File);
  generate_mult(File, bits, num_bits, argv[1]);  
  fclose(File);
  
  /* on fait le ménage */
  mpz_clear(M);
  return 0;
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
/* Génère l'en-tête du fichier                                               */
/*---------------------------------------------------------------------------*/
void generate_header(FILE *File)
{
  char header1[]="\
#include <stdio.h>\n\
#include <gmp.h>\n\
\n\
void multbyM(mpz_t res, mpz_t x);\n\
\n\
int main(int argc, char** argv)\n\
{\n\
  mpz_t x;\n\
  mpz_t res;\n\
\n\
  /* On vérifie que l'utilisateur a entré un nombre */\n\
  if(argc!=2)\n\
  {\n\
    printf(\"La syntaxe est './generate_AlgoNaif x' où x est le nombre à multiplier.\\n\");\n\
    return 1;\n\
  }\n\
\n\
  mpz_init(x);\n";
  char header2[]="\
  /* On vérifie que le nombre est valide */\n\
  if(mpz_set_str(x,argv[1],10)==-1)\n\
  {\n\
    printf(\"L'entier indiqué n'est pas valide.\\n\");\n\
    mpz_clear(x);\n\
    return 1;\n\
  }\n\
\n\
  mpz_init(res);\n\
\n\
  multbyM(res,x);\n\
  mpz_out_str(stdout,10,res);\n\
  printf(\"\\n\");\n\
\n\
  mpz_clear(x);\n\
  mpz_clear(res);\n\
  return 0;\n\
}\n\n";
  fwrite(header1, strlen(header1), 1, File);
  fwrite(header2, strlen(header2), 1, File);      
}

/*---------------------------------------------------------------------------*/
/* Génère la fonction de multiplication par M                                */
/*---------------------------------------------------------------------------*/
void generate_mult(FILE *File, char *bits, int num_bits, char *M)
{
  int last_i,i;
  char buffer[1000];
  char begin[] = "void multbyM(mpz_t res, mpz_t x)  /* Multiplication par %s */\n{\n";
  char first[] = "  mpz_set(res, x);            /* res = x */\n";
  char shift[] = "  mpz_mul_2exp(res, res, %d);  /* res = %sx */\n";
  char add[]   = "  mpz_add(res, res, x);       /* res = %sx */\n";  
  char end[]="}\n";
  char *progress_char;     /* pour afficher où on en est dans les commentaires */
  mpz_t progress,temp;  /* pour afficher où on en est dans les commentaires */

  mpz_init(temp);  
  mpz_init(progress);
  
  sprintf(buffer,begin,M);
  fwrite(buffer, strlen(buffer), 1, File);
  
  /* Le bits de gauche est forcément à 1 */
  /* on calcule la valeur actuelle de 'res' pour les commentaires */
  mpz_set_ui(progress, 1);
  progress_char = mpz_get_str(NULL, 10, progress);

  /* on initialise 'res' */      
  fwrite(first, strlen(first), 1, File);       
  free(progress_char);

  last_i=0;
  i=1;  
  while(i<num_bits)
  {
    /* on cherche le prochain 1 */
    while(i<num_bits && bits[i]=='0')
      i++;
    
    /* on quitte la boucle */ 
    if(i==num_bits) break;
      
    /* on calcule la valeur actuelle de 'res' pour les commentaires */
    mpz_mul_2exp(progress, progress, i-last_i);
    progress_char = mpz_get_str(NULL, 10, progress);
    
    /* on décale de i-last_i */
    sprintf(buffer,shift,i-last_i,progress_char);
    fwrite(buffer, strlen(buffer), 1, File);     
    free(progress_char);

    /* on calcule la valeur actuelle de 'res' pour les commentaires */       
    mpz_set_ui(temp, 1);
    mpz_add(progress, progress, temp);      
    progress_char = mpz_get_str(NULL, 10, progress);

    /* on ajoute 'x' à 'res' */      
    sprintf(buffer,add,progress_char);
    fwrite(buffer, strlen(buffer), 1, File);          
    free(progress_char);

    last_i=i;
    i++;  
  }

  /* en fin de génération, il ne faut pas oublier le dernier décalage */
  /* qui peut ne pas avoir été fait si l'écriture binaire de M fini par des 0 */
  if(i-last_i-1 != 0)
  {
    /* on calcule la valeur actuelle de 'res' pour les commentaires */
    mpz_mul_2exp(progress, progress, i-last_i-1);
    progress_char = mpz_get_str(NULL, 10, progress);
    
    /* on décale de i-last_i-1 */
    sprintf(buffer,shift,i-last_i-1,progress_char);
    fwrite(buffer, strlen(buffer), 1, File);     
    free(progress_char);  
  }
    
  fwrite(end, strlen(end), 1, File); 
  
  mpz_clear(temp);
  mpz_clear(progress);
}
