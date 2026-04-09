#ifndef SYMTABLE_H
#define SYMTABLE_H

/*------------------------------------
HelloWorld - Symbol Table 
Stage 3 : Semantic Analyzer

Tracks every variable and function 
declared in a HelloWorld program.
Supports nested scopes - each block
{} creates a new scope level. 
-------------------------------------*/

#include "ast.h"

/* A single symbol entry */

typedef struct Symbol{
    char* name;                  /* identifier name */
    LangType type;               /* its type */
    int is_func;                 /* 1 if function , 0 if var */
    int param_count;             /* for functions only */
    LangType* param_types;       /* for functions only */
    int scope_level;             /* which scope depth */
    struct Symbol* next;         /* linked list chain */
}Symbol;

/* The symbol table */
typedef struct {
    Symbol ** buckets;        /* hash table buckets */
    int size;                 /* number of buckets */
    int scope_level;          /* current scope depth */
}SymTable;


/* Functions */
SymTable* symtable_create(void);
void symtable_destroy(SymTable* st);

void symtable_enter_scope(SymTable* st);
void symtable_exit_scope(SymTable* st);

int symtable_add_var(SymTable* st,char* name,LangType type);
int symtable_add_func(SymTable* st,char* name,LangType return_type,int param_count,LangType* param_types);
Symbol* symtable_lookup(SymTable* st,char* name);
Symbol* symtable_lookup_current_scope(SymTable* st,char* name);

void symtable_print(SymTable* st);

#endif /* SYMTABLE_H */