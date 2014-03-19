#include "common_lefevre.h"

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
/* Génère la multiplication avec l'encodage de Booth                         */
/*---------------------------------------------------------------------------*/
void generate_booth(level_t level, int node_id)
{
  /* Booth optimisé pour un entier positif (premier bit = 1)  */
  int *bc = level->nodes[node_id]->bc;
  int bc_len = level->nodes[node_id]->bc_len;
  int first_i,last_i,i;
  char *x = label_var(0);
  char *res = get_var(level->nodes[node_id]->var_id);
  
  /* on cherche le premier bit de poids fort non nul */
  i=0;  
  while(i<bc_len && bc[i]==0)
    i++;
  first_i=i;  
  last_i=i;
  i++;
  
  add_comment("<<<<< Booth");
    
  /* M doit être plus grand que pour pouvoir faire le SET et SHIFT en un seul SHIFT */
  if(bc_len<=2)   /* res = x */
  {
    add_operation_str(SET,res,NULL,x); 
    add_comment("Booth >>>>>");     
    return;
  }
  
  while(i<bc_len)
  {
    /* on cherche le prochain 1 */
    while(i<bc_len && bc[i]==0)
      i++;

    /* on quitte la boucle */  
    if(i==bc_len) break;

    /* on décale de i-last_i */
    add_operation_int(SHF,res,last_i==first_i?x:res,i-last_i);     

    /* on ajoute 'x' à 'res' */      
    if(bc[i]<0)
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

  add_comment("Booth >>>>>");    
  free(res);    
}

/*---------------------------------------------------------------------------*/
/* Crée un noeud                                                             */
/*---------------------------------------------------------------------------*/
node_t create_node(int *bc, int bc_len)
{
  node_t node = malloc(sizeof(struct node_struct));
  
  node->bc = bc;
  node->bc_len = bc_len;
  node->gen_by_booth = 0;
  node->var_id = -1;
  
  return node;
}

/*---------------------------------------------------------------------------*/
/* Libère un noeud                                                           */
/*---------------------------------------------------------------------------*/
void destroy_node(node_t node)
{
  free(node->bc);
  free(node);
}

/*---------------------------------------------------------------------------*/
/* Crée un niveau                                                            */
/*---------------------------------------------------------------------------*/
level_t create_level(level_t prev_level, node_t N, node_t Ri, node_t Rj)
{
  int i,j;
  level_t level = malloc(sizeof(struct level_struct));
  
  /*** Mise à jour de level ***/
  /* On enlève M_i, M_j et on ajoute N, R_i et R_j ce qui fait un nombre de plus */
  level->nbr_nodes = prev_level->nbr_nodes + 1;
  if(prev_level->i != prev_level->j)
    level->nbr_nodes -= 2;
  else
    level->nbr_nodes -= 1;
  if(Ri) level->nbr_nodes++;
  if(Rj) level->nbr_nodes++;  
  level->nodes = malloc(sizeof(node_t) * level->nbr_nodes);
  
  /* On recopie les nombres sauf les M_i et M_j */
  for(i=0, j=0; i<prev_level->nbr_nodes; i++)
    if(i!=prev_level->i && i!=prev_level->j)
    {
      level->nodes[j] = prev_level->nodes[i];
      j++;
    }

  /* On ajoute N, Ri et Rj à la liste */
  level->nodes[j] = N;
  level->id_N = j;
  j++;
  if(Ri)
  {
    level->nodes[j] = Ri;
    level->id_Ri = j;
    j++;
  }
  else
    level->id_Ri = -1;
  if(Rj)
  {
    level->nodes[j] = Rj;
    level->id_Rj = j;    
  }
  else
    level->id_Rj = -1;
 
  prev_level->next_level = level;
  level->next_level = NULL;
  
  return level;
}

/*---------------------------------------------------------------------------*/
/* Libère un niveau                                                          */
/*---------------------------------------------------------------------------*/
void destroy_level(level_t level)
{ 
  destroy_level(level->next_level);
  
  if(level->id_N != -1)
  {
    int i;
    for(i=0; i<level->nbr_nodes; i++)
      destroy_node(level->nodes[i]);
  }
  else
  {
    destroy_node(level->nodes[level->id_N]);
    destroy_node(level->nodes[level->id_Ri]);
    if(level->id_Rj != -1)
      destroy_node(level->nodes[level->id_Rj]);        
  }
  free(level->nodes);
  free(level);
}

/*---------------------------------------------------------------------------*/
/* Genère les opérations du niveau                                           */
/*---------------------------------------------------------------------------*/
void generate_level(level_t level)
{
  if(level->next_level)
  {
    if(level->i == level->j)
    {
      char *M, *N, *R;
      int k,s0,s1,s2,s3;
      level_t next_level = level->next_level;
      
      generate_level(next_level);

      M = get_var(level->nodes[level->i]->var_id);
      N = get_var(next_level->nodes[next_level->id_N]->var_id);
      if(next_level->id_Ri != -1)
        R = get_var(next_level->nodes[next_level->id_Ri]->var_id);
      else
        R = NULL;
      k = level->nodes[level->i]->k;
      s1 = level->nodes[level->i]->s1;
      s2 = level->nodes[level->i]->s2;
      s3 = level->nodes[level->i]->s3;
      
      if(R)
      {
        s0 = PLUS;
        switch(s1 + 2*s2)
        {
          case 0:  /* N + R + 2^k N*/
          add_operation_str(ADD,R,N,R);                
          break;     
          case 1:  /* R - N */
          add_operation_str(SUB,R,R,N);                
          break;
          case 2:  /* N - R */
          add_operation_str(SUB,R,N,R);                
          break;
          case 3:  /* -N - R */
          add_operation_str(ADD,R,N,R);
          s0 = MOINS;
          break; 
        }
        generate_M(M, N, s3, R, s0, k);
      }       
      else
      {
        add_operation_int(SHF,M,N,k);
        if(s1 == PLUS)
          add_operation_str(ADD,M,M,N);
        else
          add_operation_str(SUB,M,M,N);        
      }
        
      free(M);
      free(N);
      free(R);
    }
    else
    {
      char *Mi, *Mj;
      char *Ri, *Rj;
      char *N;
      int ki, kj;
      int s1i, s2i;
      int s1j, s2j;      
      level_t next_level = level->next_level;
            
      generate_level(next_level);
      
      /* On assigne les Mi, Mj, Ri, etc... on commence par celui avec le plus petit k */
      Mi = get_var(level->nodes[level->i]->var_id);
      Mj = get_var(level->nodes[level->j]->var_id);  
      if(next_level->id_Ri != -1)
        Ri = get_var(next_level->nodes[next_level->id_Ri]->var_id);
      else
        Ri = NULL;
      if(next_level->id_Rj != -1)
        Rj = get_var(next_level->nodes[next_level->id_Rj]->var_id);    
      else
        Rj = NULL;
      N  = get_var(next_level->nodes[next_level->id_N]->var_id);     
      
      if(level->nodes[level->j]->k<level->nodes[level->i]->k)
      {
        char *tmp;
        tmp = Mi; Mi = Mj; Mj = tmp;              
        tmp = Ri; Ri = Rj; Rj = tmp;        
        ki = level->nodes[level->j]->k;
        s1i = level->nodes[level->j]->s1;
        s2i = level->nodes[level->j]->s2;        
        kj = level->nodes[level->i]->k - ki;
        s1j = level->nodes[level->i]->s1;
        s2j = level->nodes[level->i]->s2;   
      }
      else
      {         
        ki = level->nodes[level->i]->k;
        s1i = level->nodes[level->i]->s1;
        s2i = level->nodes[level->i]->s2; 
        kj = level->nodes[level->j]->k - ki;
        s1j = level->nodes[level->j]->s1;
        s2j = level->nodes[level->j]->s2; 
      }
      
      generate_M(Mi, N, s1i, Ri, s2i, ki);
      generate_M(Mj, N, s1j, Rj, s2j, kj);
                  
      free(Mi);
      free(Mj);
      free(N);
      free(Ri);
      free(Rj);                        
    }
  }
  generate_leaves(level);
}

/*---------------------------------------------------------------------------*/
/* Genère les opérations qui se font via Booth                               */
/*---------------------------------------------------------------------------*/
void generate_leaves(level_t level)
{
  if(level->id_N == -1 && level->nodes[0]->gen_by_booth)
    generate_booth(level, 0);    
  else
  {
    if(level->id_N != -1 && level->nodes[level->id_N]->gen_by_booth)
      generate_booth(level, level->id_N);    
    if(level->id_Ri != -1 && level->nodes[level->id_Ri]->gen_by_booth)
      generate_booth(level, level->id_Ri);
    if(level->id_Rj != -1 && level->nodes[level->id_Rj]->gen_by_booth)
      generate_booth(level, level->id_Rj);    
  }
   
}

/*---------------------------------------------------------------------------*/
/* Génère le calcul de M = +- 2^k N +- R                                     */
/*---------------------------------------------------------------------------*/
void generate_M(char *M, char *N, int sN, char *R, int sR, int k)
{
  if(R)
  {
    if(k != 0)
      add_operation_int(SHF,N,N,k);
    switch(sN + 2*sR)
    {
      case 0: add_operation_str(ADD,M,R,N); break;
      case 1: add_operation_str(SUB,M,R,N); break;
      case 2: add_operation_str(SUB,M,N,R); break;                
      /* Pas de cas 3 car M est positif */
    }    
  }
  else
  {
    /* M est positif */
    if(k != 0)
      add_operation_int(SHF,M,N,k); 
    else
      add_operation_str(SET,M,NULL,N); 
  }
}

/*---------------------------------------------------------------------------*/
/* Colore le graphe pour assigner une variable aux calculs intermédiaires    */
/*---------------------------------------------------------------------------*/
int assign_vars(level_t level)
{
  int i;
  int color;
  int *colors;
  int nbr_colors = 0;
  
  if(level->next_level)
    nbr_colors = assign_vars(level->next_level);
  if(nbr_colors<level->nbr_nodes) nbr_colors = level->nbr_nodes;
  
  colors = malloc(sizeof(int) * nbr_colors);
  for(i=0; i<nbr_colors; i++)    
    colors[i] = 0;
  for(i=0; i<level->nbr_nodes; i++)
    if(level->nodes[i]->var_id != -1)
      colors[level->nodes[i]->var_id] = 1;
  
  /* On cherche la première couleur non utilisée */

  color = 0;
 
  /* On colore M_i et M_j */
  if(level->next_level)  /* si on n'est pas au dernier niveau */
  {
    /* si Mi != Mj */
    if(level->i != level->j)
    {
      /* Mi->k <= Mj->k (Mi calculé en premier) */
      if(level->nodes[level->i]->k<=level->nodes[level->j]->k)
      {
        /* S'il y a un reste Ri, Mi prend la couleur du reste */
        if(level->next_level->id_Ri != -1)
        {
          level->nodes[level->i]->var_id = level->next_level->nodes[level->next_level->id_Ri]->var_id;
          colors[level->nodes[level->i]->var_id] = 1;          
        }
        else /* Sinon, Mi prend une couleur différente de N */
        {
          while(colors[color]) color++;
          level->nodes[level->i]->var_id = color;
          color++;
        }        
        /* S'il y a un reste Rj, Mj prend la couleur du reste */
        if(level->next_level->id_Rj != -1)
        {
          level->nodes[level->j]->var_id = level->next_level->nodes[level->next_level->id_Rj]->var_id;
          colors[level->nodes[level->j]->var_id] = 1;          
        }
        else /* Sinon Mj prend la couleur de N */
          level->nodes[level->j]->var_id = level->next_level->nodes[level->next_level->id_N]->var_id;
      }
      else   /* Mi->k > Mj->k (Mj calculé en premier) */
      {
        /* S'il y a un reste Rj, Mj prend la couleur du reste */      
        if(level->next_level->id_Rj != -1)
        {
          level->nodes[level->j]->var_id = level->next_level->nodes[level->next_level->id_Rj]->var_id;
          colors[level->nodes[level->j]->var_id] = 1;          
        }
        else  /* Sinon, Mj prend une couleur différente de N */
        {
          while(colors[color]) color++;
          level->nodes[level->j]->var_id = color;
          color++;
        }      
        /* S'il y a un reste Ri, Mi prend la couleur du reste */  
        if(level->next_level->id_Ri != -1)
        {
          level->nodes[level->i]->var_id = level->next_level->nodes[level->next_level->id_Ri]->var_id;
          colors[level->nodes[level->i]->var_id] = 1;          
        }
        else   /* Sinon Mi prend la couleur de N */
          level->nodes[level->i]->var_id = level->next_level->nodes[level->next_level->id_N]->var_id;      
      }
    }
    else  /* Si les deux motifs sont dans Mi */
    {
      /* S'il y a un reste Ri, Mi prend la couleur de Ri */
      if(level->next_level->id_Ri != -1)
      {
        level->nodes[level->i]->var_id = level->next_level->nodes[level->next_level->id_Ri]->var_id;
        colors[level->nodes[level->i]->var_id] = 1;
      }
      else /* Sinon, Mi prend une couleur différentes de N */
      {
        int c;
        while(colors[color]) color++;
        c = color;
        if(c == level->next_level->nodes[level->next_level->id_N]->var_id) c++;
        level->nodes[level->i]->var_id = c;
        colors[level->nodes[level->i]->var_id] = 1;        
      }      
    }
  }
  
  /* On colore N, Ri et Rj */
  if(level->id_N != -1)  /* si on n'est pas au premier niveau */
  {
    
    if(level->id_Ri != -1 && level->nodes[level->id_Ri]->var_id == -1)
    {
      while(colors[color]) color++;
      level->nodes[level->id_Ri]->var_id = color;
      color++;      
    }
    if(level->id_Rj != -1 && level->nodes[level->id_Rj]->var_id == -1)
    {
      while(colors[color]) color++;
      level->nodes[level->id_Rj]->var_id = color;              
      color++;      
    }
    if(level->nodes[level->id_N]->var_id == -1)
    {
      while(colors[color]) color++;
      level->nodes[level->id_N]->var_id = color;
    }    
  }
  
  return color;
}

/*---------------------------------------------------------------------------*/
/* Echange la variable en paramètre                                          */
/*---------------------------------------------------------------------------*/
void swap_vars(level_t level, int var1, int var2)
{
  swap_vars_rec(level, var1, var2, 0);
}

/*---------------------------------------------------------------------------*/
/* Echange la variable en paramètre                                          */
/*---------------------------------------------------------------------------*/
void swap_vars_rec(level_t level, int var1, int var2, int i)
{
  for(; i<level->nbr_nodes; i++)
    if(level->nodes[i]->var_id == var1) level->nodes[i]->var_id = var2;
    else if(level->nodes[i]->var_id == var2) level->nodes[i]->var_id = var1;
    
  i = level->nbr_nodes;
  if(level->i == level->j)
    i-=1;
  else
    i-=2;  
  if(level->next_level)  
    swap_vars_rec(level->next_level, var1, var2,i);
}

/*---------------------------------------------------------------------------*/
/* Crée l'identificateur de la variable numéro i                             */
/*---------------------------------------------------------------------------*/
char* get_var(int var_id)
{
  char *var;
  int id_size;
  if(var_id == -1)
    return NULL;
  else if(var_id == 0)
  {
    id_size = strlen(label_var(1))+1;  /* variable res */
    var = malloc(sizeof(char) * id_size);
    sprintf(var,"%s",label_var(1));        
  }
  else
  {
    id_size = log(((double)var_id))/log(10.)+5;
    var = malloc(sizeof(char) * id_size);
    sprintf(var,"tmp%d",var_id);    
  }
  return var;
}
