#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "../include/tokens.h"
#include "../include/ast.h"


/*------------------------------------------
LANG PARSER 
Stage 2 - Syntax Analysis 
Takes the token stream from the scanner
and builds an AST.

Strategy : Recursive Descent Parser
Each Grammar rule becomes one function.
--------------------------------------------*/


/*--------------------------------------------
PARSER STATE 
----------------------------------------------*/

typedef struct{
    Token* tokens;       /* full token array from scanner*/        
    int pos;             /* current position  in token array */
    int count;           /* total number of tokens */
    int had_error;       /* 1 if any error occured */
}Parser;



/*-----------------------------------------
PARSER HELPERS 
-------------------------------------------*/

/* look at current token without consuming */
static Token peek_tok(Parser* p){
    return p->tokens[p->pos];
}

/* look at next token without consuming */
static Token peek_next_tok(Parser* p){
    if(p->pos + 1 >= p->count){
        return p->tokens[p->count-1];  /* return EOF*/
    }
    return p->tokens[p->pos + 1];
}


/*check if current token matches type*/
static int check(Parser* p,TokenType type){
    return peek_tok(p).type == type;
}

/* consume current token and advance */
static Token advance_tok(Parser* p){
    Token t = p->tokens[p->pos];
    if(p->pos < p->count -1) p->pos ++;
    return t;
}

/* skip all newline tokens */
static void skip_newlines(Parser* p){
    while(check(p,TOKEN_NEWLINE)){
        advance_tok(p);
    }
}

/* consume token if it matches - return 1 if matched */
static int match(Parser* p , TokenType type){
    if(check(p,TOKEN_NEWLINE) && type!= TOKEN_NEWLINE){
        skip_newlines(p);
    }
    if(check(p,type)){
        advance_tok(p);
        return 1;
    }
    return 0;
}


/* consume token or report error */
static Token expect(Parser* p , TokenType type){
    skip_newlines(p);
    if(check(p,type)){
        return advance_tok(p);
    }
    /*error -  what we got vs what we expected */
    Token got = peek_tok(p);
    fprintf(stderr,
        "Parser error at line %d col %d : expected %s but got %s ('%s')\n",
        got.line,got.column,
        token_type_name(type),
        token_type_name(got.type),
        got.value
    );
    p->had_error = 1;
    return got;
}


/* check if current token is a type keyword */
static int is_type_token(Parser* p){
    TokenType t = peek_tok(p).type;
    return t == TOKEN_NUM || 
            t == TOKEN_DECIMAL || 
            t == TOKEN_BOOL || 
            t == TOKEN_TEXT || 
            t == TOKEN_NOTHING;
}


/* convert token type to LangType */
static LangType token_to_langtype(TokenType t){
    switch(t){
        case TOKEN_NUM: return TYPE_NUM;
        case TOKEN_DECIMAL: return TYPE_DECIMAL;
        case TOKEN_TEXT: return TYPE_TEXT;
        case TOKEN_BOOL: return TYPE_BOOL;
        case TOKEN_NOTHING: return TYPE_NOTHING;
        default: return TYPE_UNKNOWN;
    }
}

/*-------------------------------------------
AST NODE CONSTRUCTORS
---------------------------------------------*/

static ASTNode* alloc_node(NodeType type){
    ASTNode* n = (ASTNode*)calloc(1,sizeof(ASTNode));
    n->type = type;
    return n;
}

ASTNode* make_program(ASTNode** stmts, int count){
    ASTNode* n = alloc_node(NODE_PROGRAM);
    n->as.program.statements = stmts;
    n->as.program.count = count;
    return n;
}

ASTNode* make_var_decl(char* name, LangType type, ASTNode* value){
    ASTNode* n = alloc_node(NODE_VAR_DECL);
    n->as.var_decl.name = name;
    n->as.var_decl.var_type = type;
    n->as.var_decl.value = value;
    return n;
}

ASTNode* make_func_decl(char* name, Param* params, int pc, LangType rt, ASTNode* body){
    ASTNode* n = alloc_node(NODE_FUNC_DECL);
    n->as.func_decl.name = name;
    n->as.func_decl.return_type = rt;
    n->as.func_decl.params = params;
    n->as.func_decl.param_count = pc;
    n->as.func_decl.body = body;
    return n;
}

