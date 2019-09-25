#include "typecheck.h"
#include "tree.h"
#include "symbol.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>


STRUCTLIST * sl;
STRUCTLIST * head;
// STRUCTLIST ** structlistptr;

void typecheck_PROG(PROG * root, STRUCTLIST ** structlistptr){
	SymbolTable * t = initSymbolTable();
	putSymbol(t, makeSymbol_base("int", makeTYPE_baseType(intType)));
	putSymbol(t, makeSymbol_base("float64", makeTYPE_baseType(floatType)));
	putSymbol(t, makeSymbol_base("bool", makeTYPE_baseType(boolType)));
	putSymbol(t, makeSymbol_base("rune", makeTYPE_baseType(runeType)));
	putSymbol(t, makeSymbol_base("string", makeTYPE_baseType(stringType)));
	putSymbol(t, makeSymbol_const("true", makeTYPE_baseType(boolType)));
	putSymbol(t, makeSymbol_const("false",makeTYPE_baseType(boolType)));
	sl = malloc(sizeof(STRUCTLIST));
	*structlistptr = sl;
	sl->next = NULL;
	head = sl;
	t = scopeSymbolTable(t);
	typecheck_TOPSTMT(t, root->body);
	t = scopeSymbolTable(t);
	// structlistptr = &structlist;

	free(t);
}

bool checkStructEquality(TYPE * t1, TYPE *t2, int lineno){
	if(t1) {
		switch(t1->kind) {
		case intType:
		case floatType:
		case runeType:
		case stringType:
		case boolType:
		case nullType:
		case unknownType:
			if(t1 != t2) {
				return false;
			}
			break;
		case structType:
			if(t2 &&  t2->kind == structType && t1->val.fields && t2->val.fields) {
				List * lineLhs = t1->val.fields;
				List * lineRhs = t2->val.fields;
				List * identLhs = NULL;
				List * identRhs = NULL;
				while(lineLhs && lineRhs) {
					if(!checkStructEquality(lineLhs->current.param.type, lineLhs->current.param.type, lineno)) return false;
					if (!identLhs)
						identLhs = lineLhs->current.param.idlist;
					if (!identRhs)
						identRhs = lineRhs->current.param.idlist;
					while(identLhs && identRhs) {
						if (strcmp(identLhs->current.id, identRhs->current.id) != 0) {
							return false;
						}
						identLhs = identLhs->nextli;
						identRhs = identRhs->nextli;
					}
					if(!identLhs)
						lineLhs = lineLhs->nextli;
					if(!identRhs)
						lineRhs = lineRhs->nextli;
				}
				if(lineLhs || lineRhs) {
					return false;
				}
				break;
			}
			else{
				return false;
			}
		case arrayType:
			if(!t2 || t2->kind != arrayType || t1->val.arrayData.capacity != t2->val.arrayData.capacity) {
				return false;
			}
			else
				return checkStructEquality(t1->val.arrayData.entryType, t2->val.arrayData.entryType, lineno);
			break;
		case sliceType:
			if(!t2 || t2->kind != sliceType || t1->val.sliceEntryType != t2->val.sliceEntryType) {
				return false;
			}
			break;
		}
	}
	else if(t2) {
		return false;
	}
	return true;
}

void addStruct(TYPE * t) {
	if (t) {
		if (t->kind != structType) {
			fprintf(stderr, "Error: adding a non-struct to the list of structs");
		} else {
			if (sl) {
				t->rt =t;
				STRUCTLIST * currPointer = head;
				while (currPointer){
					if(checkStructEquality(t, currPointer->curr, 0)){
						t->struct_id = currPointer->struct_id;
						return;
					}
					currPointer = currPointer->next;
				}
				// printf("adding struct %d\n", sl->struct_id);
				int id = sl->struct_id;
				STRUCTLIST * l = malloc(sizeof(STRUCTLIST));
				l->struct_id = id+1;
				t->struct_id = id+1;
				l->curr = t;
				sl->next = l;
				sl = sl->next;
				sl->next = NULL;
			}
		}
	}
}


void printErrType(TYPE * t){
	if(t) {
		switch (t->kind) {
		case intType: fprintf(stderr, "int");
			break;
		case floatType: fprintf(stderr, "float");
			break;
		case runeType: fprintf(stderr, "rune");
			break;
		case stringType: fprintf(stderr, "string");
			break;
		case boolType: fprintf(stderr, "bool");
			break;
		case nullType: fprintf(stderr, "???");
			break;
		case unknownType: fprintf(stderr, "%s (identifier)", t->val.ident);
			break;
		case arrayType: fprintf(stderr, "array");
			break;
		case sliceType: fprintf(stderr, "slice");
			break;
		case structType: fprintf(stderr, "struct");
			break;
		}
	}
	else
		fprintf(stderr, "void");
}
void typecheck_TOPSTMT(SymbolTable * t, TOPSTMT * s){

	if(s) {
		if(s->isFunDecl) {
			if(strcmp(s->val.funDecl.funName, "main") == 0 || strcmp(s->val.funDecl.funName, "init") == 0) {
				if(s->val.funDecl.paramList || s->val.funDecl.result) {
					fprintf(stderr, "Error: (line %d) %s is a special function in GoLite, it cannot have parameters or a return type. \n", s->lineno, s->val.funDecl.funName);
					exit(1);
				}
			}
			
			SYMBOL * f = makeSymbol_function(s->val.funDecl.funName, NULL, NULL);
			if(strcmp(s->val.funDecl.funName, "init") != 0 )
				putSymbol(t, f);
			//Check valid return type
			if(s->val.funDecl.result){
				s->val.funDecl.result = checkValidType(t, s->val.funDecl.result, s->lineno);
			}
			
			f->typelit.functiondec.returnType = s->val.funDecl.result;

			//Check valid types of params:
			if (s->val.funDecl.paramList) {
				List * currL =  s->val.funDecl.paramList;
				while (currL) {
					currL->current.param.type = checkValidType(t, currL->current.param.type, s->lineno);
					currL = currL->nextli;
				}
			}

			SYMBOLLIST * paramList = makeSymbolList(t, s->val.funDecl.paramList);
			f->typelit.functiondec.paramList = paramList;

			t = scopeSymbolTable(t);
			SYMBOLLIST * currS = paramList;
			while (currS != NULL && currS->currSym != NULL) {
				putSymbol(t, currS->currSym);
				currS = currS->next;
			}
			if (s->val.funDecl.body != NULL && s->val.funDecl.body->val.stmtlist) {
				int ret = typecheck_STMT(t, s->val.funDecl.body->val.stmtlist, s->val.funDecl.result, 1);
				if (ret == 1 && s->val.funDecl.result) {
					fprintf(stderr, "Error: (line %d) function %s has a return type but might not return a value \n", s->lineno, s->val.funDecl.funName);
					exit(1);
				}
			}
			else if(s->val.funDecl.result) {
				fprintf(stderr, "Error: (line %d) function has a return type but no body \n", s->lineno);
				exit(1);
			}
			t = unscopeSymbolTable(t);
		}
		else{
			typecheck_STMT( t, s->val.orig, NULL, 0);
		}
		typecheck_TOPSTMT(t, s->next);
	}
}


