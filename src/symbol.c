#define HashSize 317
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "tree.h"
#include "pretty.h"
#include "symbol.h"

bool printSymbolTable;
int scopeDepth;

bool isBaseType(TYPE * type) {
	switch (type->kind) {
	case intType:
	case floatType:
	case runeType:
	case stringType:
	case boolType:
		return true; break;
	case nullType:
	case structType:
	case arrayType:
	case sliceType:
	case unknownType:
		return false; break;
	}
	return false;
}

SymbolTable *initSymbolTable() {
	SymbolTable *t = malloc(sizeof(SymbolTable));
	for (int i = 0; i < HashSize; i++) {
		t->table[i] = NULL;
	}
	t->parent = NULL;
	return t;
}

SYMBOL * makeSymbol_var(char * name, TYPE * type) {
	SYMBOL * s = malloc(sizeof(SYMBOL));
	s->kind = symkind_var;
	s->name = name;
	s->typelit.type = type;
	return s;
}

SYMBOL * makeSymbol_const(char * name, TYPE * type) {
	SYMBOL * s = malloc(sizeof(SYMBOL));
	s = makeSymbol_var(name, type);
	s->kind = symkind_const;
	return s;
}

SYMBOL * makeSymbol_function(char * name, SYMBOLLIST * paramList, TYPE *returnType){
	SYMBOL * s = malloc(sizeof(SYMBOL));
	s->kind = symkind_func;
	s->name = name;
	s->typelit.functiondec.paramList = paramList;
	s->typelit.functiondec.returnType = returnType;
	return s;
}

SYMBOL * makeSymbol_base(char* name, TYPE * type) {
	SYMBOL * s = malloc(sizeof(SYMBOL));
	s->kind = symkind_base;
	s->name = name;
	switch (type->kind) {
	case intType:
	case floatType:
	case runeType:
	case stringType:
	case boolType:
	case nullType: s->typelit.type = type; break;
	default:
		fprintf(stderr, "Error: Cannot make a base symbol from a non-base type");
		exit(1);
		break;
	}
	return s;
}

SYMBOL * makeSymbol_type(char * name, SYMBOL * underlying) {
	SYMBOL * s = malloc(sizeof(SYMBOL));
	s->kind = symkind_type;
	s->name = name;
	s->typelit.defined.underlying = underlying;
	s->typelit.defined.type = makeTYPE_ident(name);
	return s;
}

SYMBOL * makeSymbol_struct(char * name, SYMBOLLIST * fieldList, TYPE * structtype) {
	SYMBOL * s = malloc(sizeof(SYMBOL));
	s->kind = symkind_struct;
	s->name = name;
	s->typelit.structdec.fieldList = fieldList;
	s->typelit.structdec.structtype = structtype;
	return s;
}

SYMBOL * makeSymbol_array(char * name, SYMBOL * entrytype, TYPE * arraytype) {
	SYMBOL * s = malloc(sizeof(SYMBOL));
	s->kind = symkind_array;
	s->name = name;
	s->typelit.arraydec.entrytype = entrytype;
	s->typelit.arraydec.arraytype = arraytype;
	return s;
}

SYMBOL * makeSymbol_slice(char * name, SYMBOL * entrytype, TYPE * slicetype) {
	SYMBOL * s = malloc(sizeof(SYMBOL));
	s->kind = symkind_slice;
	s->name = name;
	s->typelit.slicedec.entrytype = entrytype;
	s->typelit.slicedec.slicetype = slicetype;
	return s;
}

//Primarily for making arrays and slices in typeToSymbol above.
char * typeName (TYPE * type) {
	char * returnStr = malloc(100);
	switch(type->kind) {
	case intType: return "int"; break;
	case floatType: return "float64"; break;
	case runeType: return "rune"; break;
	case stringType: return "string"; break;
	case boolType: return "bool"; break;
	case nullType: return "null"; break;
	case structType: return "struct"; break;
	case arrayType:
		// returnStr = malloc(sizeof(typeName(type->val.arrayData.entryType))+5);
		strcpy(returnStr, "[]");
		strcat(returnStr, typeName(type->val.arrayData.entryType));
		return returnStr;
		break;
	case sliceType:
		// returnStr = malloc(sizeof(typeName(type->val.arrayData.entryType))+5);
		strcpy(returnStr, "[]");
		strcat(returnStr, typeName(type->val.sliceEntryType));
		return returnStr;
		break;
	case unknownType: return type->val.ident; break;
	}
	return NULL;
}


