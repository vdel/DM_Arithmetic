#include "generate.h"

/*---------------------------------------------------------------------------*/

#define REALLOC_OPS_BY   10
#define REALLOC_VARS_BY  5

typedef struct operation
{
  int type;
  unsigned long op1;
  unsigned long op2;
  unsigned long op3;
  mpz_t value;  
  int saved;
  char *comment;
} operation;   

typedef struct variable
{
  char *var;
  mpz_t value;  
  int to_declare;
} variable;  

unsigned long max_vars;
unsigned long max_ops;
unsigned long nbr_vars;
unsigned long nbr_ops;
unsigned int nbr_add;
unsigned int nbr_mul;
unsigned int nbr_shf;
unsigned int nbr_set;
variable *vars;
operation *ops;

/*---------------------------------------------------------------------------*/

void generate_main(FILE *File, mpz_t M, const char *algo); /* génère la fonction main */

void generate_init_vars(FILE *File);    /* l'initialisation des variables temporaires */
void generate_clear_vars(FILE *File);   /* la destruction des variables temporaires */
void generate_ops(FILE *File);          /* génère les opérations */

void generate_init_var(FILE *File, int i, int is_decl); /* le code pour l'initialisation de la variable i */
void generate_clear_var(FILE *File, int i);             /* le code pour la destruction de la variable i */
void generate_op(FILE *File, int i);                    /* le code pour l'opération i */

void add_var_complete(char *var, int to_declare);
int  get_var_index(char *var);

