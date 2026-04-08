#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "../include/tokens.h"
#include "../include/ast.h"

/*forward declerations*/
typedef struct{
    const char* source;
    int pos, line, column, length;
}Scanner;

void scanner_init(Scanner* s, const char* source);
Token next_token(Scanner* s);
ASTNode* parse(Token* tokens, int count);

/* read entire file into a string */
char* read_file(const char* path){
    FILE* f = fopen(path,"r");
    if(!f){
        fprintf(stderr,"ERROR: cannot open file '%s'\n",path);
        exit(1);
    }
    fseek(f,0,SEEK_END);
    long size = ftell(f);
    rewind(f);

    char* buffer = (char*)malloc(size+1);
    fread(buffer,1,size,f);
    buffer[size] = '\0';
    fclose(f);
    return buffer;
}

/* main */
int main(int argc, char* argv[]){
    if(argc < 2){
        fprintf(stderr,"Usage: lang <file.learn>\n");
        return 1;
    }
    /* Stage 1 - SCAN */
    char* source = read_file(argv[1]);
    Scanner s;
    scanner_init(&s,source);
    
    Token* tokens = (Token*)malloc(sizeof(Token) * 4096);
    int count = 0;
    Token t;
    do{
        t = next_token(&s);
        tokens[count++] = t;
    }while(t.type != TOKEN_EOF);

    /* Stage 2 : Parse */
    ASTNode* ast = parse(tokens,count);

    /* PRINT AST */
    printf("\n=== AST ===\n");
    print_ast(ast,0);

    free_ast(ast);
    free(tokens);
    return 0;
}
