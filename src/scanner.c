#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include"../include/tokens.h"

/*------------------------------
HelloWorld Scanner 
Stage1 - Lexical Analysis 
Reads a .learn source file character 
by character and emits a stream of 
Token struct.
--------------------------------*/

/*---------------------------------
KEYWORD TABLE
When we scan an identifier we check it 
against the table.
If it matches we emit the keyword 
token instead of IDENT. 
-----------------------------------*/

typedef struct{
    const char* word;
    TokenType type;
}Keyword;


static Keyword keyword_table[] = {
    {"let", TOKEN_LET},
    {"is", TOKEN_IS},
    {"define", TOKEN_DEFINE},
    {"returns", TOKEN_RETURNS},
    {"give", TOKEN_GIVE},
    {"check", TOKEN_CHECK},
    {"otherwise", TOKEN_OTHERWISE},
    {"repeat", TOKEN_REPEAT},
    {"each", TOKEN_EACH},
    {"in", TOKEN_IN},
    {"show", TOKEN_SHOW},
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {"from", TOKEN_FROM},
    {"of", TOKEN_OF},
    {"nothing", TOKEN_NOTHING},
    {"num", TOKEN_NUM},
    {"decimal", TOKEN_DECIMAL},
    {"text", TOKEN_TEXT},
    {"bool", TOKEN_BOOL},
    {NULL, TOKEN_ERROR}
};


/*----------------------------------
SCANNER STATE
Everything the scanner needs to track 
as it walks through the source file.
------------------------------------*/

typedef struct{
    const char* source;
    int pos;
    int length;
    int line;
    int column;
}Scanner;


/*----------------------------------
SCANNER INIT
------------------------------------*/

void scanner_init(Scanner* s, const char* source){
    s->source = source;
    s->length = strlen(source);
    s->pos = 0;
    s->line = 1;
    s->column = 1;
}

/*---------------------------------
TOKEN TYPE NAME
Returns the token type as a readable
string - used in debug output and errors.
-----------------------------------*/