ASTNode* make_return(ASTNode* value){
    ASTNode* n = alloc_node(NODE_RETURN);
    n->as.ret.value = value;
    return n;
}

ASTNode* make_if(ASTNode* cond, ASTNode* then_b, ASTNode* else_b){
    ASTNode* n  = alloc_node(NODE_IF);
    n->as.if_stmt.condition = cond;
    n->as.if_stmt.then_block = then_b;
    n->as.if_stmt.else_block = else_b;
    return n;
}

ASTNode* make_repeat(ASTNode* init, ASTNode* cond,ASTNode* step, ASTNode* body){
    ASTNode* n = alloc_node(NODE_REPEAT);
    n->as.repeat.init = init;
    n->as.repeat.condition = cond;
    n->as.repeat.step = step;
    n->as.repeat.body = body;
    return n;
}

ASTNode* make_repeat_each(char* item, ASTNode* col, ASTNode* body){
    ASTNode* n = alloc_node(NODE_REPEAT_EACH);
    n->as.repeat_each.item_name = item;
    n->as.repeat_each.collection = col;
    n->as.repeat_each.body = body;
    return n;
}

ASTNode* make_show(ASTNode* value){
    ASTNode* n = alloc_node(NODE_SHOW);
    n->as.show.value = value;
    return n;
}

ASTNode* make_block(ASTNode** stmts, int count){
    ASTNode* n = alloc_node(NODE_BLOCK);
    n->as.block.statements = stmts;
    n->as.block.count = count;
    return n;
}

ASTNode* make_binary(char* op, ASTNode* left, ASTNode* right){
    ASTNode* n = alloc_node(NODE_BINARY_EXPR);
    n->as.binary.op = strdup(op);
    n->as.binary.left = left;
    n->as.binary.right = right;
    return n;
}

ASTNode* make_unary(char* op, ASTNode* operand){
    ASTNode* n = alloc_node(NODE_UNARY_EXPR);
    n->as.unary.op = strdup(op);
    n->as.unary.operand = operand;
    return n;
}

ASTNode* make_call(char* name, ASTNode** args, int count){
    ASTNode* n = alloc_node(NODE_CALL_EXPR);
    n->as.call.name = strdup(name);
    n->as.call.args = args;
    n->as.call.arg_count = count;
    return n;
}

ASTNode* make_identifier(char* name){
    ASTNode* n = alloc_node(NODE_IDENTIFIER);
    n->as.identifier.name = strdup(name);
    return n;
}

ASTNode* make_number(int value){
    ASTNode* n = alloc_node(NODE_NUMBER_LIT);
    n->as.number.value = value;
    return n;
}

ASTNode* make_decimal(double value){
    ASTNode* n = alloc_node(NODE_DECIMAL_LIT);
    n->as.decimal.value = value;
    return n;
}

ASTNode* make_string(char* value){
    ASTNode* n = alloc_node(NODE_STRING_LIT);
    n->as.string.value = strdup(value);
    return n;
}

ASTNode* make_bool(int value){
    ASTNode* n = alloc_node(NODE_BOOL_LIT);
    n->as.boolean.value = value;
    return n;
}

ASTNode* make_assign(char* name, ASTNode* value){
    ASTNode* n = alloc_node(NODE_ASSIGN);
    n->as.assign.name = name;
    n->as.assign.value = value;
    return n ;
}

const char* type_name(LangType t){
    switch(t){
        case TYPE_NUM: return "num";
        case TYPE_DECIMAL :return "decimal";
        case TYPE_TEXT: return "text";
        case TYPE_BOOL: return "bool";
        case TYPE_NOTHING: return "nothing";
        default: return "unknown";
    }
}


/*--------------------------------------
FORWARD DECLERATIONS
(functions call each other recursively)
----------------------------------------*/

static ASTNode* parse_statement(Parser* p);
static ASTNode* parse_expression(Parser* p);
static ASTNode* parse_block(Parser* p);


