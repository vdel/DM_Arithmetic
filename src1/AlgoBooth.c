#include <gmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Alloue le tableau bits et y écrit le développement binaire de M */
/* Renvoie le nombre de bits du développement */
int M_to_bit_array(mpz_t M,char **bits);

/* convertit l'entier binaire réprésenté par la chaine 'bits' */
/* en codage de Booth modifié */
int M_to_Booth(char *bits, int num_bits, int **codage);

/* renvoie un chiffre du codage */
int booth_code(char b0, char b1, char b2);


/* Génère l'en-tête du fichier */
void generate_header(FILE *File);

/* Génère la fonction de multiplication par M */
void generate_mult(FILE *File, int *booth_code, int len_code, char *M);

/* Génère le code pour calculer -2,1,1 ou 2 fois x */
void generate_set_temp(FILE *File,int code, char *progress, int init);

/*---------------------------------------------------------------------------*/
int main(int argc, char** argv)
{
  mpz_t M;
  char *bits=NULL;
  int *both_code;
  int num_bits;
  int len_code;
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
  /* on calcule le codage de booth de M */
  len_code = M_to_Booth(bits, num_bits, &both_code);  
  
  /* On génère le programme */
  File = fopen("MultBooth.c","wt");
  generate_header(File);
  generate_mult(File, both_code, len_code, argv[1]);  
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
/* convertit l'entier binaire réprésenté par la chaine 'bits'                */
/* en codage de Booth modifié                                                */
/*---------------------------------------------------------------------------*/
int M_to_Booth(char *bits, int num_bits, int **codage)
{
  int i;  
  int len = num_bits/2 + 1;
  *codage = malloc(sizeof(int) * len);

  /* les cas particuliers pour le début*/
  if(num_bits==0)
  {
    (*codage)[len-1]=0;
    return 1;
  }
  else if(num_bits==1)
  {
    (*codage)[len-1]=booth_code('0',bits[num_bits-1],'0');
    return 1;
  }
  else    
    (*codage)[len-1]=booth_code('0',bits[num_bits-1],bits[num_bits-2]);
  
  for(i=1;i<len-1;i++)
    (*codage)[len-i-1]=booth_code(bits[num_bits-1-2*i+1],bits[num_bits-1-2*i],bits[num_bits-1-2*i-1]);
  
  /* les cas particulier pour la fin */ 
  if(num_bits==len*2-1)
    (*codage)[0]=booth_code(bits[0],bits[1],'0');   
  else 
    (*codage)[0]=booth_code(bits[0],'0','0');
    
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
void generate_mult(FILE *File, int *booth_code, int len_code, char *M)
{
  int last_i,i;
  int is_gen_2x;
  char buffer[1000];
  char begin[] = "void multbyM(mpz_t res, mpz_t x)  /* Multiplication par %s */\n{\n%s";   
  char shift[] = "  mpz_mul_2exp(res, res, %d);  /* res  = %sx */\n";
  char end[]="%s}\n";
  char *progress_char;     /* pour afficher où on en est dans les commentaires */
  mpz_t progress,temp;  /* pour afficher où on en est dans les commentaires */

  mpz_init(temp);  
  mpz_init(progress);

  is_gen_2x=0;   /* on a parfois besoin de calculer 2x */
  for(i=0;i<len_code;i++)
    if(booth_code[i]==2 || booth_code[i]==-2) 
    {
      is_gen_2x=1;
      break;
    }
  
  /* on génère l'en-tête de la fonction */
  sprintf(buffer,begin,M,is_gen_2x?"  mpz_t _2x;\n  mpz_init(_2x);\n  mpz_mul_ui(_2x, x, 2);\n\n":"");
  fwrite(buffer, strlen(buffer), 1, File);
    
  /* Le bits de gauche est forcément non nul */
  /* on calcule la valeur actuelle de 'res' pour les commentaires */       
  mpz_set_ui(progress, booth_code[0]);      

  /* on initialise 'res' */      
  generate_set_temp(File,booth_code[0],NULL,1);
  
  last_i=0;
  i=1;  
  while(i<len_code)
  {
    /* on cherche le prochain 1 */
    while(i<len_code && booth_code[i]==0)
      i++;

    /* on quitte la boucle */  
    if(i==len_code) break;
      
    /* on calcule la valeur actuelle de 'res' pour les commentaires */
    mpz_mul_2exp(progress, progress, 2*(i-last_i));
    progress_char = mpz_get_str(NULL, 10, progress);
    
    /* on décale de i-last_i */
    sprintf(buffer,shift,2*(i-last_i),progress_char);
    fwrite(buffer, strlen(buffer), 1, File);     
    free(progress_char);

    /* on calcule la valeur actuelle de 'res' pour les commentaires */       
    mpz_set_si(temp, booth_code[i]);
    mpz_add(progress, progress, temp);      
    progress_char = mpz_get_str(NULL, 10, progress);

    /* on ajoute 'x' à 'res' */      
    generate_set_temp(File,booth_code[i],progress_char,0);         
    free(progress_char);
    
    last_i=i;    
    i++;      
  }

  /* en fin de génération, il ne faut pas oublier le dernier décalage */
  /* qui peut ne pas avoir été fait si l'écriture binaire de M fini par des 0 */
  if(i-last_i-1 != 0)
  {
    /* on calcule la valeur actuelle de 'res' pour les commentaires */
    mpz_mul_2exp(progress, progress, 2*(i-last_i-1));
    progress_char = mpz_get_str(NULL, 10, progress);
    
    /* on décale de i-last_i-1 */
    sprintf(buffer,shift,2*(i-last_i-1),progress_char);
    fwrite(buffer, strlen(buffer), 1, File);     
    free(progress_char);  
  }

  /* on génère la fin de la fonction */
  sprintf(buffer,end,is_gen_2x?"\n  mpz_clear(_2x);\n":"");
  fwrite(buffer, strlen(buffer), 1, File);        
  
  mpz_clear(temp);
  mpz_clear(progress);
}

/*---------------------------------------------------------------------------*/
/* Génère le code pour calculer -2,1,1 ou 2 fois x                           */
/*---------------------------------------------------------------------------*/
void generate_set_temp(FILE *File,int code, char *progress, int init)
{
  char buffer[1000];
  char mul[]   = "  mpz_mul_ui(res, x, %d);      /* res  = %dx */\n";      
  char add[]   = "  mpz_add(res, res, %s);    /* res  = %sx */\n";
  char sub[]   = "  mpz_sub(res, res, %s);    /* res  = %sx */\n";  
  
  
  if(init) /* à l'initialisation, on utilise 'res' directement */
  {
    sprintf(buffer, mul, code, code);
    fwrite(buffer, strlen(buffer), 1, File);           
  }  
  else
  {
    switch(code)
    {
      case -2: sprintf(buffer, sub, "_2x ", progress); break;
      case -1: sprintf(buffer, sub, "x   ", progress); break;
      case 1:  sprintf(buffer, add, "x   ", progress); break;
      case 2:  sprintf(buffer, add, "_2x ", progress); break;      
    }        
    fwrite(buffer, strlen(buffer), 1, File); 
  } 
}