int typecheck_STMT(SymbolTable * t, STMT * s, TYPE * returnType, int returnLast){
	if (s) {
		switch(s->kind) {
		case statementKindBlock:
			t = scopeSymbolTable(t);
			returnLast = typecheck_STMT(t, s->val.stmtlist, returnType, returnLast);
			t = unscopeSymbolTable(t);
			break;
		case statementKindVarDeclaration:
			typecheck_var_decl(t, s);
			break;
		case statementKindExpression:
			typecheck_EXP(t, s->val.expression);
			if(s->val.expression->kind != expressionKindFunctionCall) {
				fprintf(stderr, "Error: (line %d) Expression statement has to be a function call \n", s->lineno);
				exit(1);
			}
			break;
		case statementKindReturn:
			if (s->val.expression) {
				typecheck_EXP(t, s->val.expression);
				checkTypeEquality(returnType,s->val.expression->type, s->lineno);
			}
			else
				checkTypeEquality(returnType, NULL, s->lineno);
			if(s->next) {
				fprintf(stderr, "Error: (line %d) Statements after a function return \n", s->lineno);
				exit(1);
			}
			returnLast = 0;
			break;
		case statementKindAssign:
			typecheck_Assign(t, s);
			break;
		case statementKindAssignOp:
			typecheck_OpAssign(t, s);
			break;
		case statementKindShortAssign:
			typecheck_shortAssign(t, s);
			break;
		case statementKindFor:
			if(typecheck_forLoop(t, s, returnType) == 0) {
				returnLast = 0;
			}
			break;
		case statementKindTypeDeclaration:
			typecheck_type_decl(t, s);
			break;
		case statementKindIncrement:
			typecheck_EXP(t,s->val.incdec.exp);
			checkNumericType(t, s->val.incdec.exp->type, s->val.incdec.exp->lineno);
			checkLvalue(t,s->val.incdec.exp);
			//check if type resolves to a numeric base type (int, float64, rune)
			break;
		case statementKindDecrement:
			typecheck_EXP(t,s->val.incdec.exp);
			checkNumericType(t, s->val.incdec.exp->type, s->val.incdec.exp->lineno);
			//check if type resolves to a numeric base type (int, float64, rune)
			break;
		case statementKindPrint:
			typecheck_Print(t, s);
			break;
		case statementKindPrintln:
			typecheck_Print(t, s);
			break;
		case statementKindIf:
		{
			t = scopeSymbolTable(t);
			typecheck_STMT(t, s->val.ifStmt.initStmt, returnType, 0);
			typecheck_EXP(t, s->val.ifStmt.condition);
			checkTypeEquality(resolve_type_with_TYPE(t, s->val.ifStmt.condition->type), makeTYPE_baseType(boolType), s->lineno);
			int x = 0;
			if(s->val.ifStmt.ifBody)
				x += typecheck_STMT(t, s->val.ifStmt.ifBody, returnType, 1);
			if(s->val.ifStmt.elseBody)
				x += typecheck_STMT(t, s->val.ifStmt.elseBody, returnType, 1);
			if(s->val.ifStmt.elseif)
				x += typecheck_STMT(t, s->val.ifStmt.elseif, returnType, 1);
			if(!s->val.ifStmt.elseBody && !s->val.ifStmt.elseif)
				x = 1;
			if (x == 0)
				returnLast = 0;
			t = unscopeSymbolTable(t);
		}
		break;
		case statementKindSwitch:
			t = scopeSymbolTable(t);
			typecheck_STMT(t, s->val.switchStmt.initStmt, returnType, 0);
			int x;
			if ( s->val.switchStmt.switchExp) {
				typecheck_EXP(t, s->val.switchStmt.switchExp);
				checkComparable(t,  s->val.switchStmt.switchExp->type, s->lineno);
				x = typecheck_SWITCHCASE(t, s->val.switchStmt.caseStmts, s->val.switchStmt.switchExp->type, returnType);
			}
			else{
				x = typecheck_SWITCHCASE(t, s->val.switchStmt.caseStmts, makeTYPE_baseType(boolType), returnType);
			}
			if(x == 0)
				returnLast =0;
			t = unscopeSymbolTable(t);
			break;
		case statementKindEmpty:
			break;
		case statementKindBreak:
			returnLast = 0;
		case statementKindContinue:
			break;
		}
		if(s->next) {
			return typecheck_STMT(t, s->next, returnType, returnLast);
		}
		else{
			return returnLast;
		}
	}
	return returnLast;
}