/*---------------------------------------------------------------------------*/
void init_data_generate(const char* res, const char* x)
{
  max_vars = 0;
  max_ops  = 0;
  nbr_vars = 0;
  nbr_ops  = 0;
  nbr_add  = 0;
  nbr_mul  = 0;
  nbr_shf  = 0;
  nbr_set  = 0;    
  ops  = NULL;  
  vars = NULL;
  add_var_complete((char*)x  , 0);  
  add_var_complete((char*)res, 0);  
}
/*---------------------------------------------------------------------------*/
void clear_data_generate()
{
  unsigned int i;
  for(i=0; i<nbr_vars; i++)
  {
    mpz_clear(vars[i].value);
    free(vars[i].var);
  }
  for(i=0; i<nbr_ops; i++)
    mpz_clear(ops[i].value);
    
  if(vars != NULL)
    free(vars);
  if(ops != NULL)
    free(ops);

  printf("\n");
  printf("%u addition(s)/soustraction(s)\n",nbr_add);
  printf("%u multiplication(s)\n",nbr_mul);  
  printf("%u décalage(s)\n",nbr_shf);    
  printf("%u affectation(s)\n",nbr_set);  
  printf("\n");

  max_vars = 0;
  max_ops  = 0;
  nbr_vars = 0;
  nbr_ops  = 0;
  nbr_add  = 0;
  nbr_mul  = 0;
  nbr_shf  = 0;
  nbr_set  = 0;     
  ops  = NULL;  
  vars = NULL;        
}
/*---------------------------------------------------------------------------*/
void generate(FILE *File, mpz_t M, const char *algo)
{
  generate_main(File, M, algo);
  generate_init_vars(File);
  generate_ops(File);
  generate_clear_vars(File);
}
/*---------------------------------------------------------------------------*/
void generate_main(FILE *File, mpz_t M, const char *algo)
{
  char buffer[1000];
  char header[]="\
#include <gmp.h>\n\
\n\
void MultBy%s(mpz_t %s, mpz_t %s);\n\
\n\
void (*MultByM)(mpz_t %s, mpz_t %s) = MultBy%s;\n\
\n\
/* Multiplication par %s en utilisant l'algorithme %s*/\n\
void MultBy%s(mpz_t %s, mpz_t %s)\n\
{\n";
  sprintf(buffer, header,
          mpz_get_str(NULL, 10, M),  
          vars[1].var,
          vars[0].var,
          vars[1].var,
          vars[0].var,            
          mpz_get_str(NULL, 10, M),          
          mpz_get_str(NULL, 10, M),
          algo,
          mpz_get_str(NULL, 10, M),          
          vars[1].var,
          vars[0].var);  
  fwrite(buffer , strlen(buffer) , 1, File);   
}
/*---------------------------------------------------------------------------*/
void generate_init_vars(FILE *File)
{
  unsigned int i;
  for(i=0;i<nbr_vars;i++)
    if(vars[i].to_declare)
      generate_init_var(File, i, 1);
  for(i=0;i<nbr_vars;i++)
    if(vars[i].to_declare)
      generate_init_var(File, i, 0);
  if(nbr_vars>2)
    fwrite("\n", 1, 1, File);      
}
/*---------------------------------------------------------------------------*/
void generate_clear_vars(FILE *File)
{
  unsigned int i;
  if(nbr_vars>2)   
    fwrite("\n", 1, 1, File);
  for(i=0;i<nbr_vars;i++)
    if(vars[i].to_declare)
      generate_clear_var(File, i);   
    fwrite("}\n", 2, 1, File);      
}
/*---------------------------------------------------------------------------*/
void generate_ops(FILE *File)
{
  unsigned int i;
  for(i=0;i<nbr_ops;i++)
    generate_op(File, i);
}
/*---------------------------------------------------------------------------*/
void generate_init_var(FILE *File, int i, int is_decl)
{
  char buffer[100];
  char decl[] = "  mpz_t %s;\n";
  char init[] = "  mpz_init(%s);\n";  
  char *str = is_decl ? decl : init;
  
  sprintf(buffer,str,vars[i].var);
  fwrite(buffer, strlen(buffer), 1, File);      
}
/*---------------------------------------------------------------------------*/
void generate_clear_var(FILE *File, int i)
{
  char buffer[100];
  const char *str = "  mpz_clear(%s);\n";
  sprintf(buffer,str,vars[i].var);
  fwrite(buffer, strlen(buffer), 1, File);      
}
/*---------------------------------------------------------------------------*/
void generate_op(FILE *File, int i)
{
  char buffer[300];
  char add[] = "  mpz_add(%s, %s, %s);";
  char sub[] = "  mpz_sub(%s, %s, %s);";    
  char shf[] = "  mpz_mul_2exp(%s, %s, %d);";
  char mul[] = "  mpz_mul_ui(%s, %s, %d);";
  char set[] = "  mpz_set(%s, %s);";
  char setui[]="  mpz_set_ui(%s, %d);";  
  char addui[]="  mpz_add_ui(%s, %s, %d);";
  char subui[]="  mpz_sub_ui(%s, %s, %d);";      
  char comment[] = "/* %s = %s%s */\n";
  char *var1,*var2,*var3;
  int len;
  
  if(ops[i].comment)
  {
    sprintf(buffer,"  /* %s */\n",ops[i].comment);  
    fwrite(buffer, strlen(buffer), 1, File);    
    free(ops[i].comment);     
  }
  
  if(ops[i].type == EMPTY) return;
  switch(ops[i].type)
  {
    case ADD:
    var1=vars[ops[i].op1].var;
    var2=vars[ops[i].op2].var;
    var3=vars[ops[i].op3].var;    
    sprintf(buffer,add,var1,var2,var3);
    break;
    case SUB:
    var1=vars[ops[i].op1].var;
    var2=vars[ops[i].op2].var;
    var3=vars[ops[i].op3].var;    
    sprintf(buffer,sub,var1,var2,var3);
    break;
    case MULUI:
    var1=vars[ops[i].op1].var;
    var2=vars[ops[i].op2].var;
    sprintf(buffer,mul,var1,var2,ops[i].op3);
    break;
    case SHF:
    var1=vars[ops[i].op1].var;
    var2=vars[ops[i].op2].var;
    sprintf(buffer,shf,var1,var2,ops[i].op3);
    break;
    case SET:
    var1=vars[ops[i].op1].var;   
    var3=vars[ops[i].op3].var;       
    sprintf(buffer,set,var1,var3);        
    break;
    case SETUI:
    var1=vars[ops[i].op1].var;    
    sprintf(buffer,setui,var1,ops[i].op3);    
    break;    
    case ADDUI:
    var1=vars[ops[i].op1].var; 
    var2=vars[ops[i].op2].var;       
    sprintf(buffer,addui,var1,var2,ops[i].op3);  
    break;    
    case SUBUI:
    var1=vars[ops[i].op1].var;
    var2=vars[ops[i].op2].var;        
    sprintf(buffer,subui,var1,var2,ops[i].op3);          
    break;    
  }
  len = strlen(buffer);
  fwrite(buffer, len, 1, File);      
  len = 33 - len;
  if(len<1) len = 1;
  for(;len>0;len--)
    fwrite(" ", 1, 1, File); 
  sprintf(buffer,comment,var1,mpz_get_str(NULL, 10, ops[i].value),vars[0].var);  
  fwrite(buffer, strlen(buffer), 1, File); 
}
/*---------------------------------------------------------------------------*/
void add_var_complete(char *var, int to_declare)
{
  if(nbr_vars == max_vars)
  {
    max_vars += REALLOC_VARS_BY;
    vars = realloc(vars, sizeof(variable) * max_vars);  
  }
  
  vars[nbr_vars].var = malloc((strlen(var)+1) * sizeof(char));
  strcpy(vars[nbr_vars].var,var);
  mpz_init(vars[nbr_vars].value);
  mpz_set_ui(vars[nbr_vars].value, 1);
  vars[nbr_vars].to_declare = to_declare;
  
  nbr_vars++;
}
/*---------------------------------------------------------------------------*/
void add_variable(char *var)
{
  unsigned int i;
  for(i=0; i<nbr_vars; i++)
    if(!strcmp(var,vars[i].var))
      break;
     
  if(i == nbr_vars)    
    add_var_complete((char*)var, 1);
}
/*---------------------------------------------------------------------------*/
void add_operation_str(int type, const char *op1, const char *op2, const char *op3)
{
  if(nbr_ops == max_ops)
  {
    max_ops += REALLOC_OPS_BY;
    ops = realloc(ops, sizeof(operation) * max_ops);  
  }
  ops[nbr_ops].type = type;
  ops[nbr_ops].comment = NULL;  
  ops[nbr_ops].op1 = get_var_index((char*)op1); 
  if(type!=SET)
    ops[nbr_ops].op2 = get_var_index((char*)op2);  
  ops[nbr_ops].saved = 0;
  mpz_init(ops[nbr_ops].value);
  switch(type)
  {
    case ADD:
    nbr_add++;
    ops[nbr_ops].op3 = get_var_index((char*)op3); 
    mpz_add(vars[ops[nbr_ops].op1].value,
            vars[ops[nbr_ops].op2].value,
            vars[ops[nbr_ops].op3].value);
    break;             
    case SUB:
    nbr_add++;
    ops[nbr_ops].op3 = get_var_index((char*)op3); 
    mpz_sub(vars[ops[nbr_ops].op1].value,
            vars[ops[nbr_ops].op2].value,
            vars[ops[nbr_ops].op3].value);
    break;
    case SET:
    nbr_set++;
    ops[nbr_ops].op3 = get_var_index((char*)op3); 
    mpz_set(vars[ops[nbr_ops].op1].value,
            vars[ops[nbr_ops].op3].value);
    break;
    default:
    printf("Unknown STR operation\n");
    exit(1);
    break;           
  }
  mpz_set(ops[nbr_ops].value, vars[ops[nbr_ops].op1].value);
  
  nbr_ops++;
}
/*---------------------------------------------------------------------------*/
void add_operation_int(int type, const char *op1, const char *op2, unsigned long op3)
{
  if(nbr_ops == max_ops)
  {
    max_ops += REALLOC_OPS_BY;
    ops = realloc(ops, sizeof(operation) * max_ops);  
  }
  ops[nbr_ops].type = type;
  ops[nbr_ops].comment = NULL;  
  ops[nbr_ops].op1 = get_var_index((char*)op1); 
  if(type != SETUI) ops[nbr_ops].op2 = get_var_index((char*)op2);  
  ops[nbr_ops].saved = 0;  
  mpz_init(ops[nbr_ops].value);
  
  switch(type)
  {
    case SHF:
    nbr_shf++;
    ops[nbr_ops].op3 = op3;
    mpz_mul_2exp(vars[ops[nbr_ops].op1].value,
                 vars[ops[nbr_ops].op2].value,
                 ops[nbr_ops].op3);             
    break;    
    case MULUI:
    nbr_mul++;
    ops[nbr_ops].op3 = op3;
    mpz_mul_ui(vars[ops[nbr_ops].op1].value,
               vars[ops[nbr_ops].op2].value,
               ops[nbr_ops].op3);               
    break;
    case ADDUI:
    nbr_add++;
    ops[nbr_ops].op3 = op3;
    mpz_add_ui(vars[ops[nbr_ops].op1].value,
               vars[ops[nbr_ops].op2].value,
               ops[nbr_ops].op3);    
    break;        
    case SUBUI:
    nbr_add++;
    ops[nbr_ops].op3 = op3;
    mpz_sub_ui(vars[ops[nbr_ops].op1].value,
               vars[ops[nbr_ops].op2].value,
               ops[nbr_ops].op3);                   
    break;
    case SETUI:
    nbr_set++;
    ops[nbr_ops].op3 = op3;
    mpz_set_ui(vars[ops[nbr_ops].op1].value,ops[nbr_ops].op3);       
    break;
    default:
    printf("Unknown INT operation\n");
    exit(1);
    break;
  }
  
  mpz_set(ops[nbr_ops].value, vars[ops[nbr_ops].op1].value);
  fflush(stdout);  
  nbr_ops++;
}
/*---------------------------------------------------------------------------*/
void add_comment(char *comment)
{
  if(nbr_ops == max_ops)
  {
    max_ops += REALLOC_OPS_BY;
    ops = realloc(ops, sizeof(operation) * max_ops);  
  }
  ops[nbr_ops].type = EMPTY;
  ops[nbr_ops].comment = malloc((strlen(comment)+1)*sizeof(char));
  strcpy(ops[nbr_ops].comment,comment);
  nbr_ops++;  
}
/*---------------------------------------------------------------------------*/  
int get_var_index(char *var)
{
  unsigned int i;
  for(i=0; i<nbr_vars; i++)
    if(!strcmp(var,vars[i].var))
      return i;
         
  add_variable(var);
  return nbr_vars-1;
}
/*---------------------------------------------------------------------------*/
char* label_var(int n)
{
  return vars[n].var;
}
/*---------------------------------------------------------------------------*/
