#include <stdlib.h>
#include "tree.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

extern int yylineno;

EXP *makeEXP_identifier(char *id){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = expressionKindIdentifier;
	e->val.identifier = id;
	return e;
}
EXP *makeEXP_intLiteral(int intLiteral){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = expressionKindIntLiteral;
	e->val.intLiteral = intLiteral;
	e->type = makeTYPE_baseType(intType);
	return e;
}
EXP *makeEXP_floatLiteral(float floatLiteral){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = expressionKindFloatLiteral;
	e->val.floatLiteral = floatLiteral;
	e->type = makeTYPE_baseType(floatType);
	return e;
}
EXP *makeEXP_runeLiteral(char * charLiteral){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = expressionKindRuneLiteral;
	e->val.runeLiteral = charLiteral;
	e->type = makeTYPE_baseType(runeType);
	return e;
}
EXP *makeEXP_boolLiteral(bool boolLiteral){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = expressionKindBoolLiteral;
	e->val.boolLiteral = boolLiteral;
	e->type = makeTYPE_baseType(boolType);
	return e;
}
EXP *makeEXP_stringLiteral(char* stringLiteral){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = expressionKindStringLiteral;
	e->val.stringLiteral = stringLiteral;
	e->type = makeTYPE_baseType(stringType);
	return e;
}
EXP *makeEXP_binary(ExpressionKind op, EXP *lhs, EXP *rhs){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = op;
	e->val.binary.lhs = lhs;
	e->val.binary.rhs = rhs;
	return e;
}
EXP *makeEXP_unary(ExpressionKind op, EXP *unaryVal){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = op;
	e->val.unary = unaryVal;
	return e;
}
EXP *makeEXP_funCall(EXP* funName, List* arguments){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = expressionKindFunctionCall;
	e->val.funCall.funName = funName;
	e->val.funCall.args = arguments;
	return e;
}
EXP *makeEXP_append(EXP *appendto, EXP* appendfrom){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = expressionKindAppend;
	e->val.append.appendto = appendto;
	e->val.append.appendfrom = appendfrom;
	return e;
}
EXP *makeEXP_lencap(ExpressionKind op, EXP * expression){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = op;
	e->val.expression = expression;
	return e;
}
EXP *makeEXP_arrayAccess(EXP* arrayName, EXP* index){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = expressionKindArrayAccess;
	e->val.arrayAccess.identifier = arrayName;
	e->val.arrayAccess.index = index;
	return e;
}
EXP *makeEXP_structFieldAccess(EXP *lhs, char* fieldIdent){
	EXP *e = malloc(sizeof(EXP));
	e->lineno = yylineno;
	e->kind = expressionKindStructFieldAccess;
	e->val.structAccess.lhs = lhs;
	e->val.structAccess.fieldIdent = fieldIdent;
	return e;
}
PROG *makeProg(char* packageName, TOPSTMT* body){
	PROG *p =  malloc(sizeof(PROG));
	p->packageName = packageName;
	p->body = body;
	return p;
}

TOPSTMT *makeTOPSTMT_funDecl(char* funName, List* paramList, TYPE* result, STMT* body){
	TOPSTMT *topStmt = malloc(sizeof(TOPSTMT));
	topStmt->lineno = yylineno;
	topStmt->isFunDecl = true;
	topStmt->val.funDecl.funName = funName;
	topStmt->val.funDecl.paramList = paramList;
	topStmt->val.funDecl.result = result;
	topStmt->val.funDecl.body = body;
	return topStmt;
}
TOPSTMT *makeTOPSTMT_vartypeDecl(STMT *orig){
	TOPSTMT *topStmt = malloc(sizeof(TOPSTMT));
	topStmt->lineno = yylineno;
	topStmt->isFunDecl = false;
	topStmt->val.orig = orig;
	return topStmt;
}
// TOPSTMT *makeTOPSTMT_typeDecl(char *identifier, TYPE* type){
//      TOPSTMT *topStmt = malloc(sizeof(TOPSTMT));
//      topStmt->lineno = yylineno;
//      topStmt->funDecl = false;
//      topStmt->val.declaration = makeSTMT_typeDecl(identifier, type);
//      return topStmt;
// }

