#ifndef GENERATEH
#define GENERATEH

#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {EMPTY, ADD, SUB, SHF, MULUI, SET, SETUI, ADDUI, SUBUI};

/* Initialise/libère les données */
void init_data_generate(const char* res, const char* x);
void clear_data_generate();

/* Génère le fichier pour la multiplication par M */
void generate(FILE *File, mpz_t M, const char *algo);

/* ajoute une variable temporaire si elle n'existe pas déjà s*/
void add_variable(char *var);

/* effectue l'opération op1 <- op2 [type] op3 */
void add_operation_str(int type, const char *op1, const char *op2, const char *op3);
void add_operation_int(int type, const char *op1, const char *op2, unsigned long op3);

/* renvoie la variable numéro n */
char* label_var(int n);

/* ajoute un commentaire */
void add_comment(char* comment);
#endif
