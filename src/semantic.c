#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "../include/symtable.h"
#include "../include/ast.h"

/*-------------------------------------------
HelloWorld Semantic Analyzer
Stage 3

Walks the AST and:
1. Builds the symbol table
2. Checks every variable is declared
3. Checks types match in expressions
4. Checks functions return correct type
5. Checks function calls match signatures
---------------------------------------------*/


/* Analyzer state */
typedef struct{
    SymTable* table;
    int had_error;
    LangType current_func_return;
}Analyzer;


/* Error Helper */
static void sem_error(Analyzer* a, const char* msg){
    fprintf(stderr,"Semantic error : %s\n",msg);
    a->had_error = 1;
}

/* forward decleration */
static LangType analyze_expr(Analyzer* a, ASTNode* node);
static void analyze_stmt(Analyzer* a, ASTNode* node);


/*---------------------------------------
EXPRESSION ANALYSIS 
Returns the type of the exprssion
-----------------------------------------*/

static LangType analyze_expr(Analyzer* a, ASTNode* node){
    if(!node) return TYPE_UNKNOWN;

    switch(node->type){
        case NODE_NUMBER_LIT: return TYPE_NUM;
        case NODE_DECIMAL_LIT: return TYPE_DECIMAL;
        case NODE_STRING_LIT : return TYPE_TEXT;
        case NODE_BOOL_LIT: return TYPE_BOOL;

        case NODE_IDENTIFIER:{
            char* name = node->as.identifier.name;
            Symbol* sym = symtable_lookup(a->table,name);
            if(!sym){
                char msg[256];
                snprintf(msg,sizeof(msg),
            "undefined variable '%s'",name);
            sem_error(a,msg);
            return TYPE_UNKNOWN;
            }
            return sym->type;
        }

        case NODE_BINARY_EXPR:{
            LangType left = analyze_expr(a,node->as.binary.left);
            LangType right = analyze_expr(a,node->as.binary.right);
            char* op = node->as.binary.op;

            /* comparision operators always returns bool */
            if(strcmp(op,"==") == 0 || strcmp(op,"!=") == 0 
                || strcmp(op,"<")==0 || strcmp(op,">") == 0
                || strcmp(op,"<=") == 0 || strcmp(op,">=") == 0){
                    if(left != right){
                        sem_error(a,"type mismatch in comparision");
                    }
                    return TYPE_BOOL;
            }

            /* ARITHEMATIC OPERATORS */
            if(left != right ){
                char msg[256];
                snprintf(msg,sizeof(msg),
                    "type mismatch in '%s' expression",op);
                sem_error(a,msg);
                return TYPE_UNKNOWN;
            }
            if(left != TYPE_NUM && left != TYPE_DECIMAL){
                char msg[256];
                snprintf(msg,sizeof(msg),
                        "operator '%s' requires num or decimal",op);
                sem_error(a,msg);
            }
            return left;
        }

        case NODE_UNARY_EXPR:{
            LangType operand = analyze_expr(a, node->as.unary.operand);
            if (operand != TYPE_NUM && operand != TYPE_DECIMAL){
                sem_error(a,"unary '-' requires num or decimal");
            }
            return operand;
        }

        case NODE_CALL_EXPR:{
            char* name = node->as.call.name;
            Symbol* sym = symtable_lookup(a->table, name);
            if (!sym) {
                char msg[256];
                snprintf(msg, sizeof(msg),
                    "undefined function '%s'", name);
                sem_error(a, msg);
                return TYPE_UNKNOWN;
            }
            if (!sym->is_func) {
                char msg[256];
                snprintf(msg, sizeof(msg),
                    "'%s' is a variable not a function", name);
                sem_error(a, msg);
                return TYPE_UNKNOWN;
            }
            /* check argument count */
            if (node->as.call.arg_count != sym->param_count) {
                char msg[256];
                snprintf(msg, sizeof(msg),
                    "function '%s' expects %d args but got %d",
                    name, sym->param_count,
                    node->as.call.arg_count);
                sem_error(a, msg);
                return sym->type;
            }
             /* check argument types */
            for (int i = 0; i < node->as.call.arg_count; i++) {
                LangType arg_type = analyze_expr(a, node->as.call.args[i]);
                if (arg_type != sym->param_types[i]) {
                    char msg[256];
                    snprintf(msg, sizeof(msg),
                        "argument %d of '%s': expected %s got %s",
                        i + 1, name,
                        type_name(sym->param_types[i]),
                        type_name(arg_type));
                    sem_error(a, msg);
                }
            }
            return sym->type;
        }

        default:
            return TYPE_UNKNOWN;
    }
}


/*─────────────────────────────────────────
  STATEMENT ANALYSIS
─────────────────────────────────────────*/