void typecheck_var_decl(SymbolTable * t, STMT * s){
	if(s) {
		if(s->val.declStmt.type) {
			s->val.declStmt.type = checkValidType(t,s->val.declStmt.type, s->lineno);
		}
		if(s->val.declStmt.values) {
			List * currID = s->val.declStmt.identifiers;
			List * currVal = s->val.declStmt.values;
			while (currID != NULL) {
				if(strcmp(currID->current.id, "init") == 0 ||strcmp(currID->current.id, "main") == 0) {
					fprintf(stderr, "Error: (line %d) Can't create a variable with identifier main/init, those are reserved keywords\n", s->lineno);
					exit(1);
				}
				typecheck_EXP(t, currVal->current.expression);
				if(!currVal->current.expression->type) {
					fprintf(stderr, "Error: (line %d) Can't declare a variable with value void \n", s->lineno);
					exit(1);
				}
				if(s->val.declStmt.type) {
					//check type compatibility
					checkTypeEquality(s->val.declStmt.type, currVal->current.expression->type, s->lineno);
					putSymbol(t, makeSymbol_var(currID->current.id, s->val.declStmt.type));
				}
				else{

					putSymbol(t, makeSymbol_var(currID->current.id, currVal->current.expression->type));


				}
				currID = currID->nextli;
				currVal = currVal->nextli;
			}
		}
		else{
			List * currID = s->val.declStmt.identifiers;
			while (currID != NULL) {
				if(strcmp(currID->current.id, "init") == 0 ||strcmp(currID->current.id, "main") == 0) {
					fprintf(stderr, "Error: (line %d) Can't create a variable with identifier main/init, those are reserved keywords\n", s->lineno);
					exit(1);
				}
				putSymbol(t, makeSymbol_var(currID->current.id, s->val.declStmt.type));
				currID = currID->nextli;
			}
		}
		typecheck_var_decl(t, s->val.declStmt.nextd);
	}
}

void endlessRecursion(SymbolTable * t, SYMBOL * sym, TYPE * t2, int lineno){
	switch (t2->kind) {
	case structType: {
		List * fields = t2->val.fields;
		while (fields) {
			if(fields->current.param.type == sym->typelit.defined.type) {
				fprintf(stderr, "Error: (line %d) structure contains an exact number of itself, endless recursion\n", lineno);
				exit(1);
			}
			else{
				endlessRecursion(t, sym, fields->current.param.type, lineno);
			}
			fields = fields->nextli;
		}
	}
	break;
	case arrayType: {
		if(sym->typelit.defined.type == t2->val.arrayData.entryType) {
			fprintf(stderr, "Error: (line %d) new type contains an exact number of itself, endless recursion\n", lineno);
			exit(1);
		}
		else{
			endlessRecursion(t, sym, t2->val.arrayData.entryType, lineno);
		}
	}
	break;
	default:
		break;
	}
}

void typecheck_type_decl(SymbolTable *t, STMT *s){
	if(s) {
		if(s->val.typeDecl.type) {
			if(strcmp( s->val.typeDecl.identifier, "init") == 0 ||strcmp( s->val.typeDecl.identifier, "main") == 0) {
				fprintf(stderr, "Error: (line %d) Can't define a type with identifier main/init, those are reserved keywords\n", s->lineno);
				exit(1);
			}
			switch (s->val.typeDecl.type->kind) {
			case intType:
			case floatType:
			case runeType:
			case stringType:
			case boolType:
			case nullType: break;
			case structType:
			{
				SYMBOL *symStruct = makeSymbol_struct("", NULL,NULL);
				SYMBOL *symType = makeSymbol_type(s->val.typeDecl.identifier,symStruct);
				putSymbol(t, symType);
				symStruct->typelit.structdec.fieldList = makeSymbolList(t, s->val.typeDecl.type->val.fields);
				symStruct->typelit.structdec.structtype = checkValidType(t, s->val.typeDecl.type, s->lineno);
				endlessRecursion(t, symType, symStruct->typelit.structdec.structtype, s->lineno);
			}
			break;
			case arrayType:
			{
				SYMBOL *symArray = makeSymbol_array("", NULL,NULL);
				SYMBOL *symType = makeSymbol_type(s->val.typeDecl.identifier,symArray);
				putSymbol(t, symType);
				symArray->typelit.arraydec.entrytype = makeSymbolFromType(t, s->val.typeDecl.type->val.arrayData.entryType);
				symType->typelit.defined.type->rt = makeTYPE_baseType(intType);
				symArray->typelit.arraydec.arraytype = checkValidType(t, s->val.typeDecl.type, s->lineno);
				symType->typelit.defined.type->rt = symArray->typelit.arraydec.arraytype;
				endlessRecursion(t, symType, symArray->typelit.arraydec.arraytype, s->lineno);
			}
			break;
			case sliceType:
			{
				SYMBOL *symSlice = makeSymbol_slice("", NULL,NULL);
				SYMBOL *symType = makeSymbol_type(s->val.typeDecl.identifier,symSlice);
				putSymbol(t, symType);
				symSlice->typelit.slicedec.entrytype = makeSymbolFromType(t,s->val.typeDecl.type->val.sliceEntryType);
				symType->typelit.defined.type->rt = makeTYPE_baseType(intType);
				symSlice->typelit.slicedec.slicetype = checkValidType(t, s->val.typeDecl.type, s->lineno);
				symType->typelit.defined.type->rt = symSlice->typelit.slicedec.slicetype;
			}
			break;
			case unknownType:
			{
				SYMBOL *sym  = getSymbol(t, s->val.typeDecl.type->val.ident);
				if(sym->kind == symkind_var || sym->kind == symkind_func) {
					fprintf(stderr, "Error: (line %d) Can't declare a new type with %s as it is not a type. \n", s->lineno,  s->val.typeDecl.type->val.ident);
					exit(1);
				}
				if(strcmp("_", s->val.typeDecl.identifier) != 0){
					SYMBOL *tempsym = makeSymbol_type(s->val.typeDecl.identifier,sym );
					putSymbol(t,tempsym);
					tempsym->typelit.defined.type->rt = resolve_type_with_TYPE(t,tempsym->typelit.defined.type);
				}
			}
			break;
			}
			if(s->val.typeDecl.type->rt == NULL) s->val.typeDecl.type->rt = resolve_type_with_TYPE(t, s->val.typeDecl.type);
			typecheck_type_decl(t, s->val.typeDecl.nextT);
		}
	}
}

