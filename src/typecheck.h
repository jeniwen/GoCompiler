#ifndef TYPECHECK_H
#define TYPECHECK_H
#include "tree.h"
#include "symbol.h"

void typecheck_PROG(PROG * root, STRUCTLIST ** structlistptr);
void typecheck_TOPSTMT(SymbolTable * t, TOPSTMT * s);
int typecheck_STMT(SymbolTable * t, STMT * s, TYPE * returnType, int returnLast);
void typecheck_EXP(SymbolTable * t, EXP * e);
void checkTypeEquality(TYPE * t1, TYPE *t2, int lineno);
TYPE * checkValidType(SymbolTable * t, TYPE * type, int lineno);
void typecheck_var_decl(SymbolTable * t, STMT * s);
void typecheck_type_decl(SymbolTable *t, STMT *s);
int typecheck_SWITCHCASE(SymbolTable * t, SWITCHCASE * caseStmt, TYPE * type, TYPE * returnType);
int typecheck_forLoop(SymbolTable * t, STMT* s,  TYPE * returnType);
void typecheck_Assign(SymbolTable * t, STMT* s);
void typecheck_OpAssign(SymbolTable * t, STMT* s);
void typecheck_Print(SymbolTable * t, STMT * s);
TYPE * resolve_type_with_TYPE(SymbolTable *t, TYPE * type);
void checkNumericType(SymbolTable * t, TYPE * type, int lineno);
void checkComparable(SymbolTable *t, TYPE * type, int lineno);
void typecheck_shortAssign(SymbolTable * t, STMT * s);
void checkLvalue(SymbolTable *t, EXP * e);
void checkOpCompatibility(SymbolTable *t, EXP* identifier, AssignOpKind aop, EXP * expression);

#endif /* !TYPECHECK_H */
