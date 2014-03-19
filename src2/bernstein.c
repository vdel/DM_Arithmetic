#include <stdlib.h>
#include "generate.h"

/*---------------------------------------------------------------------------*/

struct tree_struct
{
  mpz_t M;
  mpz_t M_red;
  mpz_t max_add;
  int shift;
  int k;
  struct tree_struct *node[4];
};
typedef struct tree_struct *tree_t;

/*---------------------------------------------------------------------------*/

/* Point d'entrée appelé par main */
void mult_bernstein(mpz_t M);

/* Crée un noeud */
tree_t create_node(mpz_t M);

/* Libère un noeud */
void destroy_node(tree_t tree);

/* Réduction de l'étiquette M d'un noeud sous forme M' avec M=2^kM', M' impair */
void node_red(tree_t tree);

/* Construit un arbre de décomposition optimal d'un entier (étiquette de la racine) */
void build_tree(tree_t tree);

/* Génère la séquence des opérations à partir de l'arbre généré précédemment, */
/* le résultat est stocké dans var_res                                        */
void tree_to_ops(tree_t tree, int var_res);

/* Génère la fonction de multiplication par M */
void generate_mult(mpz_t M);


/*---------------------------------------------------------------------------*/

void (*mult_by_m)(mpz_t M) = &mult_bernstein;
const char *algo = "de Bernstein";
char *FilePath = "MultBernstein.c";

/*---------------------------------------------------------------------------*/
/* Point d'entrée appelé par main                                            */
/*---------------------------------------------------------------------------*/
void mult_bernstein(mpz_t M)
{
  FILE *File;
  
  /* Si M = 0 */
  if(mpz_cmp_ui(M,0)==0) 
    add_operation_int(SETUI,label_var(1),NULL,0);   
  else
    generate_mult(M); 
  
  /* On génère le programme */
  File = fopen(FilePath,"wt");
  generate(File, M, algo);
  fclose(File);
}

/*---------------------------------------------------------------------------*/
/* Génère la fonction de multiplication par M                                */
/*---------------------------------------------------------------------------*/
void generate_mult(mpz_t M)
{
  tree_t tree = create_node(M);
  
  build_tree(tree);
  tree_to_ops(tree, 0);
  
  destroy_node(tree);
}

/*-----------------------------------------------------------------------------*/
/* Crée un noeud                                                               */
/*-----------------------------------------------------------------------------*/
tree_t create_node(mpz_t M)
{
  int i;
  tree_t tree = malloc(sizeof(struct tree_struct));
  mpz_init(tree->M);
  mpz_init(tree->M_red);
  mpz_init(tree->max_add);
    
  mpz_set(tree->M, M);
  mpz_set(tree->max_add, M);  
  
  for(i=0; i<4; i++)
    tree->node[i] = NULL;

  node_red(tree);  

  return tree;   
}

/*-----------------------------------------------------------------------------*/
/* Détruit un noeud                                                            */
/*-----------------------------------------------------------------------------*/
void destroy_node(tree_t tree)
{
  int i;
  if(!tree) return;

  mpz_clear(tree->M);
  mpz_clear(tree->M_red); 
  for(i=0; i<4; i++)
    if(tree->node[i])
      destroy_node(tree->node[i]);
  free(tree);
}

/*-----------------------------------------------------------------------------*/
/* Réduction de l'étiquette M d'un noeud sous forme M' avec M=2^kM', M' impair */
/*-----------------------------------------------------------------------------*/
void node_red(tree_t tree)
{
  if(mpz_tstbit(tree->M, 0))
  {
    mpz_set(tree->M_red, tree->M);
    tree->shift = 0;
  }
  else
  {
    tree->shift = mpz_scan1(tree->M,0);
    mpz_cdiv_q_2exp(tree->M_red, tree->M, tree->shift);  
  }
}

