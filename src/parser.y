



%start program

%%

program : package tSEMICOLON topdecls
;

package : tPACKAGE tIDENT
;

topdecls : topdecl tSEMICOLON topdecls
         |
;

topdecl : funcdecl
	    | decl
;

decl : vardecl
	 | typedecl
;

type : tIDENT
	 | tOPENSQUARE exp tCLOSESQUARE type
	 | tSTRUCT tBLOCKBEGIN fielddeclist tBLOCKEND
	 | tLEFTPAREN type tRIGHTPAREN
	 | tOPENSQUARE tCLOSESQUARE type
;

fielddeclist : identlist type tSEMICOLON fielddeclist
			 | 

vardecl : tVAR varspec
        | tVAR tLEFTPAREN varspeclist tRIGHTPAREN
;

varspeclist : varspec tSEMICOLON varspeclist
		 |
;

varspec : identlist type
        | identlist tASSIGN explist
		| identlist type tASSIGN explist
;

explist : exp tCOMMA explist
		| exp
;

identlist : tIDENT tCOMMA explist
		  | tIDENT
;

typedecl : tTYPE typespec
		 | tTYPE tLEFTPAREN typespeclist tRIGHTPAREN
;

typespeclist : typespec tSEMICOLON typespeclist
          | 
;

typespec : tIDENT tASSIGN type
         | tIDENT type
;

funcdecl : tFUNC tIDENT tLEFTPAREN params tRIGHTPAREN block
         | tFUNC tIDENT tLEFTPAREN params tRIGHTPAREN type block
;

params : identlist tIDENT tCOMMA params
	   | identlist tIDENT
	   | 
;

block : tBLOCKBEGIN statementlist tBLOCKEND
;

statementlist : statement tSEMICOLON statementlist
			  | 
;

statement : tBLOCKBEGIN statementlist tBLOCKEND
		  | exp
		  | explist tASSIGN explist
		  | tIDENT addop tASSIGN exp
		  | tIDENT mulop tASSIGN exp
		  | tIDENT tASSIGN exp
		  | tIDENT tASSIGN exp
		  | decl
		  | identlist tSHORTDECL explist
		  | tIDENT tINC
		  | tIDENT tDECR
		  | tPRINT tLEFTPAREN explist tRIGHTPAREN
		  | tPRINT tLEFTPAREN tRIGHTPAREN
		  | tPRINTLN tLEFTPAREN explist tRIGHTPAREN
		  | tPRINTLN tLEFTPAREN tRIGHTPAREN
		  | tRETURN
		  | tRETURN exp
		  | ifstatement
		  | switchstatement
		  | forstatement
;
		  
ifstatement : tIF exp tBLOCKBEGIN statementlist tBLOCKEND
			| tIF exp tBLOCKBEGIN statementlist tBLOCKEND tELSE 