int typecheck_SWITCHCASE(SymbolTable * t, SWITCHCASE * caseStmts, TYPE * type, TYPE * returnType){
	int toRet;
	if(caseStmts) {
		if (caseStmts->isDefault) {
			t = scopeSymbolTable(t);
			toRet = typecheck_STMT(t, caseStmts->statement, returnType, 1);
			t = unscopeSymbolTable(t);
			if (toRet == 0) {
				typecheck_SWITCHCASE(t, caseStmts->next, type, returnType);
				return 0;
			}
		}
		else{
			List * current = caseStmts->exprlist;
			while (current) {
				typecheck_EXP(t, current->current.expression);
				checkTypeEquality(type, current->current.expression->type, caseStmts->lineno);
				current = current->nextli;
			}
			t = scopeSymbolTable(t);
			toRet = typecheck_STMT(t, caseStmts->statement, returnType, 1);
			t = unscopeSymbolTable(t);
		}
		if(toRet == 0)
			return typecheck_SWITCHCASE(t, caseStmts->next, type, returnType);
		else {
			typecheck_SWITCHCASE(t, caseStmts->next, type, returnType);
			return toRet;
		}
	}
	return 1;
}

int typecheck_forLoop(SymbolTable * t, STMT* s, TYPE * returnType){
	t = scopeSymbolTable(t);

	typecheck_STMT(t, s->val.forStmt.lhs, returnType, 0);

	if(s->val.forStmt.cond) {
		typecheck_EXP(t, s->val.forStmt.cond);
		checkTypeEquality(makeTYPE_baseType(boolType), resolve_type_with_TYPE(t, s->val.forStmt.cond->type), s->lineno);
	}
	typecheck_STMT(t, s->val.forStmt.rhs, returnType, 0);
	int a = typecheck_STMT(t, s->val.forStmt.body, returnType, 1);
	t = unscopeSymbolTable(t);
	if(!s->val.forStmt.cond && a==1)
		return 0;
	else
		return 1;
}

void typecheck_Assign(SymbolTable * t, STMT* s){
	if(s) {
		List * curLhs = s->val.assignStmt.lhs;
		List * curRhs = s->val.assignStmt.rhs;
		while(curLhs) {
			if (curLhs->current.expression->kind == expressionKindIdentifier && strcmp(curLhs->current.expression->val.identifier, "_") ==0) {
				typecheck_EXP(t, curRhs->current.expression);
				curLhs->current.expression->type = curRhs->current.expression->type;
			}
			else{
				typecheck_EXP(t, curLhs->current.expression);
				checkLvalue(t, curLhs->current.expression);
				typecheck_EXP(t, curRhs->current.expression);
				checkTypeEquality(curLhs->current.expression->type, curRhs->current.expression->type, s->lineno);
			}
			curLhs = curLhs->nextli;
			curRhs = curRhs->nextli;
		}
	}
}

void typecheck_OpAssign(SymbolTable * t, STMT* s){
	if(s) {
		typecheck_EXP(t, s->val.assignopStmt.identifier);
		checkLvalue(t, s->val.assignopStmt.identifier);
		typecheck_EXP(t, s->val.assignopStmt.expression);
		checkTypeEquality(s->val.assignopStmt.identifier->type, s->val.assignopStmt.expression->type,s->lineno);
		checkOpCompatibility(t,  s->val.assignopStmt.identifier, s->val.assignopStmt.aop,  s->val.assignopStmt.expression);
	}
}

void typecheck_Print(SymbolTable * t, STMT * s){
	if(s) {
		List * current = s->val.printList;
		while(current) {
			typecheck_EXP(t, current->current.expression);
			if(!current->current.expression->type) {
				fprintf(stderr, "Error: (line %d) Can't print expression of type void\n", s->lineno);
				exit(1);
			}
			TYPE * rt = resolve_type_with_TYPE(t, current->current.expression->type);
			if (!isBaseType(rt)) {
				fprintf(stderr, "Error: (line %d) an expression doesn't resolve to a base type\n", s->lineno);
				exit(1);
			}
			current->current.expression->type->rt = rt;
			current = current->nextli;
		}
	}
}

void checkNumericType(SymbolTable * t, TYPE * type, int lineno){
	TYPE * res = resolve_type_with_TYPE(t, type);
	switch (res->kind) {
	case intType:
	case floatType:
	case runeType:
		break;
	case stringType:
	case boolType:
	case nullType:
	case structType:
	case arrayType:
	case sliceType:
	case unknownType:
		fprintf(stderr, "Error: (line %d) the type doesn't resolve to a numeric type\n", lineno);
		exit(1);
	}
}
void checkNumericOrStringType(SymbolTable * t, TYPE * type, int lineno){
	TYPE * res = resolve_type_with_TYPE(t, type);
	switch (res->kind) {
	case intType:
	case floatType:
	case runeType:
	case stringType:
		break;
	case boolType:
	case nullType:
	case structType:
	case arrayType:
	case sliceType:
	case unknownType:
		fprintf(stderr, "Error: (line %d) the type doesn't resolve to a numeric or string type\n", lineno);
		exit(1);
	}
}

TYPE * checkValidType(SymbolTable * t, TYPE * type, int lineno){
	if(type) {
		SYMBOL * s;
		List * fields;
		switch (type->kind) {
		case unknownType:
			s = getSymbol(t, type->val.ident);
			if(!s || (s->kind != symkind_type && s->kind != symkind_base)) {
				fprintf(stderr, "Error: (line %d) %s is not a valid type\n", lineno, type->val.ident);
				exit(1);
			}
			else{
				free(type);
				if (s->kind == symkind_type) {
					if(!s->typelit.defined.type->rt)
						s->typelit.defined.type->rt = resolve_type_with_TYPE(t, s->typelit.defined.type);
					return s->typelit.defined.type;
				}
				else {
					s->typelit.type->rt = resolve_type_with_TYPE(t, s->typelit.type);
					return s->typelit.type;
				}
			}
			break;
		case nullType:    fprintf(stderr, "Error: (line %d) Nulltype, should not happen, something has gone wrong\n", lineno);
			exit(1);
		case structType:
			fields = type->val.fields;
			while (fields) {
				fields->current.param.type = checkValidType(t, fields->current.param.type, lineno);
				if(fields->current.param.type->rt == NULL)
					fields->current.param.type->rt = resolve_type_with_TYPE(t, fields->current.param.type);
				fields = fields->nextli;
			}
			type-> rt = type;
			addStruct(type);
			break;
		case arrayType:
			type->val.arrayData.entryType = checkValidType(t, type->val.arrayData.entryType, lineno);
			if (!type->val.arrayData.entryType->rt)
				type->val.arrayData.entryType->rt = resolve_type_with_TYPE(t, type->val.arrayData.entryType);
			checkValidType(t, type->val.arrayData.entryType->rt, lineno);
			type-> rt = type;
			break;
		case sliceType:
			type->val.sliceEntryType = checkValidType(t, type->val.sliceEntryType, lineno);
			type-> rt = type;
			break;
		case intType:
		case floatType:
		case runeType:
		case stringType:
		case boolType:
			type-> rt = type;
			break;
		}
	}
	return type;
}