static void analyze_stmt(Analyzer* a, ASTNode* node) {
    if (!node) return;

    switch (node->type) {

        case NODE_VAR_DECL: {
            char*    name  = node->as.var_decl.name;
            LangType type  = node->as.var_decl.var_type;
            LangType vtype = analyze_expr(a, node->as.var_decl.value);

            /* type mismatch check */
            if (vtype != TYPE_UNKNOWN && vtype != type) {
                char msg[256];
                snprintf(msg, sizeof(msg),
                    "cannot assign %s to variable '%s' of type %s",
                    type_name(vtype), name, type_name(type));
                sem_error(a, msg);
            }

            /* add to symbol table */
            if (!symtable_add_var(a->table, name, type)) {
                char msg[256];
                snprintf(msg, sizeof(msg),
                    "variable '%s' already declared in this scope", name);
                sem_error(a, msg);
            }
            break;
        }

        case NODE_ASSIGN: {
            char* name = node->as.assign.name;
            Symbol* sym = symtable_lookup(a->table, name);
            if (!sym) {
                char msg[256];
                snprintf(msg, sizeof(msg),
                    "assignment to undeclared variable '%s'", name);
                sem_error(a, msg);
                break;
            }
            LangType vtype = analyze_expr(a, node->as.assign.value);
            if (vtype != TYPE_UNKNOWN && vtype != sym->type) {
                char msg[256];
                snprintf(msg, sizeof(msg),
                    "cannot assign %s to '%s' which is %s",
                    type_name(vtype), name, type_name(sym->type));
                sem_error(a, msg);
            }
            break;
        }

        case NODE_FUNC_DECL: {
            char* name = node->as.func_decl.name;

            /* collect param types for symbol table */
            int pc = node->as.func_decl.param_count;
            LangType* ptypes = (LangType*)malloc(sizeof(LangType) * pc);
            for (int i = 0; i < pc; i++)
                ptypes[i] = node->as.func_decl.params[i].type;

            /* register function in outer scope */
            if (!symtable_add_func(a->table, name,
                    node->as.func_decl.return_type, pc, ptypes)) {
                char msg[256];
                snprintf(msg, sizeof(msg),
                    "function '%s' already declared", name);
                sem_error(a, msg);
            }
            free(ptypes);

            /* enter function scope */
            symtable_enter_scope(a->table);
            LangType prev_return = a->current_func_return;
            a->current_func_return = node->as.func_decl.return_type;

            /* add params to inner scope */
            for (int i = 0; i < pc; i++) {
                symtable_add_var(a->table,
                    node->as.func_decl.params[i].name,
                    node->as.func_decl.params[i].type);
            }

            /* analyze body */
            analyze_stmt(a, node->as.func_decl.body);

            a->current_func_return = prev_return;
            symtable_exit_scope(a->table);
            break;
        }

        case NODE_RETURN: {
            LangType ret = analyze_expr(a, node->as.ret.value);
            if (ret != TYPE_UNKNOWN &&
                ret != a->current_func_return) {
                char msg[256];
                snprintf(msg, sizeof(msg),
                    "return type mismatch: expected %s but giving %s",
                    type_name(a->current_func_return),
                    type_name(ret));
                sem_error(a, msg);
            }
            break;
        }

        case NODE_IF: {
            LangType ctype = analyze_expr(a, node->as.if_stmt.condition);
            if (ctype != TYPE_BOOL && ctype != TYPE_UNKNOWN) {
                sem_error(a, "check condition must be a bool expression");
            }
            symtable_enter_scope(a->table);
            analyze_stmt(a, node->as.if_stmt.then_block);
            symtable_exit_scope(a->table);
            if (node->as.if_stmt.else_block) {
                symtable_enter_scope(a->table);
                analyze_stmt(a, node->as.if_stmt.else_block);
                symtable_exit_scope(a->table);
            }
            break;
        }

        case NODE_REPEAT: {
            symtable_enter_scope(a->table);
            analyze_stmt(a, node->as.repeat.init);
            LangType ctype = analyze_expr(a, node->as.repeat.condition);
            if (ctype != TYPE_BOOL && ctype != TYPE_UNKNOWN) {
                sem_error(a, "repeat condition must be a bool expression");
            }
            analyze_stmt(a, node->as.repeat.step);
            analyze_stmt(a, node->as.repeat.body);
            symtable_exit_scope(a->table);
            break;
        }

        case NODE_REPEAT_EACH: {
            char* item_name = node->as.repeat_each.item_name;
            symtable_enter_scope(a->table);
            symtable_add_var(a->table, item_name, TYPE_UNKNOWN);
            analyze_stmt(a, node->as.repeat_each.body);
            symtable_exit_scope(a->table);
            break;
        }

        case NODE_SHOW: {
            analyze_expr(a, node->as.show.value);
            break;
        }

        case NODE_BLOCK: {
            for (int i = 0; i < node->as.block.count; i++)
                analyze_stmt(a, node->as.block.statements[i]);
            break;
        }

        case NODE_CALL_EXPR: {
            analyze_expr(a, node);
            break;
        }

        default:
            break;
    }
}


/*─────────────────────────────────────────
  MAIN ANALYZE FUNCTION
─────────────────────────────────────────*/

int analyze(ASTNode* root) {
    Analyzer a;
    a.table               = symtable_create();
    a.had_error           = 0;
    a.current_func_return = TYPE_NOTHING;

    /* analyze all top level statements */
    for (int i = 0; i < root->as.program.count; i++) {
        analyze_stmt(&a, root->as.program.statements[i]);
    }

    /* print symbol table */
    symtable_print(a.table);

    int had_error = a.had_error;
    symtable_destroy(a.table);
    return had_error;
}
