#ifndef CODEGEN_H
#define CODEGEN_H
#include <stdbool.h>

void cgPROG(PROG * p, char ** fileOutName, STRUCTLIST * sll);
void cgTOPSTMT(TOPSTMT * ts);
void cgList(List * l);
void cgTYPE(TYPE * t, int indentation);
void cgSTMT(STMT * s, int indentation, bool line_end);
void cgEXP(EXP * e);
void cgSWITCHCASE(SWITCHCASE *s, int indentation);
void findent(int indentation);

void cg_general_code_init(FILE*f, STRUCTLIST * sl);
void cg_struct_equality(EXP*, List*, EXP*, List*);
void cg_struct_equalitys(char*, List*, char*, List*);

void struct_init(char* structName, List* structFields);
// void struct_inits(char* structName, List* structFields);

void cg_initvar(TYPE * t, char*);

void cgParamList(List * l, int blank_count, bool newLine, int indentation);

#endif