STMT *makeSTMT_empty(){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindEmpty;
	return s;
}
STMT *makeSTMT_expStatement(EXP* expression){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindExpression;
	s->val.expression = expression;
	return s;
}
STMT *makeSTMT_assignSTMT(List* lhs, List* rhs){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindAssign;
	s->val.assignStmt.lhs = lhs;
	s->val.assignStmt.rhs = rhs;
	return s;
}
STMT *makeSTMT_assignopSTMT(EXP* identifier, AssignOpKind aop, EXP *expression){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindAssignOp;
	s->val.assignopStmt.aop = aop;
	s->val.assignopStmt.identifier = identifier;
	s->val.assignopStmt.expression = expression;
	return s;
}
STMT *makeSTMT_shortassignSTMT(List* lhs, List* rhs){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindShortAssign;
	s->val.assignStmt.lhs = lhs;
	s->val.assignStmt.rhs = rhs;
	return s;
}
STMT *makeSTMT_incdecSTMT(EXP * exp, bool inc){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindIncrement;
	s->val.incdec.exp = exp;
	s->val.incdec.inc = inc;
	return s;
}
STMT *makeSTMT_print(StatementKind kind, List* toPrint){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = kind;
	s->val.printList = toPrint;
	return s;
}
STMT *makeSTMT_typeDecl(char* identifier, TYPE* type){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindTypeDeclaration;
	s->val.typeDecl.identifier = identifier;
	s->val.typeDecl.type = type;
	return s;
}
STMT *makeSTMT_varDecl(List* identifiers, TYPE* type, List* values){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindVarDeclaration;
	s->val.declStmt.identifiers = identifiers;
	s->val.declStmt.type = type;
	s->val.declStmt.values = values;
	return s;
}
STMT *makeSTMT_ifStmt(STMT* initStmt, EXP* condition, STMT* ifBody, STMT* elseBody, STMT* elseif){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindIf;
	s->val.ifStmt.initStmt = initStmt;
	s->val.ifStmt.condition = condition;
	s->val.ifStmt.ifBody = ifBody;
	s->val.ifStmt.elseBody = elseBody;
	s->val.ifStmt.elseif = elseif;
	return s;
}
STMT *makeSTMT_switchStmt(STMT *initStmt, EXP* switchExp,  SWITCHCASE* caseStmts){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindSwitch;
	s->val.switchStmt.initStmt = initStmt;
	s->val.switchStmt.switchExp = switchExp;
	s->val.switchStmt.caseStmts = caseStmts;
	return s;
}
SWITCHCASE *makeSWITCHCASE(List *exprlist, STMT *statement, bool isDefault){
	SWITCHCASE *s = malloc(sizeof(SWITCHCASE));
	s->lineno = yylineno;
	s->isDefault = isDefault;
	// s->kind = statementKindCase;
	s->exprlist = exprlist;
	s->statement = statement;
	return s;
}
STMT *makeSTMT_block(STMT * stmtlist) {
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindBlock;
	s->val.stmtlist = stmtlist;
	return s;
}
STMT *makeSTMT_forStmt(EXP* cond, STMT* lhs, STMT* rhs, STMT* body){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindFor;
	s->val.forStmt.cond = cond;
	s->val.forStmt.lhs = lhs;
	s->val.forStmt.rhs = rhs;
	s->val.forStmt.body = body;
	return s;
}
STMT *makeSTMT_returnStmt(EXP* val){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindReturn;
	s->val.expression = val;
	return s;
}
STMT *makeSTMT_breakStmt() {
	STMT *s = malloc(sizeof(STMT));
	s->kind = statementKindBreak;
	return s;
}
STMT *makeSTMT_continueStmt() {
	STMT *s = malloc(sizeof(STMT));
	s->kind = statementKindContinue;
	return s;
}
STMT *makeSTMT_increment(EXP* val){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindIncrement;
	s->val.expression = val;
	return s;
}
STMT *makeSTMT_decrement(EXP* val){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindDecrement;
	s->val.expression = val;
	return s;
}
STMT *makeSTMT_break(){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindBreak;
	return s;
}
STMT *makeSTMT_continue(){
	STMT *s = malloc(sizeof(STMT));
	s->lineno = yylineno;
	s->kind = statementKindContinue;
	return s;
}
List* makeList_exp(EXP* expression){
	List *l = malloc(sizeof(List));
	l->kind = listKindExp;
	l->current.expression = expression;
	return l;
}
List* makeList_ident(char* ident){
	List *l = malloc(sizeof(List));
	l->kind = listKindId;
	l->current.id = ident;
	return l;
}
List* makeList_param(List * idlist, TYPE* type){
	List *l = malloc(sizeof(List));
	l->kind = listKindParam;
	l->current.param.idlist = idlist;
	l->current.param.type = type;
	return l;
}
List* makeList_varspec(List * idlist, TYPE* type, List * explist){
	List *l = malloc(sizeof(List));
	l->kind = listKindVarSpeclist;
	l->current.vardec.idlist = idlist;
	l->current.vardec.type = type;
	l->current.vardec.explist = explist;
	return l;
}
TYPE *makeTYPE_struct(List* fields){
	TYPE *t = malloc(sizeof(TYPE));
	t->kind = structType;
	t->isParen = false;
	t->val.fields = fields;
	return t;
}
TYPE *makeTYPE_array(TYPE *entryType, int capacity){
	TYPE *t = malloc(sizeof(TYPE));
	t->kind = arrayType;
	t->isParen = false;
	t->val.arrayData.entryType = entryType;
	t->val.arrayData.capacity = capacity;
	return t;
}
TYPE *makeTYPE_slice(TYPE *entryType){
	TYPE *t = malloc(sizeof(TYPE));
	t->kind = sliceType;
	t->isParen = false;
	t->val.sliceEntryType = entryType;
	return t;
}
TYPE *makeTYPE_ident(char* identifier){
	TYPE *t = malloc(sizeof(TYPE));
	t->isParen = false;
	t->kind = unknownType;
	t->val.ident = identifier;
	return t;
}

