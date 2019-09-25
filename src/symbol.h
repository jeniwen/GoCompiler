#define HashSize 317
#ifndef SYMBOL_H
#define SYMBOL_H

struct SYMBOL;

typedef enum SymbolKind {
	symkind_var,
	symkind_func,
	symkind_type,
	symkind_const,
	symkind_struct,
	symkind_array,
	symkind_slice,
	symkind_base
} SymbolKind;

typedef enum BaseType {
	symtype_int,
	symtype_float,
	symtype_rune,
	symtype_string,
	symtype_bool,
} BaseType;


typedef struct SYMBOLLIST {
	struct SYMBOL * currSym;
	struct SYMBOLLIST * next;
} SYMBOLLIST;

typedef struct SYMBOL {
	char *name;
	SymbolKind kind;
	union {
		TYPE * type;
		struct {struct SYMBOL * underlying; TYPE * type;} defined; //for defined types
		struct {SYMBOLLIST * paramList; TYPE *returnType;} functiondec;
		struct {SYMBOLLIST * fieldList; TYPE * structtype;} structdec;
		struct {struct SYMBOL * entrytype; TYPE * slicetype;} slicedec;
		struct {struct SYMBOL * entrytype; TYPE * arraytype;} arraydec;
	} typelit;
	EXP *assignedVal;
	struct SYMBOL *next;
} SYMBOL;

typedef struct SymbolTable {
	SYMBOL *table[HashSize];
	struct SymbolTable *parent;
} SymbolTable;


void printSymbolParamToString(SYMBOL * s);


SymbolTable *initSymbolTable();
void printCurrTable(SymbolTable *t);
void printSymbol(SYMBOL *s);
char * symbolKindToString(SymbolKind s);
int Hash(char *str);
bool declaredLocally(SymbolTable *t, char *name);
SymbolTable *scopeSymbolTable(SymbolTable *s);
SymbolTable * unscopeSymbolTable(SymbolTable *s);
SYMBOL *putSymbol(SymbolTable *t, SYMBOL * sym);
SYMBOL *getSymbol(SymbolTable *t, char *name);
SymbolKind getSymbolType(SymbolTable *t, char *name);
void symbolIdentifierRef(SymbolTable *st, char *id);
void symbolList();
void symbolEXP(SymbolTable * t, EXP * e);
void symbolSTMT(SymbolTable * t, STMT * s);
void symbolSWITCHCASE(SymbolTable * t, SWITCHCASE *s);
void symbolPROG (SymbolTable *st, PROG *p);
void symbolTOPSTMT(SymbolTable * t, TOPSTMT *s);
bool isBaseType(TYPE * type);
SYMBOL * makeSymbol_var(char * name, TYPE * type);
SYMBOL * makeSymbol_const(char * name, TYPE * type);
SYMBOL * makeSymbol_function(char * name, SYMBOLLIST * paramList, TYPE *returnType);
SYMBOL * makeSymbol_type(char * name, SYMBOL * underlying);
SYMBOL * makeSymbol_array(char * name, SYMBOL * entrytype, TYPE * arraytype);
SYMBOL * makeSymbol_slice(char * name, SYMBOL * entrytype, TYPE * slicetype);
SYMBOL * makeSymbol_struct(char * name, SYMBOLLIST * fieldList, TYPE * structtype);
//SYMBOL * makeSymbol_base(TYPE * type);
SYMBOL * makeSymbol_base(char * name, TYPE * type);
SYMBOLLIST * makeSymbolList(SymbolTable * t, List * l);
char * typeName (TYPE * type);
SYMBOL * typeToSymbol(SymbolTable * t, char * name, TYPE * type);
bool isDeclared(SymbolTable *t, char *name);
TYPE * resolveType(SymbolTable * t, SYMBOL * sym);
SYMBOL * makeSymbolFromType(SymbolTable * t, TYPE *type);
#endif
