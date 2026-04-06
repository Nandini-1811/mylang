#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "../include/tokens.h"

/*forward declerations*/
typedef struct{
    const char* source;
    int pos, line, column, length;
}Scanner;

void scanner_init(Scanner* s, const char* source);
Token next_token(Scanner* s);

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
    char* source = read_file(argv[1]);
    Scanner s;
    scanner_init(&s,source);
    printf("%-16s %-20s %s\n","TOKEN TYPE","VALUE","LINE:COL");
    printf("--------------------------------------------------\n");
    Token t;
    do{
        t = next_token(&s);
        /*skip newlines in output - too noisy*/
        if(t.type == TOKEN_NEWLINE){
            free(t.value);
            continue;
        }
        printf("%-16s %-20s %d:%d\n",
            token_type_name(t.type),
            t.value,t.line,t.column);
        
        free(t.value);
    }while(t.type != TOKEN_EOF);

    free(source);
    return 0;
}