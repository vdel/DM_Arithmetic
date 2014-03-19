#include "common_lefevre.h"

/*---------------------------------------------------------------------------*/

/* Point d'entrée appelé par main */
void mult_lefevre(
  mpz_t M
);

/* Génère la fonction de multiplication par M */
void generate_mult(
  mpz_t M
);

/* Génère les niveaux de calcul pour calculer le niveau level */
void compute_level(
  level_t level
);

/* Trouve le motif A ou B avec le plus de bit non nuls */
motif_t find_motif(
  int *bc,
  int bc_len
);

/* initialise les noeuds à partir du motif */
void link_motif_to_nodes(
  motif_t motif,
  node_t M, 
  node_t *N,
  node_t *R
);

/*---------------------------------------------------------------------------*/

void (*mult_by_m)(mpz_t M) = &mult_lefevre;
const char *algo = "de Lefèvre";
char *FilePath = "MultLefevre1.c";

/*---------------------------------------------------------------------------*/
/* Point d'entrée appelé par main                                            */
/*---------------------------------------------------------------------------*/
void mult_lefevre(mpz_t M)
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
  char *bits=NULL;
  int *booth_code;
  int num_bits;
  int len_code;
  node_t node;
  level_t root;

  /* On récupère l'écriture binaire de M et le nombre de bits */
  num_bits = M_to_bit_array(M, &bits);

  /* on calcule le codage de booth de M */
  len_code = M_to_Booth(bits, num_bits, &booth_code);
  
  /* On crée le noeud et le niveau associés au calcul de M */
  node = create_node(booth_code, len_code);
  root = malloc(sizeof(struct level_struct));
  root->nbr_nodes = 1;
  root->nodes = malloc(sizeof(node_t));
  root->nodes[0] = node;
  root->id_N = -1;  /* pour dire que c'est le niveau racine */
  
  /* On génère les niveaux intermédiaires menant au calcul de M */
  compute_level(root);
  
  /* On assigne les variables à chaque noeud de calcul */
  assign_vars(root);
  if(node->var_id != 0)
    swap_vars(root, 0, node->var_id);
  
  /* On génère les opérations corespondant au niveau */
  generate_level(root);
}

/*---------------------------------------------------------------------------*/
/* Génère les niveaux de calcul pour calculer le niveau level                */
/*---------------------------------------------------------------------------*/
void compute_level(level_t level)
{
  int i;
  level_t next_level;
  
  for(i=0; i<level->nbr_nodes; i++)
  {
    if(level->nodes[i]->gen_by_booth)
      continue;
    else
    {
      motif_t motif = find_motif(level->nodes[i]->bc, level->nodes[i]->bc_len);
      if(motif.nbr<2)
      {
        level->nodes[i]->gen_by_booth = 1;
        continue;
      }
      else
      {
        node_t N;
        node_t R;        
        link_motif_to_nodes(motif, level->nodes[i], &N, &R);
        
        level->i = i;
        level->j = i;
        next_level = create_level(level, N, R, NULL);
        break;
      }  
    }
  }
  
  if(i==level->nbr_nodes)
    return;
  else
    compute_level(next_level);  
}

/*---------------------------------------------------------------------------*/
/* Trouve le motif A ou B avec le plus de bit non nuls                       */
/*---------------------------------------------------------------------------*/
motif_t find_motif(int *bc, int bc_len)
{
  motif_t motif;
  int i, d;
  int *motifA, *motifB;
  int nbrA, nbrB;
  
  motifA = malloc(sizeof(int) * bc_len);
  motifB = malloc(sizeof(int) * bc_len);
  motif.motif = malloc(sizeof(int) * bc_len);
  motif.nbr = 0;
   
  for(d=1; d<bc_len-1; d++)
  {
    nbrA = 0;
    nbrB = 0;
    for(i = 0; i<bc_len; i++)
    {
      motifA[i]=0;
      motifB[i]=0;    
    }  
  
    for(i=0; i<bc_len-d; i++)
    {
      /* Recherche du motif A */
      if(motifA[i] == 0 
         && ((bc[i] == 1  && bc[i+d] == 1) 
         ||  (bc[i] == -1 && bc[i+d] == -1)))
      {
        motifA[i+d] = bc[i+d];
        nbrA++;
      }
      
      /* Recherche du motif B */
      if(motifB[i] == 0 
         && ((bc[i] == 1  && bc[i+d] == -1) 
         ||  (bc[i] == -1 && bc[i+d] == 1)))
      {
        motifB[i+d] = bc[i+d];
        nbrB++;
      }
    }
    
    if(nbrA < nbrB)
    {
      if(motif.nbr < nbrB)
      {
        memcpy(motif.motif, motifB, bc_len*sizeof(int));
        motif.nbr = nbrB;
        motif.type = B;
        motif.d = d;
      }
    }
    else if(motif.nbr < nbrA)
    {
      memcpy(motif.motif, motifA, bc_len*sizeof(int));
      motif.nbr = nbrA;
      motif.type = A;
      motif.d = d;
    }
  }

  free(motifA);
  free(motifB);
  return motif;
}

/*---------------------------------------------------------------------------*/
/* initialise les noeuds à partir du motif                                   */
/*---------------------------------------------------------------------------*/
void link_motif_to_nodes(motif_t motif,node_t M, node_t *N, node_t *R)
{
  int i;
  int first_1_N, first_1_R;
  int *bc_N, *bc_R;
  int bc_len_N, bc_len_R;
  int signe_N, signe_R;
  
  first_1_N = -1;
  first_1_R = -1;
  for(i=0; i<M->bc_len; i++)
  {
    if(first_1_N==-1 && motif.motif[i]!=0)
    {
      first_1_N = i;
      signe_N = (0<motif.motif[i]) ? PLUS : MOINS;
    }
    if(first_1_R == -1 &&
       M->bc[i] != 0 &&
       motif.motif[i] == 0 &&
       (i+motif.d >= M->bc_len || motif.motif[i+motif.d] == 0))
    {
      first_1_R = i;
      signe_R = (0<M->bc[i]) ? PLUS : MOINS;      
    }
  }
  
  bc_len_N = M->bc_len - first_1_N;
  bc_N = malloc(sizeof(int) * bc_len_N);
  
  for(i = first_1_N; i<M->bc_len; i++)
    bc_N[i-first_1_N] = motif.motif[i] * (signe_N==PLUS?1:-1);
  
  if(first_1_R != -1)  /* s'il y a un reste */  
  {
    bc_len_R = M->bc_len - first_1_R;  
    bc_R = malloc(sizeof(int) * bc_len_R);
 
    for(i = first_1_R; i<M->bc_len; i++)
      if(M->bc[i] != 0 &&
         motif.motif[i] == 0 &&
         (i+motif.d >= M->bc_len || motif.motif[i+motif.d] == 0))
        bc_R[i-first_1_R] = M->bc[i] * (signe_R==PLUS?1:-1);
      else
        bc_R[i-first_1_R] = 0;
  }
  
  *N = create_node(bc_N, bc_len_N);
  if(first_1_R != -1)
    *R = create_node(bc_R, bc_len_R);
  else
    *R = NULL;
  
  M->s1 = signe_N;
  M->s2 = signe_R;
  M->s3 = (motif.type==A)?signe_N:(1-signe_N);
  M->k = motif.d;
  
  free(motif.motif);
}