//For structs and functions: makes a list of symbols for each field/parameter respectively
//Given a the current symbol table and a list of params (struct {List *idlist; TYPE* type;}), will return a list of symbols
SYMBOLLIST * makeSymbolList(SymbolTable * t, List * l) {
	SYMBOLLIST * pl = malloc(sizeof(SYMBOLLIST));
	SYMBOLLIST * currSymList = pl;
	List * currParam = l;
	while (currParam != NULL) { //loops through list of params
		if (currParam->kind == listKindParam) {
			List * currID = currParam->current.param.idlist;
			while (currID != NULL) { //loops through idlist of list of params
				SYMBOLLIST * l2 = pl;
				while (l2 != currSymList) {
					if(strcmp(l2->currSym->name, currID->current.id) == 0 && strcmp(l2->currSym->name, "_") != 0) {
						fprintf(stderr, "Error: %s appears more than once in the list\n", l2->currSym->name);
						exit(1);
					}
					l2 = l2->next;
				}
				currSymList->currSym = makeSymbol_var(currID->current.id, currParam->current.param.type);
				currSymList->next = malloc(sizeof(SYMBOLLIST));
				currSymList = currSymList->next;
				currID =currID->nextli;
			}
		} else {
			fprintf(stderr, "Error: Trying to use makeSymbolList on a non-parameter\n");
			exit(1);
		}
		currParam = currParam->nextli;
	}
	currSymList->currSym = NULL;

	return pl;
}

void printSYMBOLLIST(SYMBOLLIST * l) {
	SYMBOLLIST * curr = l;
	while (curr != NULL && curr->currSym != NULL) {
		printf("%s ", curr->currSym->name);
		printSymbolParamToString(curr->currSym);
		printf("; ");
		curr = curr->next;
	}
}

void printStruct(SYMBOL * s) {
	if (s->kind == symkind_struct) {
		printf("struct { ");
		printSYMBOLLIST(s->typelit.structdec.fieldList);
		printf("}");
	}
	else {
		fprintf(stderr, "Error: Trying to call printStruct on a non-struct TYPE\n");
		exit(1);
	}
}



SYMBOL * getUnderlyingSymbol(SymbolTable * t, SYMBOL * s) {
	if (s->kind == symkind_type) { //?
		return getSymbol(t,s->name);
	}
	else {
		fprintf(stderr, "Error: Can only get underlying symbols of defined types\n");
		exit(1);
	}
}

void printCurrTable(SymbolTable *t) {
	for (int i = 0; i < HashSize; i++) {
		if (t->table[i] != NULL) {
			printSymbol(t->table[i]);
		}
	}
}


//For printing things like paramlists
void printSymbolParamToString(SYMBOL * s) {
	switch (s->kind) {
	case symkind_var: printf("%s",typeName(s->typelit.type)); break;
	case symkind_type: printf("%s",s->name); break;
	case symkind_const: printf("%s",typeName(s->typelit.type)); break;
	case symkind_array: printf("[]"); printSymbolParamToString(s->typelit.arraydec.entrytype); break;
	case symkind_slice: printf("[]"); printSymbolParamToString(s->typelit.slicedec.entrytype); break;
	case symkind_struct: printf("%s", s->name);
	default: break;
	}
}