/*-----------------------------------------
EXPRESSION PARSING 
Precedence layers - lowest to highest:
1.comparsion  == != < > <= >= 
2.addition    + -
3.multiply    * /
4. unary      -x
5. primary    literals, identifiers, ()
-------------------------------------------*/

static ASTNode* parse_primary(Parser* p){
    skip_newlines(p);
    Token t = peek_tok(p);

    /* number literals */
    if(t.type == TOKEN_NUMBER){
        advance_tok(p);
        return make_number(atoi(t.value));
    }

    /* decimal literal */
    if(t.type == TOKEN_DECIMAL_LIT){
        advance_tok(p);
        return make_decimal(atof(t.value));
    }

    /*string literal */
    if(t.type == TOKEN_STRING){
        advance_tok(p);
        /*strip surrounding quotes*/
        int len = strlen(t.value);
        char* val = (char*)malloc(len-1);
        strncpy(val,t.value+1,len-2);
        val[len-2] = '\0';
        ASTNode * n = make_string(val);
        free(val);
        return n;
    }

    /* boolean literals */
    if(t.type == TOKEN_TRUE){
        advance_tok(p);
        return make_bool(1);
    }
    if(t.type == TOKEN_FALSE){
        advance_tok(p);
        return make_bool(0);
    }

    /* identifiers or function calls */
    if(t.type == TOKEN_IDENT){
        advance_tok(p);

        /*check if its a function call */
        if(check(p,TOKEN_LPAREN)){
            advance_tok(p);

            /* parse arguments */
            ASTNode** args = NULL;
            int arg_count = 0;

            skip_newlines(p);
            if(!check(p,TOKEN_RPAREN)){
                args = (ASTNode**)malloc(sizeof(ASTNode*)*32);
                do{
                    skip_newlines(p);
                    args[arg_count++] = parse_expression(p);
                    skip_newlines(p);
                }while(match(p,TOKEN_COMMA));
            }
            expect(p,TOKEN_RPAREN);
            return make_call(t.value,args,arg_count);
        }
        return make_identifier(t.value);
    }

    /* parenthesized expression */
    if(t.type == TOKEN_LPAREN){
        advance_tok(p);
        ASTNode* expr = parse_expression(p);
        expect(p,TOKEN_RPAREN);
        return expr;
    }

    /* error */
    fprintf(stderr,
    "Parse error at line %d : unexpexted token '%s' \n",
    t.line,t.value);
    p->had_error = 1;
    advance_tok(p);
    return make_number(0);         /* dummy node to continue parsing */
}

static ASTNode* parse_unary(Parser* p){
    if(check(p,TOKEN_MINUS)){
        advance_tok(p);
        ASTNode* operand = parse_unary(p);
        return make_unary("-",operand);
    }
    return parse_primary(p);
}

static ASTNode* parse_multiply(Parser* p){
    ASTNode* left = parse_unary(p);
    while(check(p,TOKEN_STAR) || check(p,TOKEN_SLASH)){
        Token op = advance_tok(p);
        ASTNode* right = parse_unary(p);
        left = make_binary(op.value,left,right);
    }
    return left;
}

static ASTNode* parse_addition(Parser* p){
    ASTNode* left = parse_multiply(p);
    while(check(p,TOKEN_PLUS) || check(p,TOKEN_MINUS)){
        Token op = advance_tok(p);
        ASTNode* right = parse_multiply(p);
        left = make_binary(op.value,left,right);
    }
    return left;
}


static ASTNode* parse_comparison(Parser* p){
    ASTNode* left = parse_addition(p);
    while(check(p,TOKEN_EQUALS) ||
            check(p,TOKEN_NOT_EQ) || 
            check(p,TOKEN_LT) ||
            check(p,TOKEN_GT) ||
            check(p,TOKEN_LT_EQ) ||
            check(p,TOKEN_GT_EQ)){
                Token op = advance_tok(p);
                ASTNode* right = parse_addition(p);
                left = make_binary(op.value,left,right);
            }
    return left;
}

static ASTNode* parse_expression(Parser* p){
    return parse_comparison(p);
}



/*---------------------------------------------
STATEMENT PARSING 
----------------------------------------------*/

