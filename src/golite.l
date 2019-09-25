%{
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
#include "parser.tab.h" // Token definitions from the parser
#include <string.h>
#include <stdlib.h>

char* from_raw(char* raw_string) // *should* modify raw strings correctly to make them c_strings by adding '\' before escape sequences
{
	int length = strlen(raw_string);
	int escapes = 0;
	int dbquotes = 0;
	for (int i = 0; i < length; i++){
		escapes += (raw_string[i] == '\\');
		dbquotes += (raw_string[i] == '\"');
	}
	char* c_string = (char*)malloc(sizeof(char) * (length + escapes + dbquotes + 1));
	int gap = 0;
	int exclusion = 0;
	//for (int j = 0; j < length + escapes + 1; j++)c_string[j] = (raw_string[j - gap] == '\\' && !(exclusion)) ? '\\' + gap++-gap + ((exclusion^=1)?0:0): raw_string[j - gap] + ((exclusion = 0)?0:0); // I know, being too desparate to get this into one line
	for (int j = 0; j < length + escapes + dbquotes+ 1; j++) {
		if (raw_string[j - gap] == '\\' && !(exclusion)) {
			gap++;
			c_string[j] = '\\';
			exclusion = 1;
		}
		else if ( raw_string[j - gap] == '\"'){
			c_string[j++] = '\\';
			c_string[j] = '\"';
			gap++;
		}
		else {
			c_string[j] = raw_string[j - gap];
			exclusion = 0;
		}
	}
	c_string[0] = 0x22;
	c_string[length + escapes + dbquotes -1] = 0x22;
	return c_string;
}

// Stores the line number with each token sent to bison
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
int gentokens;


//Optional semicolons
//From https://github.com/comp520/Examples/blob/master/flex%2Bbison/optional-semicolon/tiny.l
int lastToken = -1;
#define RETURN(token) lastToken = token; return token;
int needsSemiColon()
{
	return lastToken == tIDENTIFIER ||
	lastToken == tINTVAL ||
	lastToken == tFLOATVAL ||
	lastToken == tRUNEVAL ||
	lastToken == tSTRINGVAL ||
	lastToken == tRPAREN ||
	lastToken == tRBRACE ||
	lastToken == tRBRACKET ||
	lastToken == tBREAK ||
	lastToken == tINC ||
	lastToken == tDEC ||
	lastToken == tRETURN ||
	lastToken == tCONTINUE
	;
}




%}

/* Helper definitions */
DEC_DIG   [0-9]
ID        [a-z][a-z0-9]*
OCT_DIG   [0-7]
HEX_DIG   [0-9A-Fa-f]
ASCII     [\x00-\x7F]
DECIMALS  [0-9]+