void printSymbol(SYMBOL *s) {
	indent(scopeDepth);
	printf("%s ",s->name);
	switch (s->kind) {
	case symkind_var:
		if (s->typelit.type != NULL) printf("[variable] = %s", typeName(s->typelit.type));
		else printf("[variable] = <infer>");
		break;
	case symkind_func:
		printf("[function] = (");
		SYMBOLLIST * currS = s->typelit.functiondec.paramList;
		while (currS != NULL && currS->currSym != NULL) {
			printSymbolParamToString(currS->currSym);
			currS = currS->next;
			if (currS->currSym != NULL) {
				printf(", ");
			}
		}
		printf(") -> ");
		if (s->typelit.functiondec.returnType == NULL) {
			printf("void");
		} else {
			printf("%s",typeName(s->typelit.functiondec.returnType));
		}
		break;
	case symkind_type:
		printf("[type] = ");// typeName(s->typelit.type));
		SYMBOL * curr = s;
		while (curr != NULL && curr->kind == symkind_type) {
			if (curr->typelit.defined.underlying)
				printf("%s", curr->name);
			// printf("EMPTY UNDERLYING\n");
			curr = curr->typelit.defined.underlying;
			if (curr != NULL && curr->kind != symkind_base) {
				printf(" -> ");
				if (curr->kind == symkind_var || curr->kind == symkind_const)
					printf("%s", curr->name);
				else if (curr->kind == symkind_struct) {
					printStruct(curr);
				} else if (curr->kind == symkind_array) {
					printf("[]%s", curr->typelit.arraydec.entrytype->name);
				} else if (curr->kind == symkind_slice) {
					printf("[]%s", curr->typelit.slicedec.entrytype->name);
				}
			}
		}
		break;
	case symkind_const:
		printf("[constant] = %s", typeName(s->typelit.type));
		break;
	case symkind_slice: printf("slice"); break;
	case symkind_struct:
		printf("[type] = ");
		printStruct(s);
		break;
	case symkind_array: printf("array"); break;
	case symkind_base: printf("[type] = "); printf("%s",typeName(s->typelit.type)); break;

	}
	printf("\n");
}

int Hash(char *str) {
	unsigned int hash = 0;
	while (*str) hash = (hash << 1) + *str++;
	return hash % HashSize;
}

bool declaredLocally(SymbolTable *t, char *name) {
	int i = Hash(name);
	for (SYMBOL *s = t->table[i]; s; s = s->next) {
		if (strcmp(s->name, name) == 0) return 1;
	}
	return 0;
}

SymbolTable *scopeSymbolTable(SymbolTable *s) {
	if (printSymbolTable) {
		scopeDepth++;
		for (int depth = 0; depth < scopeDepth-1; depth++) {
			printf("\t");
		}
		printf("{\n");
	}
	SymbolTable *t = initSymbolTable();
	t->parent = s;
	return t;
}

SymbolTable * unscopeSymbolTable(SymbolTable *s) {
	if (printSymbolTable) {
		scopeDepth--;
		for (int depth = 0; depth < scopeDepth; depth++) {
			printf("\t");
		}
		printf("}\n");
	}
	return s->parent;
}

SYMBOL *putSymbol(SymbolTable *t, SYMBOL * sym) {
	if (sym != NULL && strcmp(sym->name, "_") != 0) {
		if (declaredLocally(t, sym->name)) {
			fprintf(stderr, "Error: variable %s has already been declared locally.\n", sym->name);
			exit(1);
		}
		int i = Hash(sym->name);
		sym->next = t->table[i];
		t->table[i] = sym;
		if (printSymbolTable) printSymbol(sym);
	} else if (sym == NULL) {
		fprintf(stderr, "Error: trying to put a NULL symbol into the table.\n");
		exit(1);
	}
	return sym;
}

SYMBOL *getSymbol(SymbolTable *t, char *name) {
	int i = Hash(name);
	// Check the current scope
	for (SYMBOL *s = t->table[i]; s; s = s->next) {
		if (strcmp(s->name, name) == 0) return s;
	}
	// Check for existence of a parent scope
	if (t->parent == NULL) {
		fprintf(stderr, "Error:  %s has not been declared in this context\n", name);
		exit(1);
	}
	// Check the parent scopes
	return getSymbol(t->parent, name);
}

bool isDeclared(SymbolTable *t, char *name) {
	int i = Hash(name);
	for (SYMBOL *s = t->table[i]; s; s = s->next) {
		if (strcmp(s->name, name) == 0) return true;
	}
	// Check for existence of a parent scope
	if (t->parent == NULL) {
		return false;
	}
	return isDeclared(t->parent, name);
}

