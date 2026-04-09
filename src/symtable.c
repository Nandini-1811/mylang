#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "../include/symtable.h"

/*-----------------------------------------
HelloWorld Symbol Table Implementation
Uses a hash table with chaining.
Each symbol stores its scope level so 
we can remove it when scope exits.
------------------------------------------- */

#define TABLE_SIZE 64

/* Hash Function */
static int hash(const char* name , int size){
    unsigned int h = 0;
    while(*name){
        h = h* 31 + (unsigned char)*name;
        name++;
    }
    return h% size;
}


/* create a new symbol table */
SymTable* symtable_create(void){
    SymTable* st = (SymTable*)calloc(1,sizeof(SymTable));
    st->size = TABLE_SIZE;
    st->buckets = (Symbol**)calloc(TABLE_SIZE,sizeof(Symbol*));
    st->scope_level = 0;
    return st;
}

/* Enter a new scope */
void symtable_enter_scope(SymTable* st){
    st->scope_level++;
}

/* Exit scope remove all symbols at this level */
void symtable_exit_scope(SymTable* st){
    for(int i=0;i<st->size;i++){
        Symbol** ptr = &st->buckets[i];
        while(*ptr){
            if((*ptr)->scope_level == st->scope_level){
                Symbol* to_free = * ptr;
                *ptr = to_free->next;
                free(to_free->name);
                if(to_free->param_types) free(to_free->param_types);
                free(to_free);
            }
            else{
                ptr = &(*ptr)->next;
            }
        }
    }
    st->scope_level--;
}


/* Add a variable */
int symtable_add_var(SymTable* st, char* name , LangType type){
    /* check for duplicates in current scope */
    if(symtable_lookup_current_scope(st,name)){
        return 0; 
    }
    int idx = hash(name,st->size);
    Symbol* sym = (Symbol*)calloc(1,sizeof(Symbol));
    sym->name = strdup(name);
    sym->type = type;
    sym->is_func = 0;
    sym->scope_level = st->scope_level;
    sym->next = st->buckets[idx];
    st->buckets[idx] = sym;
    return 1; 
}

/* Add a function */
int symtable_add_func(SymTable* st, char* name,LangType return_type,int param_count,LangType* param_types){
    if(symtable_lookup_current_scope(st,name)){
        return 0;
    }
    int idx = hash(name,st->size);
    Symbol* sym = (Symbol*)calloc(1,sizeof(Symbol));
    sym->name = strdup(name);
    sym->type = return_type;
    sym->is_func = 1;
    sym->param_count = param_count;
    sym->param_types = (LangType*)malloc(sizeof(LangType) * param_count);
    memcpy(sym->param_types,param_types,sizeof(LangType)* param_count);
    sym->scope_level = st->scope_level;
    sym->next = st->buckets[idx];
    st->buckets[idx] = sym;
    return 1; 
}


/* Lookup - searches all scopes */
Symbol* symtable_lookup(SymTable* st,char* name){
    int idx = hash(name,st->size);
    Symbol* sym = st->buckets[idx];
    while(sym){
        if(strcmp(sym->name, name) == 0) return sym;
        sym = sym->next;
    }
    return NULL;
}


/* Lookup current scope only */
Symbol* symtable_lookup_current_scope(SymTable* st, char* name){
    int idx = hash(name,st->size);
    Symbol* sym = st->buckets[idx];
    while(sym){
        if(strcmp(sym->name, name) == 0 && sym->scope_level == st->scope_level) return sym;
        sym = sym->next;
    }
    return NULL;
}


/* Debug Printer */
void symtable_print(SymTable* st){
    printf("\n=== Symbol Table ===\n");
    printf("%-20s %-10s %-8s %s\n","Name","Type","Kind","Scope");
    printf("----------------------------------------------\n");
    for(int i=0;i<st->size;i++){
        Symbol * sym = st->buckets[i];
        while(sym){
            printf("%-20s %-10s %-8s %d\n",
                sym->name,
                type_name(sym->type),
                sym->is_func?"func" : "var",
                sym->scope_level);
            sym = sym->next;
        }
    }
}


/* Destroy */
void symtable_destroy(SymTable* st){
    for(int i=0;i<st->size;i++){
        Symbol * sym = st->buckets[i];
        while(sym){
            Symbol* next = sym->next;
            free(sym->name);
            if(sym->param_types) free(sym->param_types);
            free(sym);
            sym = next;
        }
    }
    free(st->buckets);
    free(st);
}