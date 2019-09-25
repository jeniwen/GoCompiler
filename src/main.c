#include <stdio.h>
#include "string.h"
#include "pretty.h"
#include "tree.h"
#include "weed.h"
#include "symbol.h"
#include "typecheck.h"
#include "codegen.h"


void yyparse();
int yylex();
extern FILE *yyin;
void yyrestart( FILE *new_file );
FILE *f;
int gentokens = 0;
PROG * root;
int depth = 0;
bool printSymbolTable = false;
int scopeDepth = 1;
int codegen;
FILE * fileOut;
char *cfile;
STRUCTLIST * structlist;

int main(int argc, char **argv)
{
	if (argc > 2) {
		argv++; char * mode = *argv; argv++;
		f = fopen(argv[0], "r");
		yyin = f;
		if (strcmp(mode,"scan")==0) {
			while (yylex()) {}
			printf("OK\n");
		}
		else if (strcmp(mode,"tokens")==0) {
			gentokens = 1;
			while (yylex()) {}
		}
		else if (strcmp(mode,"parse")==0) {
			yyparse();
			if(!weed_source(root))
			{
				return 1;
			}
			printf("OK\n");
		}
		else if (strcmp(mode,"pretty")==0) {
			gentokens = 0;
			yyparse();
			if(!weed_source(root))
			{
				return 1;
			}
			prettyPROG(root);
			printf("\n");
		}
		else if (strcmp(mode,"symbol")==0) {
			printSymbolTable = true;
			gentokens = 0;
			SymbolTable *st = initSymbolTable();
			yyparse();
			symbolPROG(st, root);
		}
		else if (strcmp(mode,"typecheck")==0) {
			yyparse();
			if(!weed_source(root))
			{
				return 1;
			}
			fflush(stdout);
			STRUCTLIST ** structlistptr;
			structlistptr = (STRUCTLIST **)malloc(sizeof(STRUCTLIST*));
			structlist = malloc(sizeof(STRUCTLIST));
			structlistptr = &structlist;
			typecheck_PROG(root, structlistptr);
			printf("OK\n");
		}
		else if (strcmp(mode,"codegen")==0) {
			codegen = 1;
			printSymbolTable = false;
			char *lastdot;
			cfile = malloc (strlen (argv[0]) + 1);
			strcpy (cfile, argv[0]);
			lastdot = strrchr (cfile, '.');
			
			if (++lastdot != NULL)
				*lastdot = 'c';
			*(++lastdot) = '\0';
			yyparse();
			if(!weed_source(root))
			{
				return 1;
			}
			// printf("%s\n", cfile);
			// STRUCTLIST ** structlistptr;
			// structlistptr = (STRUCTLIST **)malloc(sizeof(STRUCTLIST*));
			structlist = malloc(sizeof(STRUCTLIST));
			// structlistptr = &structlist;
			typecheck_PROG(root, &structlist);
			
			// STRUCTLIST * slist = malloc(sizeof(STRUCTLIST));
			// slist = *structlistptr;
			
			STRUCTLIST * sll = structlist->next;
			// while (sll != NULL) {
			//      if (sll->curr && sll->struct_id)
			//              printf("%d\n", sll->struct_id);
			//      sll = sll->next;
			// }
			
			cgPROG(root, &cfile, sll);
			printf("OK\n");
		}
	}
}