TYPE * resolveType(SymbolTable * t, SYMBOL * sym) {
	if (sym == NULL) {
		fprintf(stderr, "Error: Trying to resolve type of NULL.");
		exit(1);
	}
	switch (sym->kind) {
	case symkind_var:
		fprintf(stderr, "Error: Cannot resolve type for a variable.");
		exit(1);
		break;
	case symkind_func:
		fprintf(stderr, "Error: Cannot resolve type for a function.");
		exit(1);
		break;
	case symkind_const:
		fprintf(stderr, "Error: Cannot resolve type for a constant.");
		exit(1);
		break;
	case symkind_type:
		return resolveType(t, sym->typelit.defined.underlying); break;
	case symkind_struct:
		return sym->typelit.structdec.structtype; break;
	case symkind_array:
		return sym->typelit.arraydec.arraytype; break;
	case symkind_slice:
		return sym->typelit.slicedec.slicetype; break;
	case symkind_base: return sym->typelit.type; break;
	}
	return NULL;
}

void symbolList (SymbolTable * t, List * l) {
	if (l != NULL) {
		switch (l->kind) {
		case listKindExp: symbolEXP(t, l->current.expression); break;
		case listKindParam: symbolList(t, l->current.param.idlist); break;
		case listKindId: getSymbol(t, l->current.id); break;
		case listKindVarSpeclist: break;
		}
		symbolList(t, l->nextli);
	}
}


void symbolEXP( SymbolTable * t, EXP * e) {
	if (e != NULL) {
		switch (e->kind) {
		case expressionKindIdentifier:
			getSymbol(t, e->val.identifier);
			break;
		case expressionKindIntLiteral: break;
		case expressionKindFloatLiteral: break;
		case expressionKindRuneLiteral:  break;
		case expressionKindStringLiteral: break;
		case expressionKindBoolLiteral: break;
		case expressionKindUnaryPlus:
		case expressionKindUnaryMinus:
		case expressionKindUnaryNot:
		case expressionKindBitComplement: symbolEXP(t, e->val.unary); break;
		case expressionKindAddition:
		case expressionKindSubtraction:
		case expressionKindMultiplication:
		case expressionKindDivision:
		case expressionKindOr:
		case expressionKindAnd:
		case expressionKindEquals:
		case expressionKindNotEquals:
		case expressionKindLower:
		case expressionKindLeq:
		case expressionKindGreater:
		case expressionKindGeq:
		case expressionKindBitwiseOr:
		case expressionKindBitwiseXor:
		case expressionKindModulo:
		case expressionKindLeftShift:
		case expressionKindRightShift:
		case expressionKindBitwiseAnd:
		case expressionKindBitwiseAndNot: symbolEXP(t, e->val.binary.lhs); symbolEXP(t, e->val.binary.rhs); break;
		case expressionKindBitClear: break;
		case expressionKindTypeCast:
		case expressionKindFunctionCall:
			symbolEXP(t, e->val.funCall.funName); break;
			symbolList(t, e->val.funCall.args); break;
		case expressionKindAppend:
			symbolEXP(t, e->val.append.appendto);
			symbolEXP(t, e->val.append.appendfrom);
			break;
		case expressionKindLen:
		case expressionKindCap: symbolEXP(t, e->val.expression); break;
		case expressionKindArrayAccess:
			symbolEXP(t, e->val.arrayAccess.identifier);
		case expressionKindStructFieldAccess:
			symbolEXP(t, e->val.structAccess.lhs);
			getSymbol(t, e->val.structAccess.fieldIdent);
		}
	}
}

SYMBOL * makeSymbolFromType(SymbolTable * t, TYPE *type){
	switch (type->kind) {
	case unknownType: return getSymbol(t, type->val.ident);
	case structType: return makeSymbol_struct("", NULL, type);
	case sliceType: return makeSymbol_slice("", makeSymbolFromType(t, type->val.sliceEntryType), type);
	case arrayType: return makeSymbol_array("", makeSymbolFromType(t, type->val.arrayData.entryType), type);
	default: return NULL;
	}
}

