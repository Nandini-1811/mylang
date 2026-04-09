#ifndef TOKENS_H
#define TOKENS_H


/*----------------------------------------
HelloWorld - Token Type Defination
STAGE 1 : SCANNER
All token types the scanner can emit
-------------------------------------------*/

typedef enum{
    /*-------LITERALS-----------*/
    TOKEN_NUMBER ,          /* 42, 0 , 1000*/
    TOKEN_DECIMAL_LIT,      /* 3.14, 199.90*/
    TOKEN_STRING,           /* "hello" */
    TOKEN_BOOL_LIT,         /* true/false */
    TOKEN_IDENT,            /* variable names */

    /*---------KEYWORDS---------*/
    TOKEN_LET,
    TOKEN_IS,
    TOKEN_DEFINE,
    TOKEN_RETURNS,
    TOKEN_GIVE,
    TOKEN_CHECK,
    TOKEN_OTHERWISE,
    TOKEN_REPEAT,
    TOKEN_EACH,
    TOKEN_IN,
    TOKEN_SHOW,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_FROM,
    TOKEN_OF,
    TOKEN_NOTHING,

    /*--------TYPES------------*/
    TOKEN_NUM,
    TOKEN_DECIMAL,
    TOKEN_TEXT,
    TOKEN_BOOL,


    /*-----OPERATORS----------*/
    TOKEN_ASSIGN,       /* = */
    TOKEN_EQUALS,       /* == */
    TOKEN_NOT_EQ,       /* != */
    TOKEN_LT,           /* < */
    TOKEN_GT,           /* > */
    TOKEN_LT_EQ,        /* <= */
    TOKEN_GT_EQ,        /* >= */
    TOKEN_PLUS,         /* + */
    TOKEN_MINUS,        /* - */
    TOKEN_STAR,         /* * */
    TOKEN_SLASH,        /* / */


    /*------PUNCTUATIONS------------*/
    TOKEN_LPAREN,        /* ( */
    TOKEN_RPAREN,        /* ) */
    TOKEN_LBRACE,        /* { */
    TOKEN_RBRACE,        /* } */
    TOKEN_LBRACKET,      /* [ */
    TOKEN_RBRACKET,      /* ] */
    TOKEN_COMMA,         /* , */
    TOKEN_SEMICOLON,     /* ; */

    /*------SPECIAL----------------*/
    TOKEN_NEWLINE,       /* \n */
    TOKEN_EOF,           /* end of file */
    TOKEN_ERROR,         /* unrecognized character */
}TokenType;


/*-------------------------------------------------------------------
Token struct -  what the scanner returns for every token it finds
---------------------------------------------------------------------*/

typedef struct{
    TokenType type;

    char* value;     /* actual text from source */
    int line;        /* line number in source */
    int column;      /* column number in source */
}Token;


/*------------------------------------------------------
Helper - get token name as a string used for debugging 
         and error messages
--------------------------------------------------------*/

const char* token_type_name(TokenType type);

#endif /* TOKENS */