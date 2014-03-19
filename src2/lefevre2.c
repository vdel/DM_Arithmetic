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

/* Trouve le motif A ou B avec le plus de bit non nuls dans un même Mi */
motif_t find_motif_same(
  int *bc,
  int bc_len
);

/* Trouve le motif A ou B avec le plus de bit non nuls dans Mi et Mj */
motif_t find_motif_diff(
  int *bc1,
  int bc_len1,
  int *bc2,
  int bc_len2
);

/* initialise les noeuds à partir du motif */
void link_motif_to_nodes_same(
  motif_t motif,
  node_t M, 
  node_t *N,
  node_t *R
);

/* initialise les noeuds à partir du motif */
void link_motif_to_nodes_diff(
  motif_t motif,
  node_t Mi, 
  node_t Mj,  
  node_t *N,
  node_t *Ri,
  node_t *Rj
);

/*---------------------------------------------------------------------------*/

void (*mult_by_m)(mpz_t M) = &mult_lefevre;
const char *algo = "de Lefèvre amélioré";
char *FilePath = "MultLefevre2.c";

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
  printf("generating...\n"); fflush(stdout);  
  generate_level(root);
}

/*---------------------------------------------------------------------------*/
/* Génère les niveaux de calcul pour calculer le niveau level                */
/*---------------------------------------------------------------------------*/
void compute_level(level_t level)
{
  int i, j;
  int best_i, best_j;
  motif_t motif;
  motif.nbr = 0;
  
  for(i=0; i<level->nbr_nodes; i++)
    for(j=i; j<level->nbr_nodes; j++)
    {
      motif_t temp;
      if(i == j)
        temp = find_motif_same(level->nodes[i]->bc, level->nodes[i]->bc_len);
      else
        temp = find_motif_diff(level->nodes[i]->bc, level->nodes[i]->bc_len,
                               level->nodes[j]->bc, level->nodes[j]->bc_len);
      if(temp.nbr>=2)
      {
        best_i = i;
        best_j = j;
        motif = temp;  
      }  
    }
      
  if(motif.nbr == 0)
  {
    for(i=0; i<level->nbr_nodes; i++)
      level->nodes[i]->gen_by_booth = 1;
  }
  else
  {
    level_t next_level;
    node_t N;
    node_t Ri;
    node_t Rj;  
    
    level->i = best_i;  
    level->j = best_j;
        
    if(best_i == best_j)
    {
      link_motif_to_nodes_same(motif, level->nodes[best_i], &N, &Ri);
      Rj = NULL;
    }
    else
      link_motif_to_nodes_diff(motif, level->nodes[best_i], level->nodes[best_j], &N, &Ri, &Rj);
    
    level->i = best_i;
    level->j = best_j;
    next_level = create_level(level, N, Ri, Rj);
    compute_level(next_level);
  }
}