void typecheck_shortAssign(SymbolTable * t, STMT * s){
	int decCounter = 0;
	List * currLHS = s->val.assignStmt.lhs;
	List * currRHS = s->val.assignStmt.rhs;
	while (currLHS && currRHS) {
		typecheck_EXP(t, currRHS->current.expression);
		if (currRHS->current.expression->type == NULL) {
			fprintf(stderr, "Error: (line %d) Right hand side value has no type\n", s->lineno);
			exit(1);
		}
		currRHS->current.expression->type->rt = resolve_type_with_TYPE(t, currRHS->current.expression->type);
		List * currLHS2 = s->val.assignStmt.lhs;
		while (currLHS2 != currLHS) {
			if(strcmp(currLHS->current.expression->val.identifier, currLHS2->current.expression->val.identifier) == 0) {
				fprintf(stderr, "Error: (line %d) identifier %s appears more than once in the short declaration\n", s->lineno,currLHS->current.expression->val.identifier );
				exit(1);
			}
			currLHS2 = currLHS2->nextli;
		}
		if (!declaredLocally(t,currLHS->current.expression->val.identifier)) {
			if(strcmp(currLHS->current.expression->val.identifier, "_") != 0) {
				putSymbol(t, makeSymbol_var(currLHS->current.expression->val.identifier, currRHS->current.expression->type));
				currLHS->isDeclared = true;
				decCounter++;
			}
			currLHS->current.expression->type = currRHS->current.expression->type;
		}
		else{
			SYMBOL * sym = getSymbol(t, currLHS->current.expression->val.identifier);
			if (sym->kind == symkind_var) {
				checkTypeEquality(sym->typelit.type, currRHS->current.expression->type, s->lineno);
				currLHS->isDeclared = false;
				currLHS->current.expression->type = sym->typelit.type;
			}
			else{
				fprintf(stderr, "Error: (line %d) %s is not a variable, can't assign a value to it\n", s->lineno, currLHS->current.expression->val.identifier);
				exit(1);
			}
		}
		currLHS = currLHS->nextli;
		currRHS = currRHS->nextli;
	}
	if (decCounter == 0) {
		fprintf(stderr, "Error: (line %d) short declaration statement doesn't declare a new variable\n", s->lineno);
		exit(1);
	}
}

void checkOpCompatibility(SymbolTable *t, EXP* identifier, AssignOpKind aop, EXP * expression){
	EXP * e = malloc(sizeof(EXP));
	e->lineno = identifier->lineno;
	switch (aop) {
	case assignopKindRemainder:
		e->kind = expressionKindModulo;
		break;
	case assignopKindMultiplication:
		e->kind = expressionKindMultiplication;
		break;
	case assignopKindDivision:
		e->kind = expressionKindDivision;
		break;
	case assignopKindPlus:
		e->kind = expressionKindAddition;
		break;
	case assignopKindMinus:
		e->kind = expressionKindSubtraction;
		break;
	case assignopKindLShift:
		e->kind = expressionKindLeftShift;
		break;
	case assignopKindRShift:
		e->kind = expressionKindRightShift;
		break;
	case assignopKindBWXor:
		e->kind =expressionKindBitwiseXor;
		break;
	case assignopKindBWAndNot:
		e->kind = expressionKindBitwiseAndNot;
		break;
	case assignopKindBWAnd:
		e->kind = expressionKindBitwiseAnd;
		break;
	case assignopKindBWOr:
		e->kind = expressionKindBitwiseOr;
		break;
	}
	e->val.binary.lhs = identifier;
	e->val.binary.rhs = expression;
	e->type = NULL;
	e->paren = false;
	typecheck_EXP(t, e);
	checkTypeEquality(identifier->type, e->type, e->lineno);
	free(e);
}

void checkLvalue(SymbolTable *t, EXP * e){
	if(e) {
		switch(e->kind) {
		case expressionKindIdentifier: {
			SYMBOL * sym = getSymbol(t, e->val.identifier);
			if (sym->kind != symkind_var) {
				fprintf(stderr, "Error: (line %d) can't assign to %s\n", e->lineno, e->val.identifier);
				exit(1);
			}
		}
		case expressionKindStructFieldAccess:
			if(e->val.structAccess.lhs->kind == expressionKindFunctionCall) {
				fprintf(stderr, "Error: (line %d) expression can't be a left value because the structure is not addressable\n", e->lineno);
				exit(1);
			}
			break;
		case expressionKindArrayAccess: {
			if(e->val.arrayAccess.identifier->type->kind == arrayType && e->val.arrayAccess.identifier->kind == expressionKindFunctionCall) {
				fprintf(stderr, "Error: (line %d) expression can't be a left value because the array is not addressable\n", e->lineno);
				exit(1);
			}
			break;
		}
		default:
			fprintf(stderr, "Error: (line %d) expression can't be a left value\n", e->lineno);
			exit(1);
		}
	}
}
void checkComparable(SymbolTable *t, TYPE * type, int lineno){
	if(type) {
		switch(type->kind) {
		case intType:
		case floatType:
		case runeType:
		case stringType:
		case boolType:
		case nullType:
			break;
		case structType:
		{
			List * fieldList = type->val.fields;
			while(fieldList) {
				checkComparable(t, fieldList->current.param.type, lineno);
				fieldList = fieldList->nextli;
			}
		}
		break;
		case arrayType:
			checkComparable(t, type->val.arrayData.entryType, lineno);
			break;
		case sliceType:
			fprintf(stderr, "Error: (line %d) slices aren't comparable\n", lineno);
			exit(1);
		case unknownType:
			checkComparable(t, resolve_type_with_TYPE(t, type), lineno);
			break;
		}
	}
	else{
		fprintf(stderr, "Error: (line %d) void isn't comparable\n", lineno);
		exit(1);
	}
}

