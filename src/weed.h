#ifndef WEED_H
#define WEED_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"tree.h"
#include<stdbool.h>

int list_count(List*);
bool weed_statement(STMT*, bool,bool,bool);
bool weed_switch(SWITCHCASE*, bool, bool, bool);
bool weed_source(PROG*);
bool weed_identlist(List * list, int lineno);
#endif