/*---------------------------------------------------------------------------*/
/* Trouve le motif A ou B avec le plus de bit non nuls dans un même Mi       */
/*---------------------------------------------------------------------------*/
motif_t find_motif_same(int *bc, int bc_len)
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
/* Trouve le motif A ou B avec le plus de bit non nuls dans Mi et Mj         */
/*---------------------------------------------------------------------------*/
motif_t find_motif_diff(int *bc1, int bc_len1, int *bc2, int bc_len2)
{
  motif_t motif;
  int i, d;
  int *motifA, *motifB;
  int nbrA, nbrB;
  int max_bc_len = bc_len1<bc_len2?bc_len2:bc_len1;
    
  motifA = malloc(sizeof(int) * bc_len1);
  motifB = malloc(sizeof(int) * bc_len1);
  motif.motif = malloc(sizeof(int) * bc_len1);
  motif.nbr = 0;
  
  for(d=-max_bc_len+1; d<max_bc_len-1; d++)
  {
    nbrA = 0;
    nbrB = 0;
    for(i = 0; i<bc_len1; i++)
    {
      motifA[i]=0;
      motifB[i]=0;    
    }  
  
    for(i=0; i<bc_len1; i++)
    {
      if(i+d<0) continue;
      if(i+d>=bc_len2) break;
      /* Recherche du motif A */
      if(((bc1[i] == 1 && bc2[i+d] == 1) ||
         (bc1[i] == -1 && bc2[i+d] == -1)))
      {
        motifA[i] = bc1[i];
        nbrA++;
      }
      
      /* Recherche du motif B */
      if(((bc1[i] == 1 && bc2[i+d] == -1) ||
         (bc1[i] == -1 && bc2[i+d] == 1)))
      {
        motifB[i] = bc1[i];
        nbrB++;
      }
    }
    
    if(nbrA < nbrB)
    {
      if(motif.nbr < nbrB)
      {
        memcpy(motif.motif, motifB, bc_len1*sizeof(int));
        motif.nbr = nbrB;
        motif.type = B;
        motif.d = d;
      }
    }
    else if(motif.nbr < nbrA)
    {
      memcpy(motif.motif, motifA, bc_len1*sizeof(int));
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
void link_motif_to_nodes_same(motif_t motif,node_t M, node_t *N, node_t *R)
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
  *N = create_node(bc_N, bc_len_N);
    
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
    *R = create_node(bc_R, bc_len_R);    
  }
  else
    *R = NULL;
    
  M->s1 = signe_N;
  M->s2 = signe_R;
  M->s3 = (motif.type==A)?signe_N:(1-signe_N);
  M->k = motif.d;
  
  free(motif.motif);
}

/*---------------------------------------------------------------------------*/
/* initialise les noeuds à partir du motif                                   */
/*---------------------------------------------------------------------------*/
void link_motif_to_nodes_diff(motif_t motif,node_t Mi, node_t Mj, node_t *N, node_t *Ri, node_t *Rj)
{
  int i;
  int first_1_N, first_1_Ri, first_1_Rj;
  int last_1_N;
  int *bc_N, *bc_Ri, *bc_Rj;
  int bc_len_N, bc_len_Ri, bc_len_Rj;
  int signe_N, signe_Ri, signe_Rj;
   
  first_1_N  = -1;
  first_1_Ri = -1;
  first_1_Rj = -1;
  for(i=0; i<Mi->bc_len; i++)
  {
    if(motif.motif[i]!=0) last_1_N = i;
    if(first_1_N==-1 && motif.motif[i]!=0)
    {
      first_1_N = i;
      signe_N = (0<motif.motif[i]) ? PLUS : MOINS;
    }
  }
  for(i=0; i<Mi->bc_len; i++)
  {
    if(first_1_Ri == -1 &&
       Mi->bc[i] != 0 &&
       motif.motif[i] == 0)
    {
      first_1_Ri = i;
      signe_Ri = (0<Mi->bc[i]) ? PLUS : MOINS;      
    }
  }
  for(i=0; i<Mj->bc_len; i++)
  {
    if(first_1_Rj == -1 &&
       Mj->bc[i] != 0 &&
       (i-motif.d<0 || i-motif.d>=Mi->bc_len ||
       motif.motif[i-motif.d] == 0))
    {
      first_1_Rj = i;
      signe_Rj = (0<Mj->bc[i]) ? PLUS : MOINS;      
    }
  }  
  
  bc_len_N = last_1_N - first_1_N + 1;
  bc_N = malloc(sizeof(int) * bc_len_N);
  for(i = first_1_N; i<=last_1_N; i++)
    bc_N[i-first_1_N] = motif.motif[i] * (signe_N==PLUS?1:-1);
  *N = create_node(bc_N, bc_len_N);  
  
  if(first_1_Ri != -1)  /* s'il y a un reste */  
  {
    bc_len_Ri = Mi->bc_len - first_1_Ri;  
    bc_Ri = malloc(sizeof(int) * bc_len_Ri);
    
    for(i = first_1_Ri; i<Mi->bc_len; i++)
      if(Mi->bc[i] != 0 &&
         motif.motif[i] == 0)
        bc_Ri[i-first_1_Ri] = Mi->bc[i] * (signe_Ri==PLUS?1:-1);
      else
        bc_Ri[i-first_1_Ri] = 0;
    *Ri = create_node(bc_Ri, bc_len_Ri);
  }
  else
    *Ri = NULL;

  if(first_1_Rj != -1)  /* s'il y a un reste */  
  {
    bc_len_Rj = Mj->bc_len - first_1_Rj;  
    bc_Rj = malloc(sizeof(int) * bc_len_Rj);
    
    for(i = first_1_Rj; i<Mj->bc_len; i++)
      if(Mj->bc[i] != 0 &&
         (i-motif.d<0 || i-motif.d>=Mi->bc_len ||
         motif.motif[i-motif.d] == 0))
        bc_Rj[i-first_1_Rj] = Mj->bc[i] * (signe_Rj==PLUS?1:-1);
      else
        bc_Rj[i-first_1_Rj] = 0;
    *Rj = create_node(bc_Rj, bc_len_Rj);    
  }
  else
    *Rj = NULL;  
  
  Mi->s1 = signe_N;
  Mi->s2 = signe_Ri;
  Mi->k  = Mi->bc_len - 1 - last_1_N;
  Mj->s1 = (motif.type==A)?signe_N:(1-signe_N);
  Mj->s2 = signe_Rj;
  Mj->k  = Mj->bc_len - 1 - last_1_N - motif.d;

  free(motif.motif);
}