void checkOrderable(SymbolTable *t, TYPE * type, int lineno) {
	if(type) {
		switch(type->kind) {
		case intType:
		case floatType:
		case runeType:
		case stringType:
		case nullType:
			break;
		case boolType:
			fprintf(stderr, "Error: (line %d) bool not orderable.\n", lineno);
			exit(1);
			break;
		case structType:
			fprintf(stderr, "Error: (line %d) structs not orderable. \n", lineno);
			exit(1);
			break;
		case arrayType:
			fprintf(stderr, "Error: (line %d) arrays not orderable. \n", lineno);
			exit(1);
			break;
		case sliceType:
			fprintf(stderr, "Error: (line %d) slices not orderable. \n", lineno);
			exit(1);
			break;
		case unknownType:
			checkOrderable(t, resolve_type_with_TYPE(t, type), lineno);
			break;
		}
	}
}


void checkTypeEquality(TYPE * t1, TYPE *t2, int lineno){
	if(t1) {
		switch(t1->kind) {
		case intType:
		case floatType:
		case runeType:
		case stringType:
		case boolType:
		case nullType:
		case unknownType:
			if(t1 != t2) {
				fprintf(stderr, "Error: (line %d) type equality constraint failed:", lineno);
				printErrType(t1);
				fprintf(stderr, " and ");
				printErrType(t2);
				fprintf(stderr, "\n");
				exit(1);
			}
			break;
		case structType:
			if(t2 &&  t2->kind == structType && t1->val.fields && t2->val.fields) {
				List * lineLhs = t1->val.fields;
				List * lineRhs = t2->val.fields;
				List * identLhs = NULL;
				List * identRhs = NULL;
				while(lineLhs && lineRhs) {
					checkTypeEquality(lineLhs->current.param.type, lineLhs->current.param.type, lineno);
					if (!identLhs)
						identLhs = lineLhs->current.param.idlist;
					if (!identRhs)
						identRhs = lineRhs->current.param.idlist;
					while(identLhs && identRhs) {
						if (strcmp(identLhs->current.id, identRhs->current.id) != 0) {
							fprintf(stderr, "Error: (line %d) the two structure types aren't compatible because %s and %s aren't the same\n", lineno, identLhs->current.id, identRhs->current.id);
							exit(1);
						}
						identLhs = identLhs->nextli;
						identRhs = identRhs->nextli;
					}
					if(!identLhs)
						lineLhs = lineLhs->nextli;
					if(!identRhs)
						lineRhs = lineRhs->nextli;
				}
				if(lineLhs || lineRhs) {
					fprintf(stderr, "Error: (line %d) the two structure types aren't compatible\n", lineno);
					exit(1);
				}
				break;
			}
			else{
				fprintf(stderr, "Error: (line %d) one type is a structure and the other one isn't\n", lineno);
				exit(1);
			}
		case arrayType:
			if(!t2 || t2->kind != arrayType || t1->val.arrayData.capacity != t2->val.arrayData.capacity) {
				fprintf(stderr, "Error: (line %d) the two array types aren't compatible\n", lineno);
				exit(1);
			}
			else
				checkTypeEquality(t1->val.arrayData.entryType, t2->val.arrayData.entryType, lineno);
			break;
		case sliceType:
			if(!t2 || t2->kind != sliceType){
				fprintf(stderr, "Error: (line %d) the two slice types aren't compatible\n", lineno);
				exit(1);
			}
			else
				checkTypeEquality(t1->val.sliceEntryType, t2->val.sliceEntryType, lineno);
			break;
		}
	}
	else if(t2) {
		fprintf(stderr, "Error: (line %d) type equality constraint failed:", lineno);
		printErrType(t1);
		fprintf(stderr, " and ");
		printErrType(t2);
		fprintf(stderr, "\n");
		exit(1);
	}
}

TYPE * resolve_type_with_TYPE(SymbolTable *t, TYPE * type){
	if(type) {
		SYMBOL *s;
		switch (type->kind) {
		case intType:
		case floatType:
		case runeType:
		case stringType:
		case boolType:
		case nullType:
		case structType:
		case arrayType:
		case sliceType:
			return type;
		case unknownType:
			s = getSymbol(t, type->val.ident);
			if(s) {
				TYPE* temp = resolveType(t,  s);
				type->rt = temp;
				return temp;
			}
			break;
		}
		fprintf(stderr, "Error: resolve_type_with_TYPE returned NULL"); exit(1);
	}
	return NULL;
}

