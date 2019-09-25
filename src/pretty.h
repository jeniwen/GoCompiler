#ifndef PRETTY_H
#define PRETTY_H
#include "tree.h"
#include <stdbool.h>

void indent(int indentation);
void prettyTOPSTMT(TOPSTMT * ts);
void prettyList(List * l);
void prettyTYPE(TYPE * t, int indentation);
void prettyPROG(PROG* p);
void prettySTMT(STMT * s, int indentation, bool line_end);
void prettyEXP(EXP * e);
void prettySWITCHCASE(SWITCHCASE *s, int indentation);

#endif
