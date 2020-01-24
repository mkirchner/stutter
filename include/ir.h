/*
 * ir.h
 * Copyright (C) 2019 Marc Kirchner
 *
 * Distributed under terms of the MIT license.
 */

#ifndef IR_H
#define IR_H

#include "ast.h"
#include "value.h"

Value *ir_from_ast(AstSexpr *ast);
Value *ir_from_ast_atom(AstAtom *);
Value *ir_from_ast_list(AstList *);
Value *ir_from_ast_sexpr(AstSexpr *);

#endif /* !IR_H */