/*  parse a tyoe keyword and return LangType */
static LangType parse_type(Parser* p){
    skip_newlines(p);
    if(is_type_token(p)){
        Token t = advance_tok(p);
        return token_to_langtype(t.type);
    }

    fprintf(stderr,
        "Parser error at line %d: expected a type\n",
        peek_tok(p).line);
        p->had_error = 1;
        return TYPE_UNKNOWN;
}

/* parse {statements} */
static ASTNode* parse_block(Parser* p){
    expect(p,TOKEN_LBRACE);
    skip_newlines(p);

    ASTNode** stmts = (ASTNode**)malloc(sizeof(ASTNode*) * 256);
    int count = 0 ;

    while(!check(p,TOKEN_RBRACE) && !check(p,TOKEN_EOF)){
        skip_newlines(p);
        if(check(p,TOKEN_RBRACE)) break;
        stmts[count++] = parse_statement(p);
        skip_newlines(p);
    }
    expect(p,TOKEN_RBRACE);
    return make_block(stmts,count);
}

/* parse :  let x is num = 10 */
static ASTNode* parse_var_decl(Parser* p){
    expect(p,TOKEN_LET);
    Token name = expect(p,TOKEN_IDENT);
    expect(p,TOKEN_IS);
    LangType type = parse_type(p);
    expect(p,TOKEN_ASSIGN);
    ASTNode* value = parse_expression(p);
    return make_var_decl(strdup(name.value),type,value);
}

/* parse : define add(a is num , b is num ) returns num {} */
static ASTNode* parse_func_decl(Parser* p){
    expect(p,TOKEN_DEFINE);
    Token name = expect(p,TOKEN_IDENT);

    expect(p,TOKEN_LPAREN);

    /* parse parameters */
    Param * params = (Param*)malloc(sizeof(Param) * 32);
    int param_count = 0;

    skip_newlines(p);
    if(!check(p,TOKEN_RPAREN)){
        do{
            skip_newlines(p);
            Token pname = expect(p,TOKEN_IDENT);
            expect(p,TOKEN_IS);
            LangType ptype =  parse_type(p);
            params[param_count].name = strdup(pname.value);
            params[param_count].type = ptype;
            param_count++;
            skip_newlines(p);
        }while(match(p,TOKEN_COMMA));
    }
    expect(p,TOKEN_RPAREN);
    expect(p,TOKEN_RETURNS);
    LangType ret_type = parse_type(p);
    ASTNode* body = parse_block(p);

    return make_func_decl(strdup(name.value),params,param_count,ret_type,body);
}


/* parse check(...) {} otherwise{} */
static ASTNode* parse_if(Parser* p){
    expect(p,TOKEN_CHECK);
    expect(p,TOKEN_LPAREN);
    ASTNode* condition = parse_expression(p);
    expect(p,TOKEN_RPAREN);
    ASTNode* then_block = parse_block(p);
    ASTNode* else_block = NULL;

    skip_newlines(p);
    if(check(p,TOKEN_OTHERWISE)){
        advance_tok(p);
        else_block = parse_block(p);
    }

    return make_if(condition,then_block,else_block);
}

/* parse : repeat(...) {} or repeat each item in list {} */

static ASTNode* parse_repeat(Parser* p){
    expect(p,TOKEN_REPEAT);

    /* REPEAT EACH ITEM IN COLLECTION */
    if(check(p,TOKEN_EACH)){
        advance_tok(p);
        Token item = expect(p,TOKEN_IDENT);
        expect(p,TOKEN_IN);
        ASTNode* collection = parse_expression(p);
        ASTNode* body = parse_block(p);
        return make_repeat_each(strdup(item.value),collection,body);
    }

    /* repeat (init ; condition ; step) {} */
    expect(p,TOKEN_LPAREN);
    ASTNode* init = parse_var_decl(p);
    expect(p,TOKEN_SEMICOLON);
    ASTNode* condition = parse_expression(p);
    expect(p,TOKEN_SEMICOLON);

    /* parse i++ as increment step */
    Token step_var = expect(p,TOKEN_IDENT);
    expect(p,TOKEN_PLUS);
    expect(p,TOKEN_PLUS);

    /* represent i++ as i = i+ 1 */
    ASTNode* step = make_assign(
        strdup(step_var.value),
        make_binary("+",
            make_identifier(strdup(step_var.value)),
            make_number(1)
        )
    );

    expect(p,TOKEN_RPAREN);
    ASTNode* body = parse_block(p);
    return make_repeat(init,condition,step,body);
}


