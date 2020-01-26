/*
 * ir.c
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#include "ir.h"
#include "log.h"

Value *ir_from_ast(AstSexpr *ast)
{
    return ir_from_ast_sexpr(ast);
}

Value *ir_from_ast_atom(AstAtom *atom)
{
    Value *v;
    switch (atom->type) {
    case ATOM_FLOAT:
        v = value_new_float(atom->value.float_);
        break;
    case ATOM_INT:
        v = value_new_int(atom->value.int_);
        break;
    case ATOM_STRING:
        v = value_new_string(atom->value.string);
        break;
    case ATOM_SYMBOL:
        v = value_new_symbol(atom->value.string);
        break;
    default:
        LOG_CRITICAL("Unknown AST atom type: %d", atom->type);
        v = NULL;
    }
    return v;
}

Value *ir_from_ast_list(AstList *ast_list)
{
    if (ast_list->type == LIST_EMPTY) {
        return value_new_list(NULL);
    }
    Value *sexpr = ir_from_ast_sexpr(ast_list->ast.compound.sexpr);
    Value *list = ir_from_ast_list(ast_list->ast.compound.list);
    list->value.list = list_cons(list->value.list, sexpr);
    return list;
}

Value *ir_from_ast_sexpr(AstSexpr *ast)
{
    if (!ast) return NULL;
    Value *result;
    Value *quote;
    Value *sexpr;
    switch (ast->type) {
    case SEXPR_ATOM:
        result = ir_from_ast_atom(ast->ast.atom);
        break;
    case SEXPR_LIST:
        result =  ir_from_ast_list(ast->ast.list);
        break;
    case SEXPR_QUOTE:
        result = value_new_list(NULL);
        sexpr = ir_from_ast_sexpr(ast->ast.quoted);
        quote = value_new_symbol("quote");
        result->value.list = list_conj(result->value.list, quote);
        result->value.list = list_conj(result->value.list, sexpr);
        break;
    case SEXPR_QUASIQUOTE:
        result = value_new_list(NULL);
        sexpr = ir_from_ast_sexpr(ast->ast.quoted);
        quote = value_new_symbol("quasiquote");
        result->value.list = list_conj(result->value.list, quote);
        result->value.list = list_conj(result->value.list, sexpr);
        break;
    case SEXPR_UNQUOTE:
        result = value_new_list(NULL);
        sexpr = ir_from_ast_sexpr(ast->ast.quoted);
        quote = value_new_symbol("unquote");
        result->value.list = list_conj(result->value.list, quote);
        result->value.list = list_conj(result->value.list, sexpr);
        break;
    case SEXPR_SPLICE_UNQUOTE:
        result = value_new_list(NULL);
        sexpr = ir_from_ast_sexpr(ast->ast.quoted);
        quote = value_new_symbol("splice-unquote");
        result->value.list = list_conj(result->value.list, quote);
        result->value.list = list_conj(result->value.list, sexpr);
        break;
    }
    return result;
}