TYPE * baseInt = NULL;
TYPE * baseFloat = NULL;
TYPE * baseString = NULL;
TYPE * baseRune = NULL;
TYPE * baseBool = NULL;
TYPE *makeTYPE_baseType(TypeKind type){
	switch (type) {
	case intType:
		if(baseInt)
			return baseInt;
		else{
			TYPE *t = malloc(sizeof(TYPE));
			t->isParen = false;
			t->kind = type;
			baseInt = t;
			return t;
		}
		break;
	case floatType:
		if(baseFloat)
			return baseFloat;
		else{
			TYPE *t = malloc(sizeof(TYPE));
			t->isParen = false;
			t->kind = type;
			baseFloat = t;
			return t;
		}
		break;
	case runeType:
		if(baseRune)
			return baseRune;
		else{
			TYPE *t = malloc(sizeof(TYPE));
			t->isParen = false;
			t->kind = type;
			baseRune = t;
			return t;
		}
		break;
	case stringType:
		if(baseString)
			return baseString;
		else{
			TYPE *t = malloc(sizeof(TYPE));
			t->isParen = false;
			t->kind = type;
			baseString = t;
			return t;
		}
		break;
	case boolType:
		if(baseBool)
			return baseBool;
		else{
			TYPE *t = malloc(sizeof(TYPE));
			t->isParen = false;
			t->kind = type;
			baseBool = t;
			return t;
		}
		break;
	default: fprintf(stderr, "Error in tree.c for make basetype."); exit(1);
	}
}
TYPE *makeTYPE_parenType(TYPE * origtype){
	TYPE *t = malloc(sizeof(TYPE));
	t->isParen = false;
	t->val.origtype = origtype;
	return t;
}