ESC_CHAR  (\\)("a"|"b"|"f"|"n"|"r"|"t"|"v"|"\\"|"'")
ESC_CHARSTR  (\\)[abfnrtv"\\]

/* Scanner options, line number generation */
%option yylineno

/* Removes warnings about input/output functions */
%option noinput
%option nounput
%x comment
%%
[ \t\r]+

\n      { if (needsSemiColon()) { if (gentokens) printf("tSEMICOLON\n"); RETURN(tSEMICOLON); } }

"//"[^\n]*	{ if (needsSemiColon()) { if (gentokens) printf("tSEMICOLON\n"); RETURN(tSEMICOLON); } }


"/*"[^*]*"*"+([^/*][^*]*"*"+)*"/" 	{ if (needsSemiColon()) { if (gentokens) printf("tSEMICOLON\n"); RETURN(tSEMICOLON); } }

(0|([1-9]{DEC_DIG}*)) {
	yylval.intval = atoi(yytext);
  if (gentokens) printf("tINTVAL(%s)\n", yytext);
	RETURN(tINTVAL);
}

"0"("x"|"X"){HEX_DIG}{HEX_DIG}* {
	yylval.intval = (int) strtol((yytext), NULL, 16);
  if (gentokens) printf("tINTVAL(%s)\n", yytext);
	RETURN(tINTVAL);
}

"0"{OCT_DIG}* {
	yylval.intval = (int) strtol((yytext), NULL, 8);
	if (gentokens) printf("tINTVAL(%s)\n", yytext);
	RETURN(tINTVAL);
}

({DECIMALS}"."{DECIMALS})|{DECIMALS}+"."|("."{DECIMALS}) {
	yylval.floatval = atof(yytext);
  if (gentokens) printf("tFLOATVAL(%s)\n", yytext);
	RETURN(tFLOATVAL);
}

"'"(([\x00-\x26\x28-\x5B\x5D-\x7F])|{ESC_CHAR})"'" {
	yylval.runeval = strdup(yytext);
  if (gentokens) printf("tRUNEVAL(%s)\n", yytext);
	RETURN(tRUNEVAL);
}

(\")([\x00-\x09\x0B-\x21\x23-\x5B\x5D-\x7F]|{ESC_CHARSTR})*(\") {
	yylval.stringval = strdup(yytext);
  if (gentokens) printf("tSTRINGVAL(%s)\n", yytext);
	RETURN(tSTRINGVAL);
}


"`"([\x00-\x5F\x61-\x7F]|{ESC_CHARSTR})*"`" {
	yylval.stringval = from_raw(yytext);
  if (gentokens) printf("tSTRINGVAL(%s)\n", from_raw(yytext));
	RETURN(tSTRINGVAL);
}

"("     {if (gentokens) printf("tLPAREN\n"); RETURN(tLPAREN);}
")"     {if (gentokens) printf("tRPAREN\n"); RETURN(tRPAREN);}
"{"     {if (gentokens) printf("tLBRACE\n"); RETURN(tLBRACE);}
"}"     {if (gentokens) printf("tRBRACE\n"); RETURN(tRBRACE);}
"["     {if (gentokens) printf("tLBRACKET\n"); RETURN(tLBRACKET);}
"]"     {if (gentokens) printf("tRBRACKET\n"); RETURN(tRBRACKET);}

":"     {if (gentokens) printf("tCOLON\n"); RETURN(tCOLON);}
";"     {if (gentokens) printf("tSEMICOLON\n"); RETURN(tSEMICOLON);}
"."     {if (gentokens) printf("tDOT\n"); RETURN(tDOT);}
","     {if (gentokens) printf("tCOMMA\n"); RETURN(tCOMMA);}

"..."   {if (gentokens) printf("tELLIPSIS\n"); RETURN(tELLIPSIS);}

"++"    {if (gentokens) printf("tINC\n"); RETURN(tINC);}
"--"    {if (gentokens) printf("tDEC\n"); RETURN(tDEC);}

"<-"   {if (gentokens) printf("tARROW\n"); RETURN(tARROW);}

":="    {if (gentokens) printf("tDEFINE\n"); RETURN(tDEFINE);}
"%="    {if (gentokens) printf("tREMASSIGN\n"); RETURN(tREMASSIGN);}
"="    {if (gentokens) printf("tASSIGN\n"); RETURN(tASSIGN);}
"*="    {if (gentokens) printf("tTIMESASSIGN\n"); RETURN(tTIMESASSIGN);}
"/="    {if (gentokens) printf("tDIVASSIGN\n"); RETURN(tDIVASSIGN);}
"+="    {if (gentokens) printf("tPLUSASSIGN\n"); RETURN(tPLUSASSIGN);}
"-="    {if (gentokens) printf("tMINUSASSIGN\n"); RETURN(tMINUSASSIGN);}
"<<="    {if (gentokens) printf("tLSHIFTASSIGN\n"); RETURN(tLSHIFTASSIGN);}
">>="    {if (gentokens) printf("tRSHIFTASSIGN\n"); RETURN(tRSHIFTASSIGN);}
"^="    {if (gentokens) printf("tBWXORASSIGN\n"); RETURN(tBWXORASSIGN);}
"&^="    {if (gentokens) printf("tBWANDNOTASSIGN\n"); RETURN(tBWANDNOTASSIGN);}
"&^"    {if (gentokens) printf("tBWANDNOT\n"); RETURN(tBWANDNOT);}

"&="    {if (gentokens) printf("tBWANDASSIGN\n"); RETURN(tBWANDASSIGN);}
"|="    {if (gentokens) printf("tBWORASSIGN\n"); RETURN(tBWORASSIGN);}

"-"    {if (gentokens) printf("tMINUS\n"); RETURN(tMINUS);}
"!"    {if (gentokens) printf("tBANG\n"); RETURN(tBANG);}
"+"    {if (gentokens) printf("tPLUS\n"); RETURN(tPLUS);}
"*"    {if (gentokens) printf("tTIMES\n"); RETURN(tTIMES);}
"/"    {if (gentokens) printf("tDIV\n"); RETURN(tDIV);}
"%"    {if (gentokens) printf("tREM\n"); RETURN(tREM);}
"<<"    {if (gentokens) printf("tLSHIFT\n"); RETURN(tLSHIFT);}
">>"    {if (gentokens) printf("tRSHIFT\n"); RETURN(tRSHIFT);}
"&"    {if (gentokens) printf("tBWAND\n"); RETURN(tBWAND);}
"|"    {if (gentokens) printf("tBWOR\n"); RETURN(tBWOR);}
"^"    {if (gentokens) printf("tBWXOR\n"); RETURN(tBWXOR);}
"=="    {if (gentokens) printf("tEQUALS\n"); RETURN(tEQUALS);}
"!="    {if (gentokens) printf("tNE\n"); RETURN(tNE);}
"<"    {if (gentokens) printf("tLT\n"); RETURN(tLT);}
"<="    {if (gentokens) printf("tLE\n"); RETURN(tLE);}
">"    {if (gentokens) printf("tGT\n"); RETURN(tGT);}
">="    {if (gentokens) printf("tGE\n"); RETURN(tGE);}
"&&"    {if (gentokens) printf("tAND\n"); RETURN(tAND);}
"||"    {if (gentokens) printf("tOR\n"); RETURN(tOR);}


var       {if (gentokens) printf("tVAR\n"); RETURN(tVAR);}
break     {if (gentokens) printf("tBREAK\n"); RETURN(tBREAK);}
default   {if (gentokens) printf("tDEFAULT\n"); RETURN(tDEFAULT);}
func      {if (gentokens) printf("tFUNC\n"); RETURN(tFUNC);}
interface {if (gentokens) printf("tINTERFACE\n"); RETURN(tINTERFACE);}
select    {if (gentokens) printf("tSELECT\n"); RETURN(tSELECT);}
case      {if (gentokens) printf("tCASE\n"); RETURN(tCASE);}
defer     {if (gentokens) printf("tDEFER\n"); RETURN(tDEFER);}
go        {if (gentokens) printf("tGO\n"); RETURN(tGO);}
map       {if (gentokens) printf("tMAP\n"); RETURN(tMAP);}
struct    {if (gentokens) printf("tSTRUCT\n"); RETURN(tSTRUCT);}
chan      {if (gentokens) printf("tCHAN\n"); RETURN(tCHAN);}
else      {if (gentokens) printf("tELSE\n"); RETURN(tELSE);}
goto      {if (gentokens) printf("tGOTO\n"); RETURN(tGOTO);}
package   {if (gentokens) printf("tPACKAGE\n"); RETURN(tPACKAGE);}
switch    {if (gentokens) printf("tSWITCH\n"); RETURN(tSWITCH);}
const     {if (gentokens) printf("tCONST\n"); RETURN(tCONST);}
fallthrough {if (gentokens) printf("tDEFER\n"); RETURN(tDEFER);}
if        {if (gentokens) printf("tIF\n"); RETURN(tIF);}
range     {if (gentokens) printf("tRANGE\n"); RETURN(tRANGE);}
type      {if (gentokens) printf("tTYPE\n"); RETURN(tTYPE);}
continue  {if (gentokens) printf("tCONTINUE\n"); RETURN(tCONTINUE);}
for       {if (gentokens) printf("tFOR\n"); RETURN(tFOR);}
import    {if (gentokens) printf("tIMPORT\n"); RETURN(tIMPORT);}
return   {if (gentokens) printf("tRETURN\n"); RETURN(tRETURN);}
print     {if (gentokens) printf("tPRINT\n"); RETURN(tPRINT);}
println   {if (gentokens) printf("tPRINTLN\n"); RETURN(tPRINTLN);}
append    {if (gentokens) printf("tAPPEND\n"); RETURN(tAPPEND);}
len       {if (gentokens) printf("tLEN\n"); RETURN(tLEN);}
cap       {if (gentokens) printf("tCAP\n"); RETURN(tCAP);}


([a-zA-Z]|"_")([a-zA-Z]|[0-9]|"_")* {
	yylval.identifier = strdup(yytext);
  if (gentokens) printf("tIDENTIFIER(%s)\n", yytext);
	RETURN(tIDENTIFIER);
}

. 	{ fprintf (stderr, "Error: (line %d) unexpected character '%s'\n", yylineno, yytext); exit(1); }
%%
