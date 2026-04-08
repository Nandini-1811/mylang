#ifndef AST_H
#define AST_H

/*-----------------------------------------------
LANG - Abstart Syntax Tree Defination
Stage 2 : Parser
Every construct in the language maps 
to one of those node types.
-------------------------------------------------*/

/*----Node Types----*/

typedef enum{
    /*Statements*/
    NODE_PROGRAM ,                 /* root - list of statements*/
    NODE_VAR_DECL,                 /* let x is num = 10 */
    NODE_ASSIGN,                   /* x = 20 */
    NODE_FUNC_DECL,                /* define add(...) returns nums{} */
    NODE_RETURN,                   /* give x + y */
    NODE_IF,                       /* check(...) {} otherwise {} */
    NODE_REPEAT,                   /* repeat (...) {} */
    NODE_REPEAT_EACH,              /* repeat each item in list {} */
    NODE_SHOW,                     /* show(x) */
    NODE_BLOCK,                    /* {list of elements} */

    /*Expressions*/
    NODE_BINARY_EXPR,              /* a + b , x == 10 , x > y */
    NODE_UNARY_EXPR,               /* -x */
    NODE_CALL_EXPR,                /* add(10,20) */
    NODE_IDENTIFIER,               /* x, name, result */

    /* Literals */
    NODE_NUMBER_LIT,                /* 42 */
    NODE_DECIMAL_LIT,               /* 3.14 */
    NODE_STRING_LIT,                /* "hello" */
    NODE_BOOL_LIT,                  /* true/false */

} NodeType;


/* ----- Type enum -> mirrors your language types -----*/
typedef enum{
    TYPE_NUM,
    TYPE_DECIMAL,
    TYPE_TEXT,
    TYPE_BOOL,
    TYPE_NOTHING,
    TYPE_UNKNOWN,
} LangType;


/* forward declare so nodes can point to each other */
typedef struct  ASTNode ASTNode;


/* --- Parameter - for function declarations --- */
typedef struct {
    char* name;           /* parameter name */
    LangType type;        /* parameter type */
} Param;


/* The main AST Node struct */
struct ASTNode{
    NodeType type;

    union{
        /* NODE_PROGRAM - root of the tree */
        struct{
            ASTNode** statements ;         /* array of statement nodes */
            int count ;                    /* number of statements  */
        }program;

        /* NODE_VAR_DECL - let x is num = 10 */
        struct{
            char* name;            /* variable name */
            LangType var_type;     /* num/text/bool */
            ASTNode* value;        /* initial value */
        } var_decl;

        /* NODE_ASSIGN - x = 20 */
        struct {
            char* name;             /* variable name */
            ASTNode* value;         /* new value */
        } assign;

        /* NODE_FUNC_DECL - define add(a is num) returns num{} */
        struct {
            char * name;             /* function name */
            Param* params;           /* parameter list */
            int param_count;         /* number of params */
            LangType return_type;    /* return type */
            ASTNode* body;           /* block node */
        } func_decl;

        /* NODE_RETURN - give x + y */
        struct{
            ASTNode* value;          /* expression to return */
        }ret;

        /* NODE_IF - check (...) {} otherwise {} */
        struct{
            ASTNode* condition;        /* boolean expression */
            ASTNode* then_block;       /* check body */
            ASTNode* else_block;       /* otherwise body - NULL if absent */
        } if_stmt;

        /* NODE_REPEAT - repeat(let i is num=0;i<10;i++) {} */
        struct{
            ASTNode* init;
            ASTNode* condition;
            ASTNode* step;
            ASTNode* body;
        } repeat;

        /* NODE_REPEAT_EACH - repeat each item in scores {} */
        struct{
            char *  item_name;           /* loop variable name */
            ASTNode* collection;         /* the list/stack/map */
            ASTNode* body;               /* block */
        } repeat_each;

        /* NODE_SHOW - show(x) */
        struct{
            ASTNode* value;               /* expression to print */
        } show;

        /* NODE_BLOCK - {stmt1 stmt2 stmt3 }*/
        struct{
            ASTNode** statements;       /* array of statments */
            int count ;
        } block;

        /* NODE_BINARY_EXPR - a + b */
        struct{
            char* op;                      /* "+" ,"-" ,"==" ,"<=" */
            ASTNode* left;
            ASTNode* right;
        } binary;

        /* NODE_UNARY_EXPR -> -x */
        struct{
            char* op;                    /* "-" */
            ASTNode* operand;
        } unary;

        /* NODE_CALL_EXPR - add(10,20) */
        struct{
            char* name;                 /* function name */
            ASTNode** args;             /* argument list */
            int arg_count; 
        }call;

        /* NODE_IDENTIFIER - x */
        struct{
            char* name;
        } identifier;

        /* NODE_NUMBER_LIT - 42 */
        struct{
            int value;
        } number;

        /* NODE_DECIMAL_LIT - 3.14 */
        struct{
            double value;
        } decimal;

        /* NODE_STRING_LIT - "hello" */
        struct{
            char* value;
        } string;
        
        /* NODE_BOOL_LIT - true/false */
        struct{
            int value;               /* 1 = true , 0 = false */      
        } boolean;
    } as;                        /* "as" - read it as : node.as.var_decl.name */
};



/* AST NODE CONSTRUCTORS */
ASTNode* make_program(ASTNode** stmts, int count);
ASTNode* make_var_decl(char*name, LangType type, ASTNode* value);
ASTNode* make_assign(char* name, ASTNode* value);
ASTNode* make_func_decl(char*name , Param* params, int pc, LangType rt, ASTNode* body);
ASTNode* make_return(ASTNode* value);
ASTNode* make_if(ASTNode* cond, ASTNode* ten_b, ASTNode* else_b);
ASTNode* make_repeat(ASTNode* init, ASTNode* cond, ASTNode* step, ASTNode* body);
ASTNode* make_repeat_each(char* item,ASTNode* collection, ASTNode* body);
ASTNode* make_show(ASTNode* value);
ASTNode* make_block(ASTNode** stmts, int count);
ASTNode* make_binary(char* op,ASTNode* left, ASTNode* right);
ASTNode* make_unary(char* op,ASTNode* operand);
ASTNode* make_call(char* name,ASTNode** args, int count);
ASTNode* make_identifier(char* name);
ASTNode* make_number(int value);
ASTNode* make_decimal(double value);
ASTNode* make_string(char* value);
ASTNode* make_bool(int value);


/* Cleanup */
void free_ast(ASTNode* node);

/* Debig printer */
void print_ast(ASTNode * node, int indent);

/* LangType helper */
const char* type_name(LangType t);


#endif /* AST_H */