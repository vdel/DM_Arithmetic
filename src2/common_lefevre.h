#ifndef COMMON_LEFEVRE
#define COMMON_LEFEVRE

#include <stdlib.h>
#include <math.h>
#include <gmp.h>
#include "generate.h"

/*---------------------------------------------------------------------------*/

/* type de motifs */
enum {A, B};

/* Signes */
enum {PLUS, MOINS};

/* un descripteur pour les motifs */
struct motif_struct {
  int *motif;  /* Le motif M'              */
  int nbr;     /* nombre de bits non nuls  */
  int type;    /* type A ou B              */
  int d;       /* décalage des deux motifs */
};
typedef struct motif_struct motif_t;

/* un descripteur pour le calcul récursif */
struct node_struct {
  int *bc;      /* Code de Booth du Mi associé */
  int bc_len;   /* longueur du code de Booth   */
  int var_id;   /* La variable dans laquelle on devra stocker le résultat */
  /* Soit le résultat du noeud est généré par Booth... */
  int gen_by_booth;
  /* ...soit il est généré par Lefèvre avec:  */
  /* soit M =  s1 * N + s2 * R + s3 * 2^k * N */  
  /* soit Mi = s1 * 2^ki * N + s2* Ri         */
  int s1, s2, s3;
  int k;   
};
typedef struct node_struct *node_t;

/* Un descripteur pour l'ensemble des M_i */
struct level_struct {
  node_t *nodes;
  int nbr_nodes;
  struct level_struct *next_level;
  int i,j;                 /* à ce niveau on calcule M_i et M_j */
  int id_N, id_Ri, id_Rj;  /* id du motif et des restes liés au niveau précédent */
};
typedef struct level_struct *level_t;

/*---------------------------------------------------------------------------*/

/*########## Encodage de Booth ##########*/
/* calcule le développement binaire de M */
int M_to_bit_array(
  mpz_t M,
  char **bits
);      

/* calcule l'encodage de Booth */
int M_to_Booth(
  char *bits,
  int num_bits,
  int **codage
);

/* renvoie un chiffre du codage */
int booth_code(
  char b0,
  char b1,
  char b2
);

/* Génère la multiplication avec l'encodage de Booth */
void generate_booth(
  level_t level,
  int node_id
);

/*########## Opérations sur les noeuds de calcul ##########*/
/* Crée un noeud à partir du codage de booth*/
node_t create_node(
  int *bc,
  int bc_len
);

/* Libère un noeud */  
void destroy_node(
  node_t node
);    

/*########## Opérations sur les niveaux de calcul ##########*/
/* Crée un niveau */
level_t create_level(
  level_t prev_level,
  node_t N,
  node_t Ri,
  node_t Rj
);

/* Libère un niveau */
void destroy_level(
  level_t level
);  

/* Génère les opéations du niveau */
void generate_level(
  level_t level
);          

/* Genère les opérations qui se font via Booth */
void generate_leaves(
  level_t level
);

/* Génère le calcul de M = +- 2^k N +- R */
void generate_M(
  char *M,
  char *N,
  int sN,
  char *R,
  int sR,
  int k
);                                                             

/*########## Opérations sur les variables ##########*/
/* Colore le graphe pour assigner une variable aux calculs intermédiaires */
int assign_vars(
  level_t level
);

/* Echange la variable en paramètre */
void swap_vars(
  level_t level,
  int var1,
  int var2
);

/* Echange la variable en paramètre */
void swap_vars_rec(
  level_t level,
  int var1,
  int var2, 
  int i
);

/* Crée l'identificateur de la variable numéro i */
char* get_var(
  int var_id
);

#endif