/*-----------------------------------------------------------------------------*/
/* Calcul récursif pour décomposer l'étiquette du noeud de manière optimale    */
/*-----------------------------------------------------------------------------*/
void build_tree(tree_t tree)
{
  mpz_t M_tmp;
  mpz_init(M_tmp);
  
  /* si on est appelé et que l'on a le droit à aucune addition,    */
  /* cela signifie que l'on a déjà trouvé une solution directe     */
  /* par ailleurs: on rend la main et disant qu'on n'a pas trouvé. */
  if(mpz_cmp_ui(tree->max_add, 0) <= 0)
  {
    mpz_set_si(tree->max_add,-1);
    return;
  }
  
  if(!mpz_cmp_ui(tree->M_red, 1))
  {
    mpz_set_ui(tree->max_add,0);
  }
  else
  {
    char *bin;
    int len;
    int k;
    tree_t node;
   
    /* M = M'+1 */    
    mpz_sub_ui(M_tmp, tree->M_red, 1);
    tree->node[0] = create_node(M_tmp);
    mpz_sub_ui(tree->node[0]->max_add, tree->max_add, 1);
    mpz_set_si(tree->max_add, -1);
    build_tree(tree->node[0]);
    if(mpz_cmp_si(tree->node[0]->max_add, -1) != 0) 
    {
      mpz_set(tree->max_add, tree->node[0]->max_add);
      mpz_add_ui(tree->max_add, tree->max_add, 1);
    }

    /* M = M'-1 */
    mpz_add_ui(M_tmp, tree->M_red, 1);
    tree->node[1] = create_node(M_tmp);
    mpz_sub_ui(tree->node[1]->max_add, tree->max_add, 1);
    build_tree(tree->node[1]);
    if(mpz_cmp_si(tree->node[1]->max_add, -1) != 0) 
    {
      mpz_set(tree->max_add, tree->node[1]->max_add);
      mpz_add_ui(tree->max_add, tree->max_add, 1);
    }
    
    bin = mpz_get_str(NULL, 2, tree->M_red);
    len = strlen(bin);
    free(bin);
    /* M = (2^k+1)M' */
    tree->node[2] = NULL;
    for(k=len-1;k>0;k--)
    {
      mpz_set_ui(M_tmp, 1);
      mpz_mul_2exp(M_tmp, M_tmp, k);
      mpz_add_ui(M_tmp, M_tmp, 1);
      if(mpz_divisible_p(tree->M_red, M_tmp))
      {
        mpz_cdiv_q(M_tmp, tree->M_red, M_tmp);
        node = create_node(M_tmp);
        mpz_sub_ui(node->max_add, tree->max_add, 1);
        build_tree(node);
        if(mpz_cmp_si(node->max_add, -1) != 0) 
        {
          destroy_node(tree->node[2]);
          tree->node[2] = node;
          tree->k = k;
          mpz_set(tree->max_add, tree->node[2]->max_add);
          mpz_add_ui(tree->max_add, tree->max_add, 1);
        }
        else 
          destroy_node(node);    
      }
    }  
    
    /* M = (2^k-1)M' */
    tree->node[3] = NULL;
    for(k=len-1;k>0;k--)
    {
      mpz_set_ui(M_tmp, 1);
      mpz_mul_2exp(M_tmp, M_tmp, k);
      mpz_sub_ui(M_tmp, M_tmp, 1);
      if(mpz_divisible_p(tree->M_red, M_tmp))
      {
        mpz_cdiv_q(M_tmp, tree->M_red, M_tmp);
        node = create_node(M_tmp);
        mpz_sub_ui(node->max_add, tree->max_add, 1);
        build_tree(node);
        if(mpz_cmp_si(node->max_add, -1) != 0) 
        {
          destroy_node(tree->node[3]);
          tree->node[3] = node;
          tree->k = k;
          mpz_set(tree->max_add, tree->node[3]->max_add);
          mpz_add_ui(tree->max_add, tree->max_add, 1);
        }       
        else 
          destroy_node(node);            
      }  
    }
  }
  
  mpz_clear(M_tmp);
}

/*-----------------------------------------------------------------------------*/
/* Génère la séquence des opérations à partir de l'arbre généré précédemment   */
/*-----------------------------------------------------------------------------*/
void tree_to_ops(tree_t tree, int var_res)
{
  int i;
  char* var[2];
  var[0] = label_var(1);
  var[1] = "tmp";
  
  if(!mpz_cmp_ui(tree->M_red, 1))
  {
    add_operation_str(SET, var[var_res], NULL, label_var(0));   
    if(tree->shift)    
      add_operation_int(SHF, var[var_res], var[var_res], tree->shift);            
  }
  else
  {

    for(i=3; i>0; i--)
      if(tree->node[i] && mpz_cmp_si(tree->node[i]->max_add, -1) != 0) break;
    switch(i)
    {
      case 0:
        tree_to_ops(tree->node[i], var_res);
        add_operation_str(ADD, var[var_res], var[var_res], label_var(0));   
        if(tree->shift)
          add_operation_int(SHF, var[var_res], var[var_res], tree->shift);            
      break;
      case 1:
        tree_to_ops(tree->node[i], var_res);
        add_operation_str(SUB, var[var_res], var[var_res], label_var(0));     
        if(tree->shift)         
          add_operation_int(SHF, var[var_res], var[var_res], tree->shift); 
      break;
      case 2:
        tree_to_ops(tree->node[i], 1-var_res);
        add_operation_str(SET, var[var_res], NULL, var[1-var_res]);
        add_operation_int(SHF, var[1-var_res], var[1-var_res], tree->k);
        add_operation_str(ADD, var[var_res], var[var_res], var[1-var_res]);                
        if(tree->shift)        
          add_operation_int(SHF, var[var_res], var[var_res], tree->shift);
      break;
      case 3:
        tree_to_ops(tree->node[i], 1-var_res);
        add_operation_str(SET, var[var_res], NULL, var[1-var_res]);
        add_operation_int(SHF, var[var_res], var[var_res], tree->k);
        add_operation_str(SUB, var[var_res], var[var_res], var[1-var_res]);                
        if(tree->shift)        
          add_operation_int(SHF, var[var_res], var[var_res], tree->shift); 
      break;    
    }
  }
}