void symbolSTMT(SymbolTable * t, STMT * s){
	if (s != NULL) {
		switch (s->kind) {
		case statementKindEmpty: //trivially well-typed
			break;
		case statementKindBlock:
			t = scopeSymbolTable(t);
			symbolSTMT(t, s->val.stmtlist);
			t = unscopeSymbolTable(t);
			break;
		case statementKindExpression:
			symbolEXP(t, s->val.expression);
			break;
		case statementKindAssign:
			symbolList(t, s->val.assignStmt.lhs);
			symbolList(t, s->val.assignStmt.rhs);
			break;
		case statementKindAssignOp:
			symbolEXP( t, s->val.assignopStmt.identifier);
			symbolEXP( t, s->val.assignopStmt.expression);
			break;
		case statementKindShortAssign:
			//printf(""); //quell switch case "expected expression" error
		{
			List * currLHS = s->val.assignStmt.lhs;
			List * currRHS = s->val.assignStmt.rhs;
			while (currLHS != NULL && currLHS != NULL) {
				if (strcmp((currLHS->current.expression->val.identifier),"_") != 0 && !isDeclared(t,currLHS->current.expression->val.identifier)) {
					putSymbol(t, makeSymbol_var(currLHS->current.expression->val.identifier, currRHS->current.expression->type));
				}
				symbolEXP(t, currRHS->current.expression);
				currLHS = currLHS->nextli;
				currRHS = currRHS->nextli;
			}
		}
		break;
		case statementKindVarDeclaration:
			// currS = s;
			if (s != NULL && s->kind == statementKindVarDeclaration && s->val.declStmt.identifiers != NULL) {
				// Takes each identifier in identifier list one at a time
				List * currID = s->val.declStmt.identifiers;
				while (currID != NULL) {
					if (strcmp(currID->current.id, "_") != 0)
						putSymbol(t, makeSymbol_var(currID->current.id, s->val.declStmt.type));
					currID = currID->nextli;
				}
				symbolSTMT(t, s->val.declStmt.nextd);
			}

			break;
		case statementKindTypeDeclaration:
			if (s != NULL && s->kind == statementKindTypeDeclaration && s->val.typeDecl.identifier != NULL) {
				if (s->val.typeDecl.type != NULL) {
					if (strcmp(s->val.typeDecl.identifier, "_") != 0) {
						switch (s->val.typeDecl.type->kind) {
						case intType:
						case floatType:
						case runeType:
						case stringType:
						case boolType:
						case nullType: break;
						case structType:
							putSymbol(t, makeSymbol_type( s->val.typeDecl.identifier,
							                              makeSymbol_struct("",
							                                                makeSymbolList(t, s->val.typeDecl.type->val.fields),
							                                                s->val.typeDecl.type
							                                                )));
							break;
						case arrayType:
							putSymbol(t, makeSymbol_type( s->val.typeDecl.identifier,
							                              makeSymbol_array("",
							                                               makeSymbolFromType(t, s->val.typeDecl.type->val.arrayData.entryType),
							                                               s->val.typeDecl.type
							                                               )));
							break;
						case sliceType:
							putSymbol(t, makeSymbol_type( s->val.typeDecl.identifier,
							                              makeSymbol_slice("",
							                                               makeSymbolFromType(t, s->val.typeDecl.type->val.sliceEntryType),
							                                               s->val.typeDecl.type
							                                               )));
							break;
						case unknownType:
							putSymbol(t,makeSymbol_type(s->val.typeDecl.identifier, getSymbol(t, s->val.typeDecl.type->val.ident)));
							break;
						}
					}

				}
				symbolSTMT(t, s->val.typeDecl.nextT);
			}
			break;
		case statementKindIncrement:
		case statementKindDecrement: // typecheck base exp and exp is of numeric type
			symbolEXP(t,s->val.incdec.exp);
			break;
		case statementKindPrint:
			symbolList(t, s->val.printList);
			break;
		case statementKindPrintln:
			symbolList(t, s->val.printList);
			break;
		case statementKindReturn:
			symbolEXP(t, s->val.expression);
			break;
		case statementKindIf:
			//typecheck initialization
			//typecheck condition expression
			//typecheck statements in first block
			// typecheck statements in second block, if existant
			// statements of each case typechecks
			if (s->val.ifStmt.initStmt != NULL) {
				t = scopeSymbolTable(t);
				symbolSTMT(t, s->val.ifStmt.initStmt);
				t = unscopeSymbolTable(t);
			}
			if (s->val.ifStmt.condition != NULL) symbolEXP(t, s->val.ifStmt.condition);
			if (s->val.ifStmt.elseBody != NULL) {
				symbolSTMT(t,s->val.ifStmt.elseBody);
			}
			if (s->val.ifStmt.elseif != NULL) {
				symbolSTMT(t, s->val.ifStmt.elseif);
			}
			break;
		case statementKindSwitch:
			if (s->val.switchStmt.initStmt != NULL) {
				t = scopeSymbolTable(t);
				symbolSTMT(t, s->val.switchStmt.initStmt);
				t = unscopeSymbolTable(t);
			}
			if (s->val.switchStmt.switchExp != NULL) symbolEXP(t, s->val.switchStmt.switchExp);
			if (s->val.switchStmt.caseStmts != NULL) symbolSWITCHCASE(t, s->val.switchStmt.caseStmts);
			break;
		case statementKindFor:
			if (s->val.forStmt.lhs != NULL) {
				t = scopeSymbolTable(t);
				symbolSTMT(t, s->val.forStmt.lhs);
				t = unscopeSymbolTable(t);
			}
			symbolEXP(t, s->val.forStmt.cond);
			symbolSTMT(t, s->val.forStmt.rhs);
			break;
		case statementKindBreak: break;
		case statementKindContinue: break;
		}
		symbolSTMT(t, s->next);
	}

}