/* parse show(x) */
static ASTNode* parse_show(Parser* p){
    expect(p,TOKEN_SHOW);
    expect(p,TOKEN_LPAREN);
    ASTNode* value = parse_expression(p);
    expect(p,TOKEN_RPAREN);
    return make_show(value);
}


/* parse any statement */
static ASTNode* parse_statement(Parser* p){
    skip_newlines(p);
    Token t = peek_tok(p);

    switch (t.type){
        case TOKEN_LET: return parse_var_decl(p);
        case TOKEN_DEFINE: return parse_func_decl(p);
        case TOKEN_CHECK: return parse_if(p);
        case TOKEN_REPEAT: return parse_repeat(p);
        case TOKEN_SHOW: return parse_show(p);
        case TOKEN_GIVE:{
            advance_tok(p);
            ASTNode* value = parse_expression(p);
            return make_return(value);
        }

        case TOKEN_IDENT:{
            /* could be assignment x = 20 or call add(1,2) */
            Token name = advance_tok(p);
            if(check(p,TOKEN_ASSIGN)){
                advance_tok(p);
                ASTNode* value = parse_expression(p);
                return make_assign(strdup(name.value),value);
            }

            /* function call as assignment */
            if(check(p,TOKEN_LPAREN)){
                advance_tok(p);
                ASTNode** args = NULL;
                int arg_count = 0;
                skip_newlines(p);
                if(!check(p,TOKEN_RPAREN)){
                    args = (ASTNode**)malloc(sizeof(ASTNode*)*32);
                    do{
                        skip_newlines(p);
                        args[arg_count++] = parse_expression(p);
                        skip_newlines(p);
                    }while(match(p,TOKEN_COMMA));
                }
                expect(p,TOKEN_RPAREN);
                return make_call(name.value,args,arg_count);
            }
            fprintf(stderr,
                "Parse error at line %d: unexpected token after identifier\n",
                t.line);
                p->had_error = 1;
                return make_number(0);
        }
        default:
            fprintf(stderr,
                "Parser error at line %d: unexpected token '%s'\n", 
            t.line,t.value);
            p->had_error = 1;
            advance_tok(p);
            return make_number(0);
        
    }
}


/*--------------------------------------------
AST DEBUG PRINTER
Prints the tree with indentation - 
lets you see exactly what was parsed
---------------------------------------------*/