void typecheck_EXP(SymbolTable * t, EXP * e){
	if(e && !e->type) {
		SYMBOL * sym;
		List* fields;
		List* idlist;
		TYPE* tempType1;
		TYPE* tempType2 = NULL;
		List* args;
		switch (e->kind) {
		case expressionKindIdentifier:
			sym = getSymbol(t, e->val.identifier);
			if (sym == NULL) {
				fprintf(stderr, "Error: (line %d) undeclared variable %s\n", e->lineno, e->val.identifier);
				exit(1);
			}
			if(strcmp(e->val.identifier,"_") == 0)
			{
				fprintf(stderr, "Error: (line %d) Blank identifier not valid in expressions\n", e->lineno);
				exit(1);
			}
			switch(sym->kind)
			{
			case symkind_var:
				e->type = sym->typelit.type;
				break;
			case symkind_const:
				e->type = makeTYPE_baseType(boolType);
				break;
			case symkind_struct:
				e->type = sym->typelit.structdec.structtype;
				break;
			case symkind_array:
				e->type = sym->typelit.arraydec.arraytype;
				break;
			case symkind_slice:
				e->type = sym->typelit.slicedec.slicetype;
				break;
			default:
				fprintf(stderr, "Error: (line %d) Invalid identifier\n", e->lineno);
				exit(1);
			}
			break;
		case expressionKindIntLiteral:
			e->type = makeTYPE_baseType(intType);
			break;
		case expressionKindFloatLiteral:
			e->type = makeTYPE_baseType(floatType);
			break;
		case expressionKindRuneLiteral:
			e->type = makeTYPE_baseType(runeType);
			break;
		case expressionKindStringLiteral:
			e->type = makeTYPE_baseType(stringType);
			break;
		case expressionKindBoolLiteral:
			e->type = makeTYPE_baseType(boolType);
			break;
		case expressionKindUnaryPlus:
		case expressionKindUnaryMinus:
			typecheck_EXP(t, e->val.unary);
			e->type = e->val.unary->type;
			checkNumericType(t, e->type, e->lineno);
			break;
		case expressionKindUnaryNot:
			typecheck_EXP(t, e->val.unary);
			e->type = e->val.unary->type;
			if(resolve_type_with_TYPE(t, e->type) != makeTYPE_baseType(boolType))
			{
				fprintf(stderr, "Error: (line %d) Unary NOT may only be applied to expressions which resolve to a boolean\n", e->lineno);
				exit(1);
			}
			break;
		case expressionKindBitComplement:
			typecheck_EXP(t, e->val.unary);
			tempType1 = resolve_type_with_TYPE(t,e->val.unary->type);
			if(tempType1->kind != intType && tempType1->kind != runeType)
			{
				fprintf(stderr, "Error: (line %d) Bit Complement may only be applied to expressions which resolve to a integer\n", e->lineno);
				exit(1);
			}
			e->type = e->val.unary->type;
			break;
		case expressionKindAddition:
			typecheck_EXP(t, e->val.binary.lhs);
			typecheck_EXP(t, e->val.binary.rhs);
			checkTypeEquality(e->val.binary.lhs->type,e->val.binary.rhs->type, e->lineno);
			e->type = e->val.binary.lhs->type;
			checkNumericOrStringType(t, e->type, e->lineno);
			break;
		case expressionKindSubtraction:
		case expressionKindMultiplication:
		case expressionKindDivision:
			typecheck_EXP(t, e->val.binary.lhs);
			typecheck_EXP(t, e->val.binary.rhs);
			checkTypeEquality(e->val.binary.lhs->type,e->val.binary.rhs->type, e->lineno);
			e->type = e->val.binary.lhs->type;
			checkNumericType(t, e->type, e->lineno);
			break;
		case expressionKindOr:
		case expressionKindAnd:
			typecheck_EXP(t, e->val.binary.lhs);
			typecheck_EXP(t, e->val.binary.rhs);
			checkTypeEquality(e->val.binary.lhs->type,e->val.binary.rhs->type, e->lineno);
			e->type = e->val.binary.lhs->type;
			if(resolve_type_with_TYPE(t, e->val.binary.lhs->type) != makeTYPE_baseType(boolType))
			{
				fprintf(stderr, "Error: (line %d) Boolean binary operands must resolve to a boolean type\n", e->lineno);
				exit(1);
			}
			break;
		case expressionKindEquals:
		case expressionKindNotEquals:
			typecheck_EXP(t, e->val.binary.lhs);
			typecheck_EXP(t, e->val.binary.rhs);
			checkComparable(t,  e->val.binary.lhs->type, e->lineno);
			checkTypeEquality(e->val.binary.lhs->type,e->val.binary.rhs->type, e->lineno);
			e->type = e->val.binary.lhs->type;
			/*
			   if(e->type->kind != boolType && e->type->kind != intType && e->type->kind != floatType && e->type->kind != stringType && e->type->kind != structType && e->type->kind != arrayType)
			   {
			        fprintf(stderr, "Error: (line %d) Types must be comparable\n", e->lineno);
			        exit(1);
			   }
			 */
			e->type = makeTYPE_baseType(boolType);
			break;
		case expressionKindLower:
		case expressionKindLeq:
		case expressionKindGreater:
		case expressionKindGeq:
			typecheck_EXP(t, e->val.binary.lhs);
			typecheck_EXP(t, e->val.binary.rhs);
			checkTypeEquality(e->val.binary.lhs->type,e->val.binary.rhs->type, e->lineno);
			e->type = e->val.binary.lhs->type;
			checkOrderable(t, e->type, e->lineno);
			e->type = makeTYPE_baseType(boolType);
			break;
		case expressionKindBitwiseOr:
		case expressionKindBitwiseXor:
		case expressionKindModulo:
		case expressionKindLeftShift:
		case expressionKindRightShift:
		case expressionKindBitwiseAnd:
		case expressionKindBitwiseAndNot:
		case expressionKindBitClear:
			typecheck_EXP(t, e->val.binary.lhs);
			typecheck_EXP(t, e->val.binary.rhs);
			checkTypeEquality(e->val.binary.lhs->type,e->val.binary.rhs->type, e->lineno);
			e->type = e->val.binary.lhs->type;
			{
				TYPE * type = resolve_type_with_TYPE(t,e->type);
				if(type != makeTYPE_baseType(intType) && type != makeTYPE_baseType(runeType))
				{
					fprintf(stderr, "Error: (line %d) Operands must use types which resolve to integers\n", e->lineno);
					exit(1);
				}
			}
			break;
		case expressionKindTypeCast:
		case expressionKindFunctionCall:
			if(e->val.funCall.funName->type != expressionKindIdentifier)
			{
				fprintf(stderr, "Error: (line %d) Expected identifier", e->lineno);
				exit(1);
			}

			sym = getSymbol(t, e->val.funCall.funName->val.identifier);

			if(sym->kind == symkind_func)
			{
				e->type = sym->typelit.functiondec.returnType;
				SYMBOLLIST * paramList;
				if(e->val.funCall.args && e->val.funCall.args->kind != listKindExp)
				{
					fprintf(stderr, "Error: (line %d) Expected expression list\n", e->lineno);
					exit(1);
				}
				for(paramList = sym->typelit.functiondec.paramList, args = e->val.funCall.args; paramList->currSym != NULL && args != NULL; args = args->nextli, paramList = paramList->next)
				{
					typecheck_EXP(t, args->current.expression);
					checkTypeEquality(paramList->currSym->typelit.type, args->current.expression->type, e->lineno);
				}
				if(paramList->currSym)
				{
					fprintf(stderr, "Error: (line %d) Too few arguments in function call\n", e->lineno);
					exit(1);
				}
				if(args != NULL)
				{
					fprintf(stderr, "Error: (line %d) Too many arguments in function call\n", e->lineno);
					exit(1);
				}

				e->isCast = false;
			}
			else if(sym->kind == symkind_type || sym->kind == symkind_base)
			{

				TYPE * res = resolveType(t, sym);
				if(!res) {
					fprintf(stderr, "Error: (line %d) resolved type to null\n", e->lineno);
					exit(1);
				}
				if(!isBaseType(res)) {
					fprintf(stderr, "Error: (line %d) can only typecast to base types\n", e->lineno);
					exit(1);
				}
				e->type = res;
				e->type->rt = res;
				if(!e->val.funCall.args) {
					fprintf(stderr, "Error: (line %d) The typecast doesn't contain an expression to be cast\n", e->lineno);
					exit(1);
				}
				typecheck_EXP(t, e->val.funCall.args->current.expression);
				if(e->val.funCall.args->nextli != NULL)
				{
					fprintf(stderr, "Error: (line %d) Too many arguments in function call\n", e->lineno);
					exit(1);
				}
				tempType1 = resolve_type_with_TYPE(t, e->val.funCall.args->current.expression->type);
				if(tempType1->kind == e->type->kind)
				{

				}
				else if(e->type->kind == stringType && (tempType1->kind == intType || tempType1->kind == runeType))
				{

				}
				else if((e->type->kind == intType || e->type->kind == floatType || e->type->kind == runeType) && (tempType1->kind == intType || tempType1->kind == floatType || tempType1->kind == runeType))
				{

				}
				else
				{
					fprintf(stderr, "Error: (line %d) Types may not be converted against each other\n", e->lineno);
					exit(1);
				}
				e->kind = expressionKindTypeCast;
				if(sym->kind == symkind_base)
					e->type = sym->typelit.type;
				if(sym->kind == symkind_type)
					e->type = sym->typelit.defined.type;

				e->isCast = true;
			}
			else
			{
				fprintf(stderr, "Error: (lineno %d) invalid identifier for function call\n", e->lineno);
				exit(1);
			}
			break;
		case expressionKindAppend:
			typecheck_EXP(t, e->val.append.appendto);
			typecheck_EXP(t, e->val.append.appendfrom);
			tempType1 = resolve_type_with_TYPE(t, e->val.append.appendto->type);
			if(tempType1->kind != sliceType)
			{
				fprintf(stderr, "Error: (line %d) Left argument of append must be slice type\n", e->lineno);
				exit(1);
			}
			tempType2 = e->val.append.appendfrom->type;
			checkTypeEquality(tempType2, tempType1->val.sliceEntryType, e->lineno);
			/*
			   if(tempType2 != e->val.append.appendto->type->val.sliceEntryType)
			   {

			        fprintf(stderr, "Error: (line %d) Right argument of append must match slice type\n", e->lineno);
			        exit(1);
			   }
			 */
			e->type = e->val.append.appendto->type;
			break;
		case expressionKindLen:
		{
			typecheck_EXP(t, e->val.expression);
			TYPE * type = resolve_type_with_TYPE(t, e->val.expression->type);
			if(type->kind != arrayType && type->kind != sliceType && type->kind != stringType) {
				fprintf(stderr, "Error: (line %d) Can only get the length of a slice, a string or an array\n", e->lineno);
				exit(1);
			}
			e->type = makeTYPE_baseType(intType);
		}
		break;
		case expressionKindCap:
		{
			typecheck_EXP(t, e->val.expression);
			TYPE * type = resolve_type_with_TYPE(t, e->val.expression->type);
			if(type->kind != arrayType && type->kind != sliceType) {
				fprintf(stderr, "Error: (line %d) Can only get the capacity of a slice or an array\n", e->lineno);
				exit(1);
			}
			e->type = makeTYPE_baseType(intType);
		}
		break;
		case expressionKindArrayAccess:
			typecheck_EXP(t, e->val.arrayAccess.identifier);
			typecheck_EXP(t, e->val.arrayAccess.index);
			tempType1 = resolve_type_with_TYPE(t,e->val.arrayAccess.identifier->type);
			if(tempType1->kind == arrayType)
			{
				e->type = tempType1->val.arrayData.entryType;
			}
			else if(tempType1->kind == sliceType)
			{
				e->type = tempType1->val.sliceEntryType;
			}
			else
			{
				fprintf(stderr, "Error: (line %d) Indexed type must be array or slice\n", e->lineno);
				exit(1);
			}
			if(resolve_type_with_TYPE(t, e->val.arrayAccess.index->type) != makeTYPE_baseType(intType))
			{
				fprintf(stderr, "Error: (line %d) Array index must have int type\n", e->lineno);
				exit(1);
			}
			break;
		case expressionKindStructFieldAccess:
			typecheck_EXP(t, e->val.structAccess.lhs);
			tempType1 = resolve_type_with_TYPE(t,e->val.structAccess.lhs->type);
			if(tempType1->kind != structType)
			{
				fprintf(stderr, "Error: (line %d) Must have struct type to access field\n", e->lineno);
				exit(1);
			}

			for(fields = tempType1->val.fields; fields != NULL; fields = fields->nextli)
			{
				for(idlist = fields->current.param.idlist; idlist != NULL; idlist = idlist->nextli)
				{
					if(strcmp(e->val.structAccess.fieldIdent, idlist->current.id) == 0)
					{
						e->type = fields->current.param.type;
						break;
					}
				}

				if(e->type != NULL)
				{
					break;
				}
			}
			if(e->type == NULL)
			{
				fprintf(stderr, "Error: (line %d) Field undefined\n", e->lineno);
				exit(1);
			}
			break;
		}
	}
}
