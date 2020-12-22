#include "ir.h"
#include "log.h"

Value *ir_from_ast(AstSexpr *ast)
{
    return ir_from_ast_sexpr(ast);
}

Value *ir_from_ast_atom(AstAtom *atom)
{
    Value *v;
    switch (atom->node.type) {
    case AST_ATOM_FLOAT:
        v = value_new_float(atom->as.decimal);
        break;
    case AST_ATOM_INT:
        v = value_new_int(atom->as.integer);
        break;
    case AST_ATOM_STRING:
        v = value_new_string(atom->as.string);
        break;
    case AST_ATOM_SYMBOL:
        v = value_new_symbol(atom->as.string);
        break;
    default:
        LOG_CRITICAL("Unknown AST atom type: %d", atom->node.type);
        v = NULL;
    }
    return v;
}

Value *ir_from_ast_list(AstList *ast_list)
{
    if (ast_list->node.type == AST_LIST_EMPTY) {
        return value_new_list(NULL);
    }
    Value *sexpr = ir_from_ast_sexpr(ast_list->as.compound.sexpr);
    Value *list = ir_from_ast_list(ast_list->as.compound.list);
    list->value.list = list_prepend(list->value.list, sexpr);
    return list;
}

Value *ir_from_ast_sexpr(AstSexpr *ast)
{
    if (!ast) return NULL;
    Value *result;
    Value *quote;
    Value *sexpr;
    switch (ast->node.type) {
    case AST_SEXPR_ATOM:
        result = ir_from_ast_atom(ast->as.atom);
        break;
    case AST_SEXPR_LIST:
        result =  ir_from_ast_list(ast->as.list);
        break;
    case AST_SEXPR_QUOTE:
        result = value_new_list(NULL);
        sexpr = ir_from_ast_sexpr(ast->as.quoted);
        quote = value_new_symbol("quote");
        result->value.list = list_append(result->value.list, quote);
        result->value.list = list_append(result->value.list, sexpr);
        break;
    case AST_SEXPR_QUASIQUOTE:
        result = value_new_list(NULL);
        sexpr = ir_from_ast_sexpr(ast->as.quoted);
        quote = value_new_symbol("quasiquote");
        result->value.list = list_append(result->value.list, quote);
        result->value.list = list_append(result->value.list, sexpr);
        break;
    case AST_SEXPR_UNQUOTE:
        result = value_new_list(NULL);
        sexpr = ir_from_ast_sexpr(ast->as.quoted);
        quote = value_new_symbol("unquote");
        result->value.list = list_append(result->value.list, quote);
        result->value.list = list_append(result->value.list, sexpr);
        break;
    case AST_SEXPR_SPLICE_UNQUOTE:
        result = value_new_list(NULL);
        sexpr = ir_from_ast_sexpr(ast->as.quoted);
        quote = value_new_symbol("splice-unquote");
        result->value.list = list_append(result->value.list, quote);
        result->value.list = list_append(result->value.list, sexpr);
        break;
    }
    return result;
}