void symbolSWITCHCASE(SymbolTable * t, SWITCHCASE *s){
	if (s != NULL) {
		t = scopeSymbolTable(t);
		symbolSTMT(t, s->statement);
		symbolSWITCHCASE(t, s->next);
		t = unscopeSymbolTable(t);
	}
}

void symbolPROG(SymbolTable * t, PROG *p) {
	if (p != NULL) {
		if (printSymbolTable)
			printf("{\n");
		
		putSymbol(t, makeSymbol_base("int", makeTYPE_baseType(intType)));
		putSymbol(t, makeSymbol_base("float64", makeTYPE_baseType(floatType)));
		putSymbol(t, makeSymbol_base("bool", makeTYPE_baseType(boolType)));
		putSymbol(t, makeSymbol_base("rune", makeTYPE_baseType(runeType)));
		putSymbol(t, makeSymbol_base("string", makeTYPE_baseType(stringType)));
		putSymbol(t, makeSymbol_const("true", makeTYPE_baseType(boolType)));
		putSymbol(t, makeSymbol_const("false",makeTYPE_baseType(boolType)));
		scopeDepth++;
		if (printSymbolTable)
			printf("\t{\n");
		t = scopeSymbolTable(t);
		symbolTOPSTMT(t, p->body);
		t = unscopeSymbolTable(t);
		if (printSymbolTable)
			printf("\t}\n");
		scopeDepth--;
		if (printSymbolTable)
			printf("}\n");
	}
}

void symbolTOPSTMT(SymbolTable * t, TOPSTMT *ts) {
	if (ts != NULL) {
		TOPSTMT *curr = ts;
		while (curr != NULL) {
			if (curr->isFunDecl) {
				//TODO: Check for main function special cases

				SYMBOLLIST * paramList = makeSymbolList(t, curr->val.funDecl.paramList);
				if (strcmp(curr->val.funDecl.funName, "_" ) != 0 && strcmp(curr->val.funDecl.funName, "init" ) != 0) {
					putSymbol(t, makeSymbol_function(curr->val.funDecl.funName, paramList, curr->val.funDecl.result));
				}
				t = scopeSymbolTable(t);
				SYMBOLLIST * currS = paramList;
				while (currS != NULL && currS->currSym != NULL) {
					if (strcmp(currS->currSym->name, "_" ) != 0)
						putSymbol(t, currS->currSym);
					currS = currS->next;
				}
				if (curr->val.funDecl.body != NULL) {
					symbolSTMT(t, curr->val.funDecl.body->val.stmtlist);
				}
				t = unscopeSymbolTable(t);
			}
			else {
				symbolSTMT( t, curr->val.orig);
			}
			curr = curr->next;
		}
	}
}