void print_ast(ASTNode* node, int indent){
    if(!node) return;
    for(int i=0;i<indent;i++) printf(" ");

    switch(node->type){
        case NODE_PROGRAM:
            printf("Program (%d statements\n",
                    node->as.program.count);
            for(int i=0;i<node->as.program.count;i++){
                print_ast(node->as.program.statements[i],indent+1);
            }
            break;

        case NODE_VAR_DECL:
            printf("VarDecl: %s is %s \n", node->as.var_decl.name,
                    type_name(node->as.var_decl.var_type));
            print_ast(node->as.var_decl.value,indent+1);
            break;
        
        case NODE_ASSIGN:
                printf("Assign: %s = \n" ,node->as.assign.name);
                print_ast(node->as.assign.value,indent+1);
                break;
        case NODE_FUNC_DECL:
                printf("FuncDecl: %s returns %s (%d params)\n",
                    node->as.func_decl.name,
                    type_name(node->as.func_decl.return_type),
                    node->as.func_decl.param_count
                );

                for(int i=0; i< node->as.func_decl.param_count;i++){
                    printf(" %*sParam: %s is %s\n",indent*2,"",
                        node->as.func_decl.params[i].name,
                        type_name(node->as.func_decl.params[i].type));
                }
                print_ast(node->as.func_decl.body,indent+1);
                break;
        
        case NODE_RETURN:
                printf("Return:\n");
                print_ast(node->as.ret.value,indent+1);
                break;
        
        case NODE_IF:
            printf("If:\n");
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf("condition:\n");
            print_ast(node->as.if_stmt.condition, indent+2);
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf("then:\n");
            print_ast(node->as.if_stmt.then_block, indent+2);
            if (node->as.if_stmt.else_block) {
                for (int i = 0; i < indent+1; i++) printf("  ");
                printf("otherwise:\n");
                print_ast(node->as.if_stmt.else_block, indent+2);
            }
            break;

        case NODE_REPEAT:
            printf("Repeat:\n");
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf("init:\n");
            print_ast(node->as.repeat.init, indent+2);
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf("condition:\n");
            print_ast(node->as.repeat.condition, indent+2);
            for (int i = 0; i < indent+1; i++) printf("  ");
            printf("body:\n");
            print_ast(node->as.repeat.body, indent+2);
            break;

        case NODE_REPEAT_EACH:
            printf("RepeatEach: %s in collection\n",
                   node->as.repeat_each.item_name);
            print_ast(node->as.repeat_each.body, indent+1);
            break;

        case NODE_SHOW:
            printf("Show:\n");
            print_ast(node->as.show.value, indent+1);
            break;

        case NODE_BLOCK:
            printf("Block (%d statements)\n",
                   node->as.block.count);
            for (int i = 0; i < node->as.block.count; i++)
                print_ast(node->as.block.statements[i], indent+1);
            break;

        case NODE_BINARY_EXPR:
            printf("Binary: %s\n", node->as.binary.op);
            print_ast(node->as.binary.left,  indent+1);
            print_ast(node->as.binary.right, indent+1);
            break;

        case NODE_UNARY_EXPR:
            printf("Unary: %s\n", node->as.unary.op);
            print_ast(node->as.unary.operand, indent+1);
            break;

        case NODE_CALL_EXPR:
            printf("Call: %s (%d args)\n",
                   node->as.call.name,
                   node->as.call.arg_count);
            for (int i = 0; i < node->as.call.arg_count; i++)
                print_ast(node->as.call.args[i], indent+1);
            break;

        case NODE_IDENTIFIER:
            printf("Identifier: %s\n",
                   node->as.identifier.name);
            break;

        case NODE_NUMBER_LIT:
            printf("Number: %d\n", node->as.number.value);
            break;

        case NODE_DECIMAL_LIT:
            printf("Decimal: %g\n", node->as.decimal.value);
            break;

        case NODE_STRING_LIT:
            printf("String: \"%s\"\n", node->as.string.value);
            break;

        case NODE_BOOL_LIT:
            printf("Bool: %s\n",
                   node->as.boolean.value ? "true" : "false");
            break;

        default:
            printf("Unknown node\n");
                
    }
}


/*----------------------------------------
MAIN PARSER FUNCTION
Entry Point - call this with your
token array to get back an AST
------------------------------------------*/

ASTNode* parse(Token* tokens, int count){
    Parser p;
    p.tokens = tokens;
    p.pos = 0;
    p.count = count;
    p.had_error = 0;

    ASTNode** stmts = (ASTNode**)malloc(sizeof(ASTNode*)*512);
    int stmt_count = 0;

    skip_newlines(&p);

    while(!check(&p,TOKEN_EOF)){
        skip_newlines(&p);
        if(check(&p,TOKEN_EOF)) break;
        stmts[stmt_count++] = parse_statement(&p);
        skip_newlines(&p);
    }

    if(p.had_error){
        fprintf(stderr,"Parsing failed with errors.\n");
    }

    return make_program(stmts , stmt_count);
}




/*------------------------------------------
FREE AST - cleanup memory 
-------------------------------------------*/

void free_ast(ASTNode* node){
    if(!node) return;
    switch(node->type){
        case NODE_PROGRAM:
            for(int i=0; i<node->as.program.count;i++){
                free_ast(node->as.program.statements[i]);
            }
            free(node->as.program.statements);
            break;
        
        case NODE_VAR_DECL:
            free(node->as.var_decl.name);
            free_ast(node->as.var_decl.value);
            break;
        
        case NODE_BLOCK:
            for(int i=0;i<node->as.block.count;i++){
                free_ast(node->as.block.statements[i]);
            }
            free(node->as.block.statements);
            break;

        default : break;
    }

    free(node);
}