const char* token_type_name(TokenType type){
    switch(type){
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_DECIMAL_LIT: return "DECIMAL_LIT";
        case TOKEN_STRING: return "STRING";
        case TOKEN_BOOL_LIT: return "BOOL_LIT";
        case TOKEN_IDENT: return "IDENT";
        case TOKEN_LET: return "LET";
        case TOKEN_IS: return "IS";
        case TOKEN_DEFINE: return "DEFINE";
        case TOKEN_RETURNS: return "RETURNS";
        case TOKEN_GIVE: return "GIVE";
        case TOKEN_CHECK: return "CHECK";
        case TOKEN_OTHERWISE: return "OTHERWISE";
        case TOKEN_REPEAT: return "REPEAT";
        case TOKEN_EACH: return "EACH";
        case TOKEN_IN: return "IN";
        case TOKEN_SHOW: return "SHOW";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_FROM: return "FROM";
        case TOKEN_OF: return "OF";
        case TOKEN_NOTHING: return "NOTHING";
        case TOKEN_NUM: return "NUM";
        case TOKEN_DECIMAL: return "DECIMAL";
        case TOKEN_TEXT: return "TEXT";
        case TOKEN_BOOL: return "BOOL";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_EQUALS: return "EQUALS";
        case TOKEN_NOT_EQ: return "NOT_EQ";
        case TOKEN_LT: return "LT";
        case TOKEN_GT: return "GT";
        case TOKEN_LT_EQ: return "LT_EQ";
        case TOKEN_GT_EQ: return "GT_EQ";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_NEWLINE: return "NEWLINE";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

/*----------------------------------
SCANNER HELPERS
------------------------------------*/

/* look at the current character without consuming*/
static char peek(Scanner* s){
    if(s->pos >= s->length)
        return '\0';
    return s->source[s->pos];
}

/* look one chracter ahead without consuming */
static char peek_next(Scanner* s){
    if(s->pos >= s->length)
        return '\0';
    return s->source[s->pos + 1];
}

/* consume current character and advance */
static char advance(Scanner*s){
    char c = s->source[s->pos];
    s->pos++;
    if(c == '\n'){
        s->line++;
        s->column = 1;
    }
    else{
        s->column++;
    }
    return c;
}

/* build a Token - copies the substring from source */
static Token make_token(Scanner* s, TokenType type, int start, int length, int line , int col){
    Token t;
    t.type = type;
    t.line = line;
    t.column = col;

    /*copy the exact text from source*/
    t.value = (char*)malloc(length+1);
    strncpy(t.value, s->source+start, length);
    t.value[length] = '\0';
    return t;
}

/* check identifier against keyword table */
static TokenType check_keyword(const char* word){
    for(int i=0;keyword_table[i].word != NULL;i++){
        if(strcmp(word,keyword_table[i].word) == 0){
            return keyword_table[i].type;
        }
    }
    return TOKEN_IDENT;
}


/*-----------------------------------------------
NEXT TOKEN
The core scanning function.
Call this repeatedly to get one token
at a time from the source code. 
--------------------------------------------------*/

Token next_token(Scanner * s){
    /*skip spaces and tabs - but not newlines */
    while(peek(s) == ' ' || peek(s) == '\t'){
        advance(s);
    }

    int start = s->pos;
    int line = s->line;
    int col = s->column;

    /* if we hit end of source emit EOF*/
    if(peek(s) == '\0'){
        return make_token(s,TOKEN_EOF,start,0,line,col);
    }

    char c = advance(s);

    /*NEWLINE - statement terminator*/
    if(c == '\n'){
        return make_token(s,TOKEN_NEWLINE,start,1,line,col);
    }

    /*COMMENT - note: skips the rest of line*/
    if(c == 'n' && (start+4) < s->length && strncmp(s->source + start , "note:", 5) == 0){
        /*consume rest of "ote:" (4 chars - 'n' was already consumed)*/
        advance(s); advance(s); advance(s); advance(s);
        /*skip everything until newline*/
        while(peek(s) != '\n'  && peek(s) != '\0'){
            advance(s);
        }

        /*call next_token again to get the real next token*/
        return next_token(s);
    }

    /*IDENTIFIERS AND KEYWORDS*/
    if(isalpha(c) || c == '_'){
        while(isalnum(peek(s)) || peek(s) == '_'){
            advance(s);
        }
        int len = s->pos - start;
        char* word = (char*)malloc(len+1);
        strncpy(word, s->source + start, len);
        word[len] = '\0';

        TokenType type = check_keyword(word);
        free(word);
        return make_token(s, type, start, len, line, col);
    }

    /*NUMBER LITERALS*/
    if(isdigit(c)){
        while(isdigit(peek(s))){
            advance(s);
        }
        /*check for decimal*/
        if(peek(s) == '.' && isdigit(peek_next(s))){
            advance(s);
            while(isdigit(peek(s))){
                advance(s);
            }
            return make_token(s, TOKEN_DECIMAL_LIT, start, s->pos - start, line, col);
        }
        return make_token(s, TOKEN_NUMBER, start, s->pos - start, line, col);
    }

    /*STRING LITERAL*/
    if(c == '"'){
        while(peek(s) != '"' && peek(s) != '\0'){
            advance(s);
        }
        if(peek(s) == '"'){
            advance(s);
        }
        return make_token(s, TOKEN_STRING, start, s->pos - start, line, col);
    }

    /*TWO CHARACTER OPERATORS */
    /*MUST CHECK THESE BEFORE SINGLE CHARACTER OPERATOR*/
    if(c == '=' && peek(s) == '='){
        advance(s);
        return make_token(s,TOKEN_EQUALS,start,2,line,col);
    }
    if(c == '!' && peek(s) == '='){
        advance(s);
        return make_token(s,TOKEN_NOT_EQ,start,2,line,col);
    }
    if(c == '<' && peek(s) == '='){
        advance(s);
        return make_token(s,TOKEN_LT_EQ,start,2,line,col);
    }
    if(c == '>' && peek(s) == '='){
        advance(s);
        return make_token(s,TOKEN_GT_EQ,start,2,line,col);
    }

    /*SINGLE CHARACTER TOKEN*/
    switch(c){
        case '(': return make_token(s,TOKEN_LPAREN,start,1,line,col);
        case ')': return make_token(s,TOKEN_RPAREN,start,1,line,col);
        case '{': return make_token(s,TOKEN_LBRACE,start,1,line,col);
        case '}': return make_token(s,TOKEN_RBRACE,start,1,line,col);
        case '[': return make_token(s,TOKEN_LBRACKET,start,1,line,col);
        case ']': return make_token(s,TOKEN_RBRACKET,start,1,line,col);
        case ',': return make_token(s,TOKEN_COMMA,start,1,line,col);
        case ';': return make_token(s,TOKEN_SEMICOLON,start,1,line,col);
        case '+': return make_token(s,TOKEN_PLUS,start,1,line,col);
        case '-': return make_token(s,TOKEN_MINUS,start,1,line,col);
        case '*': return make_token(s,TOKEN_STAR,start,1,line,col);
        case '/': return make_token(s,TOKEN_SLASH,start,1,line,col);
        case '=': return make_token(s,TOKEN_ASSIGN,start,1,line,col);
        case '<': return make_token(s,TOKEN_LT,start,1,line,col);
        case '>': return make_token(s,TOKEN_GT,start,1,line,col);
    }

    /* UNKNOWN CHARACTER */
    return make_token(s,TOKEN_ERROR,start,1,line,col);
}