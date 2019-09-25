#ifndef TREE_H
#define TREE_H
#include <stdbool.h>

typedef enum {
	intType,
	floatType,
	runeType,
	stringType,
	boolType,
	nullType,
	structType,
	arrayType,
	sliceType,
	unknownType
} TypeKind;


typedef enum {
	expressionKindIdentifier,
	expressionKindIntLiteral,
	expressionKindFloatLiteral,
	expressionKindRuneLiteral,
	expressionKindStringLiteral,
	expressionKindBoolLiteral,
	expressionKindUnaryPlus,
	expressionKindUnaryMinus,
	expressionKindUnaryNot,
	expressionKindBitComplement,
	expressionKindAddition,
	expressionKindSubtraction,
	expressionKindMultiplication,
	expressionKindDivision,
	expressionKindOr,
	expressionKindAnd,
	expressionKindEquals,
	expressionKindNotEquals,
	expressionKindLower,
	expressionKindLeq,
	expressionKindGreater,
	expressionKindGeq,
	expressionKindBitwiseOr,
	expressionKindBitwiseXor,
	expressionKindModulo,
	expressionKindLeftShift,
	expressionKindRightShift,
	expressionKindBitwiseAnd,
	expressionKindBitwiseAndNot,
	expressionKindBitClear,
	expressionKindFunctionCall,
	expressionKindAppend,
	expressionKindLen,
	expressionKindCap,
	expressionKindArrayAccess,
	expressionKindStructFieldAccess,
	expressionKindTypeCast
} ExpressionKind;


typedef struct EXP EXP;
typedef struct List List;
typedef struct STMT STMT;
typedef struct TOPSTMT TOPSTMT;
typedef struct TYPE TYPE; //struct, struct
typedef struct PROG PROG;
typedef struct SWITCHCASE SWITCHCASE;
typedef struct STRUCTLIST STRUCTLIST;
//parameter

struct SWITCHCASE {
	int lineno;
	bool isDefault;
	List * exprlist;
	STMT * statement;
	SWITCHCASE *next;
};

struct EXP {
	int lineno;
	ExpressionKind kind;
	bool paren;
	TYPE * type;
	bool isCast; // For funcalls
	union {
		char *identifier;
		int intLiteral;
		float floatLiteral;
		char* stringLiteral;
		char* runeLiteral;
		bool boolLiteral;
		EXP* unary;
		struct { EXP* identifier; EXP* index;} arrayAccess;
		struct { EXP* lhs; char* fieldIdent;} structAccess;
		struct { EXP *lhs; EXP *rhs; } binary;
		struct { EXP* funName; List* args; } funCall;
		struct { EXP *appendto; EXP* appendfrom;} append;
		EXP * expression; //For len or cap
	} val;
};

typedef enum {
	assignopKindRemainder,
	assignopKindMultiplication,
	assignopKindDivision,
	assignopKindPlus,
	assignopKindMinus,
	assignopKindLShift,
	assignopKindRShift,
	assignopKindBWXor,
	assignopKindBWAndNot,
	assignopKindBWAnd,
	assignopKindBWOr,
} AssignOpKind;

struct TYPE {
	TypeKind kind;
	TypeKind resolveType;
	TYPE * rt;
	bool isParen;
	int struct_id;
	union {
		List* fields; //For structures;
		struct {TYPE *entryType; int capacity;} arrayData;//arrays
		TYPE *sliceEntryType;//For slices
		TYPE *origtype;//For parenthesized types
		char* ident; //If an unknown identifier is used, will be checked in the typechecker
	} val;
};



typedef enum {
	listKindExp,
	listKindParam,
	listKindId,
	listKindVarSpeclist
} ListKind;

struct List {
	ListKind kind;
	bool isDeclared; // From typecheck for codegen
	union {
		EXP* expression;
		struct {List *idlist; TYPE* type;} param;
		struct {List *idlist; TYPE* type; List * explist;} vardec;
		char * id;
	} current;
	List* nextli;
};

typedef enum {
	statementKindEmpty,
	statementKindBlock,
	statementKindExpression,
	statementKindAssign,
	statementKindAssignOp,
	statementKindShortAssign,
	statementKindVarDeclaration,
	statementKindTypeDeclaration,
	statementKindIncrement,
	statementKindDecrement,
	statementKindPrint,
	statementKindPrintln,
	statementKindReturn,
	statementKindIf,
	statementKindSwitch,
	statementKindFor,
	statementKindBreak,
	statementKindContinue
} StatementKind;


struct STMT {
	int lineno;
	StatementKind kind;
	union {
		EXP* expression;
		struct { List* lhs; List* rhs;} assignStmt;
		struct { EXP* identifier; AssignOpKind aop; EXP * expression;} assignopStmt;
		struct { EXP * exp; bool inc;} incdec;
		//char* ident;
		List* printList;
		struct { char* identifier; TYPE* type; STMT *nextT;} typeDecl;
		struct { List* identifiers; TYPE* type; List* values; STMT *nextd;} declStmt;
		struct { STMT* initStmt; EXP* condition; STMT* ifBody; STMT* elseBody; STMT* elseif;} ifStmt;
		struct { STMT* initStmt; EXP* switchExp;  SWITCHCASE* caseStmts;} switchStmt; //cases as statements? //swicth clauses as separate
		struct { EXP* cond; STMT* lhs; STMT* rhs;  STMT* body; } forStmt;
		STMT * stmtlist;
	} val;
	STMT* next;
};


struct TOPSTMT {
	int lineno;
	bool isFunDecl; //True if it's a function decl, otherwise it contains a statement of kind var decl or type decl
	union {
		STMT* orig;
		struct {char* funName; List* paramList; TYPE* result; STMT* body;} funDecl;
	} val;
	TOPSTMT* next;
};

struct PROG {
	char* packageName;
	TOPSTMT* body;
};

struct STRUCTLIST {
	int struct_id;
	TYPE * curr;
	STRUCTLIST * next;
};

EXP *makeEXP_identifier(char *id);
EXP *makeEXP_intLiteral(int intLiteral);
EXP *makeEXP_floatLiteral(float floatLiteral);
EXP *makeEXP_runeLiteral(char * charLiteral);
EXP *makeEXP_boolLiteral(bool boolLiteral);
EXP *makeEXP_stringLiteral(char* stringLiteral);
// EXP *makeEXP_parenEXP(EXP * expression);
EXP *makeEXP_structFieldAccess(EXP* lhs, char* fieldIdent);
EXP *makeEXP_binary(ExpressionKind op, EXP *lhs, EXP *rhs);
EXP *makeEXP_unary(ExpressionKind op, EXP *unaryVal);
EXP *makeEXP_funCall(EXP* funName, List* arguments);
EXP *makeEXP_append(EXP *appendto, EXP* appendfrom);
EXP *makeEXP_lencap(ExpressionKind op, EXP *expression);
EXP *makeEXP_arrayAccess(EXP* arrayName, EXP* index);
PROG *makeProg(char* packageName, TOPSTMT* body);

TOPSTMT *makeTOPSTMT_funDecl(char* funName, List* paramList, TYPE* result, STMT* body);
// TOPSTMT *makeTOPSTMT_varDecl(List* identifiers, TYPE* type, List* values);
// TOPSTMT *makeTOPSTMT_typeDecl(char *identifier, TYPE* type);
TOPSTMT *makeTOPSTMT_vartypeDecl(STMT * varDecl);

STMT *makeSTMT_block(STMT * stmtlist);
STMT *makeSTMT_expStatement(EXP* expression);
STMT *makeSTMT_assignSTMT(List* lhs, List* rhs);
STMT *makeSTMT_assignopSTMT(EXP* identifier, AssignOpKind aokind, EXP * expression);
STMT *makeSTMT_shortassignSTMT(List* lhs, List* rhs);
STMT *makeSTMT_incdecSTMT(EXP * exp, bool inc);
STMT *makeSTMT_print(StatementKind kind, List* toPrint);
STMT *makeSTMT_typeDecl(char* identifier, TYPE* type);
STMT *makeSTMT_varDecl(List* identifiers, TYPE* type, List* values);
STMT *makeSTMT_ifStmt(STMT* initStmt, EXP* condition, STMT* ifBody, STMT* elseBody, STMT* elseif);
STMT *makeSTMT_switchStmt(STMT *initStmt, EXP* switchExp, SWITCHCASE* caseStmts);
SWITCHCASE *makeSWITCHCASE(List *exprlist, STMT *statement, bool isDefault);

STMT *makeSTMT_forStmt(EXP* cond, STMT* lhs, STMT* rhs, STMT* body);
STMT *makeSTMT_returnStmt(EXP* val);
STMT *makeSTMT_breakStmt();
STMT *makeSTMT_continueStmt();

List* makeList_varspec(List * idlist, TYPE* type, List * explist);
List* makeList_exp(EXP* expression);
List* makeList_ident(char* ident);
List* makeList_param(List * idlist, TYPE* type);
// List* makeList_vardecl(STMT * vardec)

TYPE *makeTYPE_struct(List* fields);
TYPE *makeTYPE_array(TYPE *entryType, int capacity);
TYPE *makeTYPE_slice(TYPE *entryType);
TYPE *makeTYPE_ident(char* identifier);
TYPE *makeTYPE_baseType(TypeKind type);
TYPE *makeTYPE_parenType(TYPE *origtype);
#endif /* !TREE_H */
