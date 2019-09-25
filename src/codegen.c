#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>


#include "tree.h"
#include "codegen.h"
#include "pretty.h"

FILE * fout;
char * cfile;
int initNum = 0;
int has_main = 0;
int temp_var = 0;
int break_lbl = -1;
int struct_id = 0;
int continue_lbl = -1;
int rec_struct_count = 0;
int blankid_dec = 0;

void findent(int indentation) {
	for (int tabs = 0; tabs < indentation; tabs++) {
		fprintf(fout, "\t");
	}
}

void cgList(List * l) {
	//TODO: generate code for lists Nevermind
	if (l != NULL) {
		if (l->kind == listKindExp) {
			cgEXP(l->current.expression);
		}
		else if (l->kind == listKindParam) {
			cgList(l->current.param.idlist);
		}
		else if (l->kind == listKindId) {
			fprintf(fout, "%s", l->current.id);
		}
		cgList(l->nextli);
	}
}


void cg_prefixID(char * orig) {
	fprintf(fout, "__golite__%s", orig);
}

void cgTYPE(TYPE * t, int indentation){
	if (t != NULL) {
		switch (t->kind) {
		case unknownType: cgTYPE(t->rt, indentation); // I may need to change resolveType to be TYPE* rather than TypeKind, we'll see
			break;
		case intType: fprintf(fout, "int"); break;
		case floatType: fprintf(fout, "float"); break;
		case runeType: fprintf(fout, "int"); break; // This is supposed to be int
		case stringType: fprintf(fout, "char* "); break;
		case boolType: fprintf(fout, "bool"); break;
		case structType:
			fprintf(fout, "__golite_struct_%d ", t->struct_id);
			break;
		case arrayType: fprintf(fout, "ARRAY");
			break;
		case sliceType: fprintf(fout, "SLICE");
			break;
		default: break;
		}
	}
}

void cg_initvar(TYPE * t, char* name) { // I'm not sure we need "name," so I guess I added it erroneously, but I won't get rid of it since I don't know for sure
	if (t) {
		switch (t->kind) {
		case intType: fprintf(fout, "0"); break;
		case floatType: fprintf(fout, "0.0"); break;
		case runeType: fprintf(fout, "0"); break;
		case stringType: fprintf(fout, "\"\""); break;
		case boolType: fprintf(fout, "false"); break;
		case arrayType:
		{
			char* underlyingType = "intType"; // As my default
			int underlying_cap = 0; // Default types have no capacity
			if(t->val.arrayData.entryType->kind == arrayType)
			{
				underlyingType = "arrayType";
			}
			else if(t->val.arrayData.entryType->kind == sliceType)
			{
				underlyingType = "sliceType";
			}

			fprintf(fout, "make_array(make_type(%s, sizeof(", underlyingType);
			cgTYPE(t->val.arrayData.entryType,0);
			fprintf(fout, "), ");
			if(t->val.arrayData.entryType->kind == arrayType || t->val.arrayData.entryType->kind == sliceType)
			{
				fprintf(fout, "(");
				cg_initvar(t->val.arrayData.entryType, name);
				fprintf(fout, ".underlying)");
				underlying_cap = t->val.arrayData.entryType->val.arrayData.capacity;
			}
			else
			{
				fprintf(fout, "NULL");
			}
			fprintf(fout, ", %d), %d)", underlying_cap, t->val.arrayData.capacity);
			break;
		}
		case sliceType:
		{
			char* underlyingType = "intType"; // As my default, will account for everything with no special cases
			if(t->val.sliceEntryType->kind == arrayType)
			{
				underlyingType = "arrayType";
			}
			else if(t->val.sliceEntryType->kind == sliceType)
			{
				underlyingType = "sliceType";
			}

			fprintf(fout, "make_slice(make_type(%s, sizeof(", underlyingType);
			cgTYPE(t->val.arrayData.entryType,0);
			fprintf(fout, "), ");
			if(t->val.sliceEntryType->kind == arrayType || t->val.sliceEntryType->kind == sliceType)
			{
				fprintf(fout, "(");
				cg_initvar(t->val.sliceEntryType, name);
				fprintf(fout, ".underlying)");
			}
			else
			{
				fprintf(fout, "NULL");
			}
			fprintf(fout, ", 0))");
		}
		break;
		case structType:
			fprintf(fout, "struct_copy%d(NULL)", t->struct_id);
			// fprintf(fout, "malloc(sizeof(");
			// cgTYPE(t, 0);
			// fprintf(fout, "));\n");
			// struct_init(name, t->val.fields);
			break;
		case unknownType: cg_initvar(t->rt, name);
			break;
		case nullType: fprintf(fout, "NULL"); break;
		}
	}
}

void cgParamList(List * l, int blank_count, bool newLine, int indentation) {
	if (l) {
		List * currID = l->current.param.idlist;
		while (currID) {
			if (newLine) findent(indentation);
			cgTYPE(l->current.param.type, 0);
			fprintf(fout, " ");
			if(l->current.param.type->rt->kind == structType) fprintf(fout, "*");
			if (strcmp(currID->current.id, "_") != 0)
				cg_prefixID(currID->current.id);
			else {
				fprintf(fout, "__golite__blank_%d", blank_count);
				blank_count++;
			}
			currID = currID->nextli;
			if (currID) {
				if (!newLine)
					fprintf(fout, ", ");
				else
					fprintf(fout, ";\n");
			}
		}
		if (l->nextli) {
			if (!newLine) fprintf(fout, ", ");
			else fprintf(fout, ";\n");
		}
		else if (newLine) fprintf(fout, ";");
		cgParamList(l->nextli, blank_count, newLine, indentation);


	}
}


void cgSTMT(STMT * s, int indentation, bool line_end){
	List* lhs;
	List* rhs;
	STMT * currS;
	if (s != NULL) {
		switch (s->kind) {
		case statementKindEmpty:
			if (line_end)
				fprintf(fout, "\n");
			break;
		case statementKindBlock:
			fprintf(fout, " {\n");
			cgSTMT(s->val.stmtlist, indentation + 1, true);
			findent(indentation);
			fprintf(fout, "}");
			if(line_end) fprintf(fout, "\n");
			break;
		case statementKindExpression:
			if (line_end) findent(indentation);
			cgEXP(s->val.expression);
			// if (line_end)
			fprintf(fout, ";\n");
			break;
		case statementKindAssign:
			lhs = s->val.assignStmt.lhs;
			rhs = s->val.assignStmt.rhs;
			fprintf(fout, "\n");
			findent(indentation);
			fprintf(fout, "{\n");
			int arrays = 0;
			int vars = 0;
			while(lhs != NULL && rhs != NULL)
			{
				if(lhs->current.expression->type->resolveType != arrayType && lhs->current.expression->type->resolveType != sliceType && lhs->current.expression->type->rt->kind != structType)
				{
					findent(indentation+1);
					cgTYPE(rhs->current.expression->type,indentation);
					fprintf(fout, " tmp_%d", vars++);
					fprintf(fout, "_var = ");
					cgEXP(rhs->current.expression);
					fprintf(fout, ";\n");

				}
				else if(lhs->current.expression->type->kind == arrayType)
				{
					fprintf(fout, "array_assign( ");
					fprintf(fout, " tmp_%d", arrays++);
					fprintf(fout, "_var");
					fprintf(fout, ", ");
					cgEXP(rhs->current.expression);
					fprintf(fout, ");");
				}
				else if(lhs->current.expression->type->kind == sliceType)
				{
					fprintf(fout, "slice_assign( ");
					fprintf(fout, " tmp_%d", arrays++);
					fprintf(fout, "_var");
					fprintf(fout, ", ");
					cgEXP(rhs->current.expression);
					fprintf(fout, ");");
				}
				else if (lhs->current.expression->type->rt->kind == structType) {
					findent(indentation+1);
					cgTYPE(lhs->current.expression->type->rt,indentation);
					fprintf(fout, "* tmp_%d", vars++);
					fprintf(fout, "_var = struct_copy%d(", rhs->current.expression->type->rt->struct_id);
					cgEXP(rhs->current.expression);
					fprintf(fout, ");\n");
				}
				lhs = lhs->nextli;
				rhs = rhs->nextli;
			}
			lhs = s->val.assignStmt.lhs;
			arrays = 0;
			vars = 0;
			while(lhs != NULL)
			{
				if(lhs->current.expression->type->resolveType != arrayType && lhs->current.expression->type->resolveType != sliceType && (lhs->current.expression->val.identifier == NULL ||strcmp(lhs->current.expression->val.identifier, "_") != 0))
				{
					findent(indentation+1);
					cgEXP(lhs->current.expression);
					fprintf(fout, " = ");
					fprintf(fout, "tmp_%d", vars++);
					fprintf(fout, "_var;");

					fprintf(fout, "\n");

				}
				else if(lhs->current.expression->type->kind == arrayType && (lhs->current.expression->val.identifier == NULL ||strcmp(lhs->current.expression->val.identifier, "_") != 0))
				{
					fprintf(fout, "array_assign( ");
					cgEXP(lhs->current.expression);
					fprintf(fout, ", ");
					fprintf(fout, " tmp_%d", arrays++);
					fprintf(fout, "_var");
					fprintf(fout, ");");
				}
				else if(lhs->current.expression->type->kind == sliceType && (lhs->current.expression->val.identifier == NULL ||strcmp(lhs->current.expression->val.identifier, "_") != 0))
				{
					fprintf(fout, "slice_assign( ");
					cgEXP(lhs->current.expression);
					fprintf(fout, ", ");
					fprintf(fout, " tmp_%d", arrays++);
					fprintf(fout, "_var");
					fprintf(fout, ");");
				}
				if(lhs->current.expression->val.identifier && strcmp(lhs->current.expression->val.identifier, "_") == 0){
					if(lhs->current.expression->type->rt->kind == sliceType || lhs->current.expression->type->rt->kind == arrayType)
						arrays++;
					else
						vars++;
				}
				lhs = lhs->nextli;
			}
			findent(indentation);
			fprintf(fout, "}\n");
			break;
		case statementKindAssignOp:
			if (line_end) findent(indentation);
			if(s->val.assignopStmt.expression->type->rt->kind != stringType)
			{
				cgEXP(s->val.assignopStmt.identifier);
			}
			switch (s->val.assignopStmt.aop) {
			case assignopKindRemainder:
				fprintf(fout, " %%= ");
				cgEXP(s->val.assignopStmt.expression);
				fprintf(fout, ";");
				break;
			case assignopKindMultiplication:
				fprintf(fout, " *= ");
				cgEXP(s->val.assignopStmt.expression);
				fprintf(fout, ";");
				break;
			case assignopKindDivision:
				fprintf(fout, " /= ");
				cgEXP(s->val.assignopStmt.expression);
				fprintf(fout, ";");
				break;
			case assignopKindPlus:
				if(s->val.assignopStmt.expression->type->rt->kind == stringType)
				{
					fprintf(fout, "{\n");
					findent(indentation + 1);
					fprintf(fout, "char* tmp_string1 = ");
					cgEXP(s->val.assignopStmt.identifier);
					fprintf(fout, ";\n");
					findent(indentation + 1);
					fprintf(fout, "char* tmp_string2 = ");
					cgEXP(s->val.assignopStmt.expression);
					fprintf(fout, ";\n");
					findent(indentation + 1);
					cgEXP(s->val.assignopStmt.identifier);
					fprintf(fout," = stradd(tmp_string1, tmp_string2);"); // *Should be done* but don't think it works for i[f(x)] += a, but idk what to do
				}
				else
				{
					fprintf(fout, " += ");
					cgEXP(s->val.assignopStmt.expression);
					fprintf(fout, ";");
				}
				break;
			case assignopKindMinus:
				fprintf(fout, " -= ");
				cgEXP(s->val.assignopStmt.expression);
				fprintf(fout, ";");
				break;
			case assignopKindLShift:
				fprintf(fout, " <<= ");
				cgEXP(s->val.assignopStmt.expression);
				fprintf(fout, ";");
				break;
			case assignopKindRShift:
				fprintf(fout, " >>= ");
				cgEXP(s->val.assignopStmt.expression);
				fprintf(fout, ";");
				break;
			case assignopKindBWXor:
				fprintf(fout, " ^= ");
				cgEXP(s->val.assignopStmt.expression);
				fprintf(fout, ";");
				break;
			case assignopKindBWAndNot:
				fprintf(fout, " &= ~");
				cgEXP(s->val.assignopStmt.expression);
				fprintf(fout, ";");
				break;
			case assignopKindBWAnd:
				fprintf(fout, " &= ");
				cgEXP(s->val.assignopStmt.expression);
				fprintf(fout, ";");
				break;
			case assignopKindBWOr:
				fprintf(fout, " |= ");
				cgEXP(s->val.assignopStmt.expression);
				fprintf(fout, ";");
				break;
			}
			if(line_end) fprintf(fout, "\n");
			break;
		case statementKindShortAssign:
		{
			for(List* lhs = s->val.assignStmt.lhs; lhs != NULL; lhs = lhs->nextli)
			{
				if(lhs->isDeclared == true) // isDeclared should be read more like getsDeclared
				{
					findent(indentation+1);
					cgTYPE(lhs->current.expression->type,indentation);
					fprintf(fout, " ");
					cgEXP(lhs->current.expression);
					fprintf(fout, ";\n");
				}
			}
			lhs = s->val.assignStmt.lhs;
			rhs = s->val.assignStmt.rhs;
			fprintf(fout, "\n");
			findent(indentation);
			fprintf(fout, "{\n");
			int arrays = 0;
			int vars = 0;
			while(lhs != NULL && rhs != NULL)
			{
				if(lhs->current.expression->type->rt->kind != arrayType && lhs->current.expression->type->rt->kind != sliceType)
				{
					findent(indentation+1);
					cgTYPE(rhs->current.expression->type,indentation);
					fprintf(fout, " tmp_%d", vars);
					fprintf(fout, "_var = ");
					cgEXP(rhs->current.expression);
					fprintf(fout, ";\n");
					vars++;
				}
				else if(lhs->current.expression->type->rt->kind == arrayType)
				{
					fprintf(fout, "ARRAY ");
					fprintf(fout, "tmp_");
					fprintf(fout, "array_%d", arrays);
					fprintf(fout, "array_assign( ");
					fprintf(fout, "tmp_");
					fprintf(fout, "array_%d", arrays);
					fprintf(fout, ", ");
					arrays++;
					cgEXP(rhs->current.expression);
					fprintf(fout, ");");
				}
				else if(lhs->current.expression->type->resolveType == sliceType)
				{
					fprintf(fout, "SLICE ");
					fprintf(fout, "tmp_");
					fprintf(fout, "slice_%d", arrays);
					fprintf(fout, "slice_assign( ");
					fprintf(fout, "tmp_");
					fprintf(fout, "array_%d", arrays);
					fprintf(fout, ", ");
					arrays++;
					cgEXP(rhs->current.expression);
					fprintf(fout, ");");
				}

				lhs = lhs->nextli;
				rhs = rhs->nextli;
			}
			lhs = s->val.assignStmt.lhs;
			rhs = s->val.assignStmt.rhs;
			arrays = 0;
			vars = 0;
			while(lhs != NULL && rhs != NULL)
			{
				if((lhs->current.expression->val.identifier == NULL ||strcmp(lhs->current.expression->val.identifier, "_") != 0)){
					if(lhs->current.expression->type->resolveType != arrayType && lhs->current.expression->type->resolveType != sliceType)
					{
						findent(indentation+1);
						cgEXP(lhs->current.expression);
						fprintf(fout, " = ");
						fprintf(fout, "tmp_%d", vars);
						vars++;
						fprintf(fout, "_var;");
					}
					else if(lhs->current.expression->type->kind == arrayType)
					{
						fprintf(fout, "array_assign( ");
						cgEXP(lhs->current.expression);
						fprintf(fout, ", ");
						fprintf(fout, "tmp_");
						fprintf(fout, "array_%d", arrays);
						arrays++;
						fprintf(fout, ");");
					}
					else if(lhs->current.expression->type->kind == sliceType)
					{
						fprintf(fout, "slice_assign( ");
						cgEXP(lhs->current.expression);
						fprintf(fout, ", ");
						fprintf(fout, "tmp_");
						fprintf(fout, "slice_%d", arrays);
						arrays++;
						fprintf(fout, ");");
					}
				}
				if(lhs->current.expression->val.identifier && strcmp(lhs->current.expression->val.identifier, "_") == 0){
					if(lhs->current.expression->type->rt->kind == sliceType || lhs->current.expression->type->rt->kind == arrayType)
						arrays++;
					else
						vars++;
				}
				lhs = lhs->nextli;
				rhs = rhs->nextli;
			}

			fprintf(fout, "\n");
			findent(indentation);
			fprintf(fout, "}\n");
		}
		break;
		case statementKindVarDeclaration:
			currS = s;
			rhs = s->val.declStmt.values;
			while (currS && currS->kind == statementKindVarDeclaration) {

				lhs = currS->val.declStmt.identifiers;

				while (lhs && lhs->kind == listKindId) {
					if (line_end) findent(indentation);
					if (!rhs) {
						cgTYPE(currS->val.declStmt.type, indentation);
					} else {
						cgTYPE(rhs->current.expression->type, indentation);
					}

					if((currS->val.declStmt.type && currS->val.declStmt.type->rt->kind == structType) || (rhs && rhs->current.expression->type->rt->kind == structType))
					{
						fprintf(fout, "*");
					}

					fprintf(fout, " ");
					fprintf(fout, "tmp");
					if ((strcmp(lhs->current.id, "_") == 0))
						fprintf(fout, "%d", ++blankid_dec);
					cg_prefixID(lhs->current.id);
					fprintf(fout, " = ");
					if (!rhs) {
						cg_initvar(currS->val.declStmt.type, lhs->current.id);
					} else if (rhs->current.expression->type->rt->kind == arrayType) {
						fprintf(fout, "array_copy(");
						cgEXP(rhs->current.expression);
						fprintf(fout, ")");
					} else if (rhs->current.expression->type->rt->kind == sliceType) {
						fprintf(fout, "slice_copy(");
						cgEXP(rhs->current.expression);
						fprintf(fout, ")");
					} else if (rhs->current.expression->type->rt->kind == structType) {
						fprintf(fout, "struct_copy%d(", rhs->current.expression->type->rt->struct_id);
						cgEXP(rhs->current.expression);
						fprintf(fout, ")");
					} else {
						cgEXP(rhs->current.expression);
					}
					fprintf(fout, ";\n");

					if (strcmp(lhs->current.id, "_") != 0)
					{
						if (!rhs) {
							cgTYPE(currS->val.declStmt.type, indentation);
						} else {
							cgTYPE(rhs->current.expression->type, indentation);
						}
						if((currS->val.declStmt.type && currS->val.declStmt.type->rt->kind == structType) || (rhs && rhs->current.expression->type->rt->kind == structType))
						{
							fprintf(fout, "*");
						}
						fprintf(fout, " ");
						cg_prefixID(lhs->current.id);
						fprintf(fout, " = ");
						// if((currS->val.declStmt.type && currS->val.declStmt.type->rt->kind == structType) || (rhs && rhs->current.expression->type->rt->kind == structType))
						// {
						// 	fprintf(fout, "struct_copy%d(tmp", currS->val.declStmt.type->rt->struct_id);
						// 	cg_prefixID(lhs->current.id);
						// 	fprintf(fout, ");\n");
						// } else {
							fprintf(fout, "tmp");
							cg_prefixID(lhs->current.id);
						// }
						fprintf(fout, ";\n");
					}
					lhs = lhs->nextli;
					if (rhs) rhs = rhs->nextli;
				}
				fprintf(fout, "\n");
				currS = currS->val.declStmt.nextd;
			}

			break;
		case statementKindTypeDeclaration:
		{ // Keep hither the scope
			STMT* tmp = s;
			while(tmp != NULL)
			{
				if(tmp->val.typeDecl.type->kind == structType)
				{
					// findent(indentation);
					// fprintf(fout, "typedef ");
					// cgTYPE(tmp->val.typeDecl.type, indentation+1);
					// fprintf(fout, " __golite_struct_%s;\n\n", tmp->val.typeDecl.identifier);
				}

				tmp = tmp->val.typeDecl.nextT;
			}
		}
		break;
		case statementKindDecrement:
		case statementKindIncrement:
		if(s->val.incdec.inc)
		{
			fprintf(fout,"("); cgEXP(s->val.incdec.exp); fprintf(fout,")++"); fprintf(fout,";\n");
		}
		else
		{
			fprintf(fout,"("); cgEXP(s->val.incdec.exp); fprintf(fout,")--"); fprintf(fout,";\n");
		}
		break;
		case statementKindPrint:
		case statementKindPrintln:
		{
			List* exps = s->val.printList;

			while(exps != NULL)
			{
				findent(indentation);
				switch (exps->current.expression->type->rt->kind) {
				case unknownType: fprintf(stderr, "Error: 3 I tried to make this case impossible, so shitty if you're here");
					exit(1);
					break;
				case floatType:
					fprintf(fout, "{\n");
					findent(indentation + 1);
					fprintf(fout, "float tmp_float_x = ");
					cgEXP(exps->current.expression);
					fprintf(fout, ";\n");
					findent(indentation+1);
					fprintf(fout, "printf((tmp_float_x==0)?\"+0.000000e+000\":\"%%+e\", tmp_float_x");
					// cgEXP(exps->current.expression);
					fprintf(fout, ");\n");
					findent(indentation);
					fprintf(fout, "}\n");
					break;
				case intType: fprintf(fout, "printf(\"%%i\", ");
					cgEXP(exps->current.expression);
					fprintf(fout, ");\n");
					break;
				case runeType:
					fprintf(fout, "printf(\"%%d\", ");
					cgEXP(exps->current.expression);
					fprintf(fout, ");\n");
					break;
				case stringType:
					fprintf(fout, "printf(\"%%s\", ");
					cgEXP(exps->current.expression);
					fprintf(fout, " );\n");
					break;
				case boolType:
					fprintf(fout, "printf(\"%%s\", (");
					cgEXP(exps->current.expression);
					fprintf(fout, ")?\"true\":\"false\");\n");
					break;
				case structType:
				case arrayType:
				case sliceType:
					fprintf(stderr, "Error: This should be impossible (printing struct,array or slice)");
					exit(1);
					break;
				default: break;
				}
				if(s->kind == statementKindPrintln && exps->nextli != NULL)
					fprintf(fout, "printf(\" \");\n");

				exps = exps->nextli;


			}
			if(s->kind == statementKindPrintln)
				fprintf(fout, "printf(\"\\n\");\n");
			break;
		}
		case statementKindReturn:
			if (line_end) findent(indentation);
			fprintf(fout, "return ");
			cgEXP(s->val.expression);
			fprintf(fout, ";\n");
			break;
		case statementKindIf:
			if (line_end) findent(indentation);
			if (s->val.ifStmt.initStmt != NULL) {fprintf(fout, "{\n"); cgSTMT(s->val.ifStmt.initStmt, 0, false); }
			fprintf(fout, "if (");
			cgEXP(s->val.ifStmt.condition);
			fprintf(fout, ")");
			cgSTMT(s->val.ifStmt.ifBody, indentation, false);
			if (s->val.ifStmt.elseBody != NULL) {
				fprintf(fout, " else ");
				cgSTMT(s->val.ifStmt.elseBody, indentation, false);
			}
			if (s->val.ifStmt.elseif != NULL) {
				findent(indentation);
				fprintf(fout, " else ");
				cgSTMT(s->val.ifStmt.elseif, indentation, false);
			}
			if (s->val.ifStmt.initStmt != NULL) {fprintf(fout, "}\n");}
			fprintf(fout, "\n");
			break;
		case statementKindSwitch:
			fprintf(fout, "{\n");
			findent(indentation + 1);
			if(s->val.switchStmt.initStmt) {
				cgSTMT(s->val.switchStmt.initStmt, indentation, false);
				findent(indentation + 1);
			}
			if(s->val.switchStmt.switchExp) {
				cgTYPE(s->val.switchStmt.switchExp->type, indentation);
				fprintf(fout, " tmp_switch_exp%d = ", s->lineno);
				cgEXP(s->val.switchStmt.switchExp);
				fprintf(fout, ";\n");
			}
			else{
				fprintf(fout, "bool tmp_switch_exp%d = true;\n", s->lineno);
			}
			findent(indentation + 1);
			if(s->val.switchStmt.caseStmts) {
				SWITCHCASE * currCase = s->val.switchStmt.caseStmts;
				int tmp_break = break_lbl;
				break_lbl = s->lineno;
				int num = 0;
				while(currCase) {
					List * currExp = currCase->exprlist;
					if(!currCase->isDefault && currExp) {
						fprintf(fout, "if (tmp_switch_exp%d == ", s->lineno);
						cgEXP(currExp->current.expression);
						currExp = currExp->nextli;
						while (currExp) {
							fprintf(fout, "|| tmp_switch_exp%d == ", s->lineno);
							cgEXP(currExp->current.expression);
							currExp = currExp->nextli;
						}
						fprintf(fout, ")");
					}
					if(num == 0 && currCase->isDefault)
						fprintf(fout, "if (true)");
					fprintf(fout, "{\n");
					findent(indentation+2);
					cgSTMT(currCase->statement, indentation+2, true);
					findent(indentation+1);
					fprintf(fout, "}\n");
					if (currCase->next) {
						findent(indentation+1);
						fprintf(fout, " else ");
					}
					num++;
					currCase = currCase->next;
				}
				findent(indentation+1);
				fprintf(fout, "break__lbl%d:;\n", break_lbl);
				break_lbl = tmp_break;
			}
			findent(indentation);
			fprintf(fout, "}\n");
			break;
		case statementKindFor:
			fprintf(fout, "{\n");
			{
				int tmp_continue = continue_lbl;
				continue_lbl = s->lineno;
				int tmp_break = break_lbl;
				break_lbl = s->lineno;
				cgSTMT(s->val.forStmt.lhs, indentation + 1, true);
				fprintf(fout, "while( ");
				if (s->val.forStmt.cond)
					cgEXP(s->val.forStmt.cond);
				else (fprintf(fout, "true"));
				fprintf(fout, " ) {\n");
				cgSTMT(s->val.forStmt.body, indentation + 2, true);
				fprintf(fout, "continue__lbl%d:;\n", continue_lbl);
				cgSTMT(s->val.forStmt.rhs, indentation + 1, true);
				findent(indentation + 1);
				fprintf(fout, "}\n");
				fprintf(fout, "break__lbl%d:;\n", break_lbl);
				break_lbl = tmp_break;
				continue_lbl = tmp_continue;
			}
			fprintf(fout, "\n}\n");
			break;
		case statementKindBreak:
			if(break_lbl !=-1) {
				fprintf(fout, "goto break__lbl%d;\n", break_lbl);
			}
			break;
		case statementKindContinue:
			if(continue_lbl != -1) {
				fprintf(fout, "goto continue__lbl%d;\n", continue_lbl);
			}
			break;
		}
		cgSTMT(s->next, indentation, line_end);
	}
}
void cgEXP(EXP * e){
	if (e != NULL) {
		char * sym = "";
		switch (e->kind) {
		case expressionKindIdentifier: fprintf(fout, "__golite__%s", e->val.identifier); break;
		case expressionKindIntLiteral:
			fprintf( fout, "%d", e->val.intLiteral);
			break;
		case expressionKindFloatLiteral: fprintf( fout, "%f", e->val.floatLiteral); break;
		case expressionKindRuneLiteral:
			// if (e->val.runeLiteral[0] == '\\') {
			fprintf( fout, "\'");
			if (e->val.runeLiteral[1] == '\\') {
				fprintf( fout, "%c",'\\');
				fprintf( fout, "%c",e->val.runeLiteral[2]);
			} else {
				fprintf( fout, "%c", e->val.runeLiteral[1]);
			}
			fprintf( fout, "\'");
			break;
		case expressionKindBoolLiteral: fprintf(fout, "%s",  e->val.boolLiteral ? "true" : "false"); break;
		case expressionKindStringLiteral: fprintf( fout,"%s", e->val.stringLiteral); break;
		case expressionKindStructFieldAccess:
			cgEXP(e->val.structAccess.lhs);
			fprintf(fout, "->__golite__%s", e->val.structAccess.fieldIdent);
			break;
		case expressionKindUnaryPlus:

			sym = "+"; fprintf( fout, "(%s", sym); cgEXP(e->val.unary); fprintf( fout, ")"); break;
		case expressionKindUnaryMinus: sym = "-";  fprintf( fout, "(%s", sym); cgEXP(e->val.unary); fprintf( fout, ")"); break;
		case expressionKindUnaryNot: sym = "!";  fprintf( fout, "(%s", sym); cgEXP(e->val.unary); fprintf( fout, ")"); break;
		case expressionKindBitComplement: sym = "~"; fprintf( fout, "(%s", sym); cgEXP(e->val.unary); fprintf( fout, ")"); break;
		case expressionKindAddition:
			switch(e->type->rt->kind)
			{
			case intType:
			case floatType:
			case runeType:
				sym = " + ";  fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")");
				break;
			case stringType:
				fprintf(fout, "stradd(");
				cgEXP(e->val.binary.lhs);
				fprintf(fout, ", ");
				cgEXP(e->val.binary.rhs);
				fprintf(fout, ")");
				break;
			case boolType:
			case nullType:
			case structType:
			case arrayType:
			case sliceType:
			case unknownType:
				fprintf(stderr, "Error: This should be impossible (addition)\n");
				exit(1);
				break;
			default:
				break;
			}
			break;
		//sym = " + ";  fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindSubtraction: sym = " - "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindMultiplication: sym = " * "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindDivision: sym = " / "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindOr: sym = " || "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindAnd: sym = " && "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindNotEquals: fprintf(fout, "!");
		case expressionKindEquals:
			switch(e->val.binary.lhs->type->rt->kind)
			{
			case intType:
			case floatType:
			case runeType:
			case boolType:
				sym = " == ";  fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")");
				break;
			case stringType:
				fprintf(fout, "!strcmp("); // negation to make it actually 0 or 1 for false and true respectively
				cgEXP(e->val.binary.lhs);
				fprintf(fout, ", ");
				cgEXP(e->val.binary.rhs);
				fprintf(fout, ")");
				break;
			case structType:
				cg_struct_equality(e->val.binary.lhs, e->val.binary.lhs->type->rt->val.fields,e->val.binary.rhs , e->val.binary.rhs->type->rt->val.fields);
				break;
			case arrayType:
				fprintf(fout, "arrays_equal(");
				cgEXP(e->val.binary.lhs);
				fprintf(fout, ", ");
				cgEXP(e->val.binary.rhs);
				fprintf(fout, ")");
				break;
			case unknownType:
			case nullType:
			case sliceType:
				fprintf(stderr, "Error: This should be impossible, removed in typecheck (equality)\n");
				exit(1);
			default:
				break;
			}
			break;
		case expressionKindLower: if(e->val.binary.lhs->type->rt->kind == stringType){fprintf(fout, "(strcmp(");  cgEXP(e->val.binary.lhs); fprintf( fout, ","); cgEXP(e->val.binary.rhs); fprintf( fout, ") < 0)"); break;} sym = " < "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindLeq: if(e->val.binary.lhs->type->rt->kind == stringType){fprintf(fout, "(strcmp(");  cgEXP(e->val.binary.lhs); fprintf( fout, ","); cgEXP(e->val.binary.rhs); fprintf( fout, ") <= 0)"); break;}sym = " <= "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindGreater: if(e->val.binary.lhs->type->rt->kind == stringType){fprintf(fout, "(strcmp(");  cgEXP(e->val.binary.lhs); fprintf( fout, ","); cgEXP(e->val.binary.rhs); fprintf( fout, ") > 0)"); break;} sym = " > "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindGeq: if(e->val.binary.lhs->type->rt->kind == stringType){fprintf(fout, "(strcmp(");  cgEXP(e->val.binary.lhs); fprintf( fout,","); cgEXP(e->val.binary.rhs); fprintf( fout, ") >= 0)"); break;} sym = " >= "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindBitwiseOr: sym = " | "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindBitwiseXor: sym = " ^ "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindModulo: sym = " % "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindLeftShift: sym = " << "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindRightShift: sym = " >> "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindBitwiseAnd: sym = " & "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")");  break;
		case expressionKindBitClear: // Apparently this is the same things as BitwiseAndNot
		case expressionKindBitwiseAndNot: sym = " & "; fprintf( fout, "("); cgEXP(e->val.binary.lhs); fprintf( fout, "%s~", sym); cgEXP(e->val.binary.rhs); fprintf( fout, ")"); break;
		case expressionKindTypeCast:
		case expressionKindFunctionCall:
			if(e->isCast) {
				// TYPE * caster = e->type;
				TYPE * caster;
				EXP * castee;
				if (e->type && e->type->rt) {
					caster = e->type->rt;
					if (e->val.funCall.args && e->val.funCall.args->current.expression->type) {
						castee = e->val.funCall.args->current.expression;
						if (caster && castee && castee->type && castee->type->rt) {
							TypeKind eType = castee->type->rt->kind;
							if (caster->kind == eType) {
								cgEXP(castee);                //Return the expression if it's casting to itself
							} else if (caster->kind == stringType) {
								if (eType == intType || eType == runeType) {
									fprintf(fout, "string_cast(");
									cgEXP(castee);
									fprintf(fout, ")");
								}
							} else if (caster->kind == intType || caster->kind == runeType) {
								if (eType == floatType) {
									fprintf(fout, "(int) round(");
									cgEXP(castee);
									fprintf(fout, ")");
								} else if (eType == runeType) {
									cgEXP(castee);
								}
							} else if (caster->kind == floatType) {
								if (eType == intType || eType == runeType)
									cgEXP(castee);
							}
						}
					}
				}

			}
			else
			{
				cgEXP(e->val.funCall.funName);
				fprintf(fout, "(");
				List* args = e->val.funCall.args;
				while(args != NULL)
				{
					if(args->current.expression->type->rt->kind != sliceType && args->current.expression->type->rt->kind != arrayType && args->current.expression->type->rt->kind != structType)
					{
						cgEXP(args->current.expression);
					}
					else if (args->current.expression->type->rt->kind == arrayType)
					{
						fprintf(fout, "array_copy(");
						cgEXP(args->current.expression);
						fprintf(fout, ")");
					}
					else if (args->current.expression->type->rt->kind == sliceType)
					{
						fprintf(fout, "slice_copy(");
						cgEXP(args->current.expression);
						fprintf(fout, ")");
					}
					else if (args->current.expression->type->rt->kind == structType)
					{
						fprintf(fout, "struct_copy%d(", args->current.expression->type->rt->struct_id);
						cgEXP(args->current.expression);
						fprintf(fout, ")");
					}

					if(args->nextli != NULL)
						fprintf(fout, ", ");
					args = args->nextli;
				}
				fprintf(fout, ")");
			}
			break;
		case expressionKindAppend: fprintf(fout, "append(" ); cgEXP(e->val.append.appendto); fprintf(fout, ", ");  cgEXP(e->val.append.appendfrom); fprintf(fout, ")"); break;
		case expressionKindCap: fprintf(fout, "cap(" ); cgEXP(e->val.expression); fprintf(fout, " )"); break;
		case expressionKindLen: if(e->val.expression->type->rt->kind == stringType){fprintf(fout, "((int) strlen(");cgEXP(e->val.expression);fprintf(fout, "))"); break;}fprintf(fout, "len(" ); cgEXP(e->val.expression); fprintf(fout, " )"); break;
		case expressionKindArrayAccess:
			fprintf(fout, "*(");
			cgTYPE(e->val.arrayAccess.identifier->type->rt->val.arrayData.entryType, 0);
			if(e->val.arrayAccess.identifier->type->rt->val.arrayData.entryType->rt->kind == structType)
				fprintf(fout, " *");
			fprintf(fout, " * )array_access( ");
			cgEXP(e->val.arrayAccess.identifier);
			fprintf(fout, ", ");
			cgEXP(e->val.arrayAccess.index);
			fprintf(fout, " )");
			break;
		}

	}
}
void cgSWITCHCASE(SWITCHCASE *s, int indentation){
	if (s != NULL) {
		findent(indentation);
		if (s->isDefault) fprintf(fout, "default: \n");
		else {
			fprintf(fout, "case ");
			// cgList(s->exprlist); //TODO: need to have a separate "case" for each expression
			fprintf(fout, ": \n");
		}
		cgSTMT(s->statement, indentation+1, true);
		cgSWITCHCASE(s->next, indentation);

	}
}

void cgGlobalDefs(TOPSTMT * ts)
{
	if(ts != NULL)
	{
		if (ts->isFunDecl) {
			// Do Nothing
		} else {
			if(ts->val.orig != NULL)
			{

				STMT * currS;
				List* rhs;
				List * lhs;
				switch(ts->val.orig->kind)
				{
				case statementKindVarDeclaration:
					currS = ts->val.orig;
					while (currS && currS->kind == statementKindVarDeclaration) {
						rhs = ts->val.orig->val.declStmt.values;
						lhs = currS->val.declStmt.identifiers;
						TYPE * ty = currS->val.declStmt.type;
						while (lhs && lhs->kind == listKindId) {
							findent(1);
							if (!rhs && ty) {
								cgTYPE(currS->val.declStmt.type->rt, 0);
							} else {
								cgTYPE(rhs->current.expression->type->rt, 0);
							}
							if (ty && ty->rt->kind == structType) {fprintf(fout, "*");}

							fprintf(fout, " ");
							fprintf(fout, "tmp");
							cg_prefixID(lhs->current.id);
							fprintf(fout, " = ");
							if (!rhs) {
								cg_initvar(currS->val.declStmt.type,lhs->current.id);
							} else {
								cgEXP(rhs->current.expression);
							}
							fprintf(fout, ";\n");
							fprintf(fout, " ");
							cg_prefixID(lhs->current.id);
							fprintf(fout, " = ");
							fprintf(fout, "tmp");
							cg_prefixID(lhs->current.id);


							fprintf(fout, ";\n");
							lhs = lhs->nextli;
							if (rhs) rhs = rhs->nextli;
						}
						fprintf(fout, "\n");
						currS = currS->val.declStmt.nextd;
					}

					break;
				default:
					// cgSTMT(ts->val.orig, 0, true);
					break;
				}
			}
		}

		cgGlobalDefs(ts->next);
	}
}

void cgPrototypes(TOPSTMT * ts)
{
	if(ts != NULL)
	{
		if (ts->isFunDecl) {
			if(strcmp(ts->val.funDecl.funName, "main") == 0) {
				fprintf(fout, "%s\n", "void __golite__main();");
			}
			else if(strcmp(ts->val.funDecl.funName, "init") == 0) {
				fprintf(fout, "%s%d();\n", "void __golite__init", initNum++);
			}
			else if(strcmp(ts->val.funDecl.funName, "_") == 0) {
			}
			else{
				if (ts->val.funDecl.result != NULL) {
					cgTYPE(ts->val.funDecl.result, 0);
					if (ts->val.funDecl.result->rt->kind == structType) fprintf(fout, " *");
				} else {
					fprintf(fout, "void");
				}
				fprintf(fout, " __golite__%s (", ts->val.funDecl.funName);
				cgParamList(ts->val.funDecl.paramList, 0, false, 0);
				fprintf(fout, ");\n");
			}
		} else {
			if(ts->val.orig != NULL)
			{
				STMT* currS;
				List * lhs;
				List * rhs;
				TYPE * ty;
				switch(ts->val.orig->kind) {
				case statementKindVarDeclaration:
					currS = ts->val.orig;
					while (currS && currS->kind == statementKindVarDeclaration) {

						lhs = currS->val.declStmt.identifiers;
						rhs = currS->val.declStmt.values;
						while (lhs && lhs->kind == listKindId) {
							if(currS->val.declStmt.type)
								{cgTYPE(currS->val.declStmt.type, 0); ty = currS->val.declStmt.type;}
							else
								{cgTYPE(rhs->current.expression->type, 0); ty = rhs->current.expression->type;}

							if (ty && ty->rt->kind == structType) {fprintf(fout, "*");}
							fprintf(fout, " ");
							cg_prefixID(lhs->current.id);
							fprintf(fout, ";\n");
							lhs = lhs->nextli;
							if(rhs) rhs = rhs->nextli;
						}
						fprintf(fout, "\n");
						currS = currS->val.declStmt.nextd;
					}

					break;
				case statementKindTypeDeclaration:
					// { // Keep hither the scope
					//      STMT* tmp = ts->val.orig;
					//      while(tmp != NULL)
					//      {
					//              if(tmp->val.typeDecl.type->kind == structType)
					//              {
					//                      findent(1);
					//                      fprintf(fout, "typedef ");
					//                      cgTYPE(tmp->val.typeDecl.type, 1);
					//                      fprintf(fout, " __golite_struct_%s;\n\n", tmp->val.typeDecl.identifier);
					//              }
					//
					//              tmp = tmp->val.typeDecl.nextT;
					//      }

					break;
				default:
					cgSTMT(ts->val.orig, 0, true);
					fprintf(fout,";\n");
					break;
				}
			}
		}

		cgPrototypes(ts->next);
	}
	else
	{
		fprintf(fout, "\n");
	}
}


void cgPROG(PROG * p, char ** fileOutName, STRUCTLIST * sll) {
	if (p != NULL) {

		fout = fopen(*fileOutName, "w");
		cg_general_code_init(fout, sll);
		initNum = 0;
		cgPrototypes(p->body);
		initNum = 0;
		fprintf(fout, "void %s(){\n", "__golite__definitions");

		cgGlobalDefs(p->body);

		fprintf(fout, "}\n");
		initNum = 0;
		cgTOPSTMT(p->body);

		fprintf(fout, "int main () {\n");
		fprintf(fout, "\t%s();\n", "__golite__definitions");
		for (int i = 0; i < initNum; i++) {
			fprintf(fout, "\t%s%d();\n", "__golite__init", i);
		}
		if(has_main) {
			fprintf(fout, "\t%s();\n", "__golite__main");
		}
		fprintf(fout, "\treturn 0;\n}\n\n");
		fclose(fout);
	}
}

void cgTOPSTMT(TOPSTMT * ts) {
	if (ts != NULL) {
		if (ts->isFunDecl) {
			if(strcmp(ts->val.funDecl.funName, "main") == 0) {
				fprintf(fout, "%s\n", "void __golite__main(){");
				cgSTMT(ts->val.funDecl.body->val.stmtlist, 0, 0);
				fprintf(fout, "}\n\n");
				has_main = 1;
			}
			else if(strcmp(ts->val.funDecl.funName, "init") == 0) {
				fprintf(fout, "%s%d%s\n", "void __golite__init", initNum++, "(){");
				cgSTMT(ts->val.funDecl.body->val.stmtlist, 0, 0);
				fprintf(fout, "}\n\n");
			}
			else if(strcmp(ts->val.funDecl.funName, "_") == 0) {
			}
			else{
				if (ts->val.funDecl.result != NULL) {
					cgTYPE(ts->val.funDecl.result, 0);
					if(ts->val.funDecl.result->rt->kind == structType) fprintf(fout, " *");
				} else {
					fprintf(fout, "void");
				}
				fprintf(fout, " __golite__%s (", ts->val.funDecl.funName);
				cgParamList(ts->val.funDecl.paramList, 0, false, 0);
				fprintf(fout, ") ");
				cgSTMT(ts->val.funDecl.body, 1, true);
				fprintf(fout, "\n\n");
			}
		} else {
			// cgSTMT(ts->val.orig, 0, true);
		}
		cgTOPSTMT(ts->next);
	}
} 

void cg_struct_equality(EXP* struct1Name, List* struct1Fields, EXP* struct2Name, List* struct2Fields) // Best way I figured to do this
{

	List* current1 = struct1Fields;
	List* current2 = struct2Fields;

	fprintf(fout, "( ");
	if(current1 == NULL && current2 == NULL) {fprintf(fout, "true");}
	while(current1 != NULL && current2 != NULL)
	{
		List* idlist1 = current1->current.param.idlist;
		List* idlist2 = current2->current.param.idlist;

		switch(current1->current.param.type->kind)
		{
		case intType:
		case floatType:
		case runeType:
		case stringType:
		case boolType:

			while(idlist1 != NULL && idlist2 != NULL)
			{
				if (strcmp(idlist1->current.id, "_") != 0 && strcmp(idlist2->current.id, "_") != 0 ) {
				cgEXP(struct1Name);
				fprintf(fout, "->__golite__%s == ", idlist1->current.id);
				cgEXP(struct2Name);
				fprintf(fout, "->__golite__%s", idlist2->current.id);
				}
				idlist1 = idlist1->nextli;
				idlist2 = idlist2->nextli;
			}					// }
			break;
		case nullType:
			break;
		case structType: // Only mostly sure this works

			while(idlist1 != NULL && idlist2 != NULL)
			{
				cg_struct_equalitys(idlist1->current.id, idlist1->current.param.type->val.fields, idlist2->current.id, idlist2->current.param.type->val.fields);
				idlist1 = idlist1->nextli;
				idlist2 = idlist1->nextli;
			}
			break;
		case arrayType:
			while(idlist1 != NULL && idlist2 != NULL)
			{
				fprintf(fout, "arrays_equal(");
				cgEXP(struct1Name);
				fprintf(fout, "->__golite__%s,", idlist1->current.id);
				cgEXP(struct2Name);
				fprintf(fout, "->__golite__%s", idlist2->current.id);
				fprintf(fout, ")");
				idlist1 = idlist1->nextli;
				idlist2 = idlist2->nextli;
			}
			break;
		case sliceType:
			while(idlist1 != NULL && idlist2 != NULL)
			{
				fprintf(fout, "slices_equal(");
				cgEXP(struct1Name);
				fprintf(fout, "->__golite__%s,", idlist1->current.id);
				cgEXP(struct2Name);
				fprintf(fout, "->__golite__%s", idlist2->current.id);
				fprintf(fout, ")");
				idlist1 = idlist1->nextli;
				idlist2 = idlist2->nextli;
			}//
			break;
		case unknownType:
			fprintf(stderr, "Error: 1 This error is supposed to not exists, so frick\n");
			break;         // I'll change what I can to remove this concern from existence
		}

		current1 = current1->nextli;
		current2 = current2->nextli;

		if(current1 != NULL && current2 != NULL)
		{
			fprintf(fout, " && ");
		}
	}

	fprintf(fout, " )");
}

void cg_struct_equalitys(char* struct1Name, List* struct1Fields, char* struct2Name, List* struct2Fields) { // Best way I figured to do this{
	List* current1 = struct1Fields;
	List* current2 = struct2Fields;
	
	fprintf(fout, "( ");
	if(current1 == NULL && current2 == NULL) {fprintf(fout,"true");}
	while(current1 != NULL && current2 != NULL)
	{
		List* idlist1 = current1->current.param.idlist;
		List* idlist2 = current2->current.param.idlist;

		switch(current1->current.param.type->kind)
		{
		case intType:
		case floatType:
		case runeType:
		case stringType:
		case boolType:
			while(idlist1 != NULL && idlist2 != NULL)
			{
				fprintf(fout, "%s->__golite__%s == %s->__golite__%s", struct1Name, idlist1->current.id,struct2Name,idlist2->current.id);
				idlist1 = idlist1->nextli;
				idlist2 = idlist2->nextli;
			}
			break;
		case nullType:
			break;
		case structType:
			while(idlist1 != NULL && idlist2 != NULL)
			{
				cg_struct_equalitys(idlist1->current.id, idlist1->current.param.type->val.fields, idlist2->current.id, idlist2->current.param.type->val.fields);
				idlist1 = idlist1->nextli;
				idlist2 = idlist2->nextli;
			}
			break;
		case arrayType:
			while(idlist1 != NULL && idlist2 != NULL)
			{
				fprintf(fout, "arrays_equal(%s->__golite__%s, %s->__golite__%s)",struct1Name, idlist1->current.id,struct2Name, idlist2->current.id);
				idlist1 = idlist1->nextli;
				idlist2 = idlist2->nextli;
			}
			break;
		case sliceType:
			while(idlist1 != NULL && idlist2 != NULL)
			{
				fprintf(fout, "slices_equal(%s->__golite__%s, %s->__golite__%s)",struct1Name, idlist1->current.id,struct2Name, idlist2->current.id);
				idlist1 = idlist1->nextli;
				idlist2 = idlist2->nextli;
			}
			break;
		case unknownType:
			fprintf(stderr, "Error: 1 This error is supposed to not exists, so frick\n"); // Wow, censoring my swears
			break;         // I'll change 	fprintf(fout, "\n");
		}

		current1 = current1->nextli;
		current2 = current2->nextli;

		if(current1 != NULL && current2 != NULL)
		{
			fprintf(fout, " && ");
		}
	}

	// fprintf(fout, " )");
}

void struct_init(char* structName, List* structFields){
	List* current = structFields;

	// fprintf(fout, "( ");


	while(current != NULL)
	{					// }
		List* idlist = current->current.param.idlist;


		while(idlist != NULL)
		{
			if (current->current.param.type && current->current.param.type->kind == structType){
				cgTYPE(current->current.param.type, 0);
				char tmpStructName [strlen(idlist->current.id) + strlen("tmpstruct__") + 3];
				char tmpStructNameLong [strlen(idlist->current.id) + strlen("tmpstruct__") + strlen("tmp__golite__") + 3];
				sprintf(tmpStructName, "tmpstruct__%d", ++rec_struct_count); //replace this;
				sprintf(tmpStructNameLong, "tmp__golite__%s", tmpStructName); //replace this;

				// fprintf(fout, "* %s = malloc(sizeof(", tmpStructNameLong);
				// cgTYPE(current->current.param.type, 0);
				// fprintf(fout, "));\n");
				//
				//
				// struct_init(tmpStructName, current->current.param.type->val.fields);
				// fprintf(fout, "%s", tmpStructName);
				// cg_prefixID(structName);
				// fprintf(fout, "->__golite__%s = %s", idlist->current.id, tmpStructNameLong);
				idlist = idlist->nextli;
				// fprintf(fout, ";\n");
			} else {
				fprintf(fout, "tmp");
				cg_prefixID(structName);
				fprintf(fout, "->__golite__%s = ",  idlist->current.id);
				// if (current->current.param.type && current->current.param.type->kind != structType)
				cg_initvar(current->current.param.type, idlist->current.id);
				idlist = idlist->nextli;
				fprintf(fout, ";\n");
			}
		}

		current = current->nextli;


		// fprintf(fout, " )");
	}
}

void cgStructCopy(STRUCTLIST * currSL){
	fprintf(fout, "__golite_struct_%d * struct_copy%d(__golite_struct_%d * toCopy){\n", currSL->struct_id,currSL->struct_id, currSL->struct_id);
	findent(1);
	fprintf(fout, "__golite_struct_%d * toRet = malloc(sizeof(__golite_struct_%d));\n", currSL->struct_id, currSL->struct_id);
	List * currFieldptr;
	List * currField = currSL->curr->val.fields;
	currFieldptr = currField;

	int blank_identifier = 0;
	findent(1);
	fprintf(fout, "if (toCopy) {\n");
	while(currField) {
		List * ids = currField->current.param.idlist;
		while(ids) {
			switch(currField->current.param.type->rt->kind) {
			case arrayType:
				findent(2);
				if(strcmp(ids->current.id, "_") != 0)
					fprintf(fout, "toRet->__golite__%s = array_copy(toCopy->__golite__%s);\n", ids->current.id, ids->current.id);
				else{
					fprintf(fout, "toRet->__golite__blank_%d = array_copy(toCopy->__golite__blank_%d);\n", blank_identifier, blank_identifier);
					blank_identifier++;
				}
				break;
			case sliceType:
				findent(2);
				if(strcmp(ids->current.id, "_") != 0)
					fprintf(fout, "toRet->__golite__%s = slice_copy(toCopy->__golite__%s);\n", ids->current.id, ids->current.id);
				else{
					fprintf(fout, "toRet->__golite__blank_%d = slice_copy(toCopy->__golite__blank_%d);\n", blank_identifier, blank_identifier);
					blank_identifier++;
				}
				break;
			case structType:
				findent(2);
				if(strcmp(ids->current.id, "_") != 0)
					fprintf(fout, "toRet->__golite__%s = struct_copy%d(toCopy->__golite__%s);\n", ids->current.id,currField->current.param.type->rt->struct_id,  ids->current.id);
				else{
					fprintf(fout, "toRet->__golite__blank_%d = struct_copy%d(toCopy->__golite__blank_%d);\n", blank_identifier,currField->current.param.type->rt->struct_id,  blank_identifier);
					blank_identifier++;
				}
				break;
			default:
				findent(2);
				if(strcmp(ids->current.id, "_") != 0)
					fprintf(fout, "toRet->__golite__%s = toCopy->__golite__%s;\n", ids->current.id, ids->current.id);
				else{
					fprintf(fout, "toRet->__golite__blank_%d = toCopy->__golite__blank_%d;\n",blank_identifier, blank_identifier);
					blank_identifier++;
				}
				break;
			}
			ids = ids->nextli;
		}
		currField = currField->nextli;
	}
	findent(1);
	fprintf(fout, "} else {\n");
		currField = currFieldptr;
		while(currField) {
		List * ids = currField->current.param.idlist;
		while(ids) {
			switch(currField->current.param.type->rt->kind) {
			default:
				// findent(2);
				if(strcmp(ids->current.id, "_") != 0) {
					findent(2);
					fprintf(fout, "toRet->__golite__%s = ", ids->current.id);
					cg_initvar(currField->current.param.type->rt, ids->current.id);
					fprintf(fout, ";\n");
				}
				break;
			}
			ids = ids->nextli;
		}
		currField = currField->nextli;
	}

	findent(1);
	fprintf(fout, "}\n");
	findent(1);
	fprintf(fout, "return toRet;\n}\n");

}

void cg_general_code_init(FILE* fout, STRUCTLIST * sl)
{
	char* includes = "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <stdbool.h>\n#include<math.h>\n\n#define append(slice, element) _Generic((element), void*: appendv, int:appendi,float:appendf,bool:appendb, double:appendf, struct DATA:appendSD)(slice,element)\n\n";
	fprintf(fout, "%s", includes);
	char* type_kind = "typedef enum {\n\tintType,\n\tfloatType,\n\truneType,\n\tstringType,\n\tboolType,\n\tnullType,\n\tstructType,\n\tarrayType,\n\tsliceType,\n\tunknownType\n} TypeKind;\n\n";
	fprintf(fout, "%s", type_kind);
	char* type_struct = "struct TYPE_BASE {\n\tTypeKind kind;\n\tint struct_id;\n\tint size;\nstruct TYPE_BASE* underlying;\nint length;};\n\n";
	fprintf(fout, "%s", type_struct);
	char* type_typedef = "typedef struct TYPE_BASE TYPE;\n\n";
	fprintf(fout, "%s", type_typedef);
	char* data_struct = "struct DATA {\n\tvoid* pointer;\n\tunsigned int capacity;\n\tunsigned int length;\n\tTYPE* underlying;\nint* refs;\n};\n\n";
	fprintf(fout, "%s", data_struct);
	char* array_typedef = "typedef struct DATA ARRAY;\n";
	fprintf(fout, "%s", array_typedef);
	char* slice_typedef = "typedef struct DATA SLICE;\n\n";
	fprintf(fout, "%s", slice_typedef);
	fprintf(fout, "SLICE slice_copy(SLICE slice);\n\n");

	char * forstringcasts = "char temp_stringcast[4*sizeof(char)];\n\nchar * string_cast(int num) {\n\t\tnum = num % 128;\n\t\tsprintf(temp_stringcast, \"%c\", num);\n\t\treturn temp_stringcast;\n}\n\n";
	fprintf(fout, "%s", forstringcasts);

	char* array_access = "void* array_access(ARRAY array_name, int index){\n\tif(index < 0){\n\t\tfprintf(stderr, \"Error: Array access out of bounds error\\n\");\n\t\texit(1);\n\t} else if (index >= array_name.length) {\n\t\tfprintf(stderr, \"Error: Array access out of bounds error\\n\");\n\t\texit(1);\n\t} else {\n\t\treturn (void*)((char*)array_name.pointer + index*array_name.underlying->size);\n\t}\n}\n\n";
	fprintf(fout, "%s", array_access);
	char* slice_access = "void* slice_access(SLICE slice_name, int index){\n\tif(index < 0){\n\t\tfprintf(stderr, \"Error: Slice access out of bounds error\\n\");\n\t\texit(1);\n\t} else if (index >= slice_name.length) {\n\t\tfprintf(stderr, \"Error: Slice access out of bounds error\\n\");\n\t\texit(1);\n\t} else {\n\t\treturn (slice_name.pointer + index);\n\t}\n}\n\n";
	fprintf(fout, "%s", slice_access);

	char* array_copy = "ARRAY array_copy(ARRAY array){\n\tARRAY new_array;\n\tnew_array.refs = NULL;\n\tnew_array.underlying = array.underlying;\n\tnew_array.capacity = array.capacity;\n\tnew_array.length = array.length;\n\tnew_array.pointer = malloc(array.capacity * array.underlying->size);\n\tif(array.underlying->kind == arrayType){\n\tfor(int i = 0; i < array.capacity; i++){\n\t((ARRAY*)new_array.pointer)[i] = array_copy(((ARRAY*)array.pointer)[i]);\n\t}\n\t}else if (array.underlying->kind == sliceType){\n\tfor(int i = 0; i < array.capacity; i++){\n\t((SLICE*)new_array.pointer)[i] = slice_copy(((SLICE*)array.pointer)[i]);\n\t}\n\t}else \n\t{memcpy(new_array.pointer, array.pointer, array.capacity*array.underlying->size);\n\t}\n\treturn new_array;}\n\n";
	fprintf(fout, "%s", array_copy);
	char* slice_copy = "SLICE slice_copy(SLICE slice){\n\tSLICE new_slice;\n\tnew_slice.refs = slice.refs;\n\tnew_slice.capacity = slice.capacity;\n\tnew_slice.length = slice.length;\n\tnew_slice.underlying = slice.underlying;\n\tnew_slice.pointer = slice.pointer;\n\treturn new_slice;\n}\n\n";
	fprintf(fout, "%s", slice_copy);
	fprintf(fout, "\n");

	fprintf(fout, "bool __golite__true = true;\nbool __golite__false = false;\n");

	char* arrays_equal = "bool arrays_equal(ARRAY array1, ARRAY array2){\n\tif(array1.underlying->kind == array2.underlying->kind){\n\t\tif(array1.length == array2.length){\n\t\t\tbool bool_tally = true;\n\t\t\tswitch(array1.underlying->kind){\n\t\t\t\tcase arrayType:\n\t\t\t\t\tfor(int i = 0; i < array1.length; i++){\n\t\t\t\t\t\tbool_tally &= arrays_equal(*(ARRAY*)(array1.pointer + i), *(ARRAY*)(array2.pointer + i));\n\t\t\t\t\t\tif(bool_tally == false) break;\n\t\t\t\t\t}\n\t\t\t\t\treturn bool_tally;\n\t\t\t\tcase sliceType:\n\t\t\t\t\treturn false; // Slices aren't comparable\n\t\t\t\tcase unknownType:\n\t\t\t\t\tfprintf(stderr, \"Warning: (2) <THIS IS IN arrays_equal(...) THIS ERROR SHOULDN'T BE POSSIBLE, FIX IT>\\n\"); // This error should be dealt with by the way I imagine this all eventually being implememted. If this arises, it likely implies a communication error\n\t\t\t\t\treturn false; // Shouln't happen, but not doing an exit, just a loud warning\n\t\t\t\tdefault:\n\t\t\t\t\tif(memcmp(array1.pointer, array2.pointer, array1.underlying->size * array1.length) == 0){\n\t\t\t\t\t\treturn true;\n\t\t\t\t\t} else {\n\t\t\t\t\t\treturn false;\n\t\t\t\t\t}\n\t\t\t\t}\n\t\t} else {\n\t\t\tfprintf(stderr, \"Warning: (3) <THIS IS IN arrays_equal(...) THIS ERROR SHOULDN'T BE POSSIBLE, FIX IT>\\n\"); // The errors like this should've been ruled out by the typechecker\n\t\t\treturn false; // Shouldn't happen, but not doing an exit, just a loud warning\n\t\t}\n\t} else {\n\t\tfprintf(stderr, \"Warning: (1) <THIS IS IN arrays_equal(...) THIS ERROR SHOULDN'T BE POSSIBLE, FIX IT>\\n\"); // The errors like this should've been ruled out by the typechecker\n\t\treturn false; // Shouldn't happen, but not doing an exit, just a loud warning\n\t}\n}\n\n";;
	fprintf(fout, "%s", arrays_equal);

	char* make_type = "TYPE* make_type(TypeKind kind, int size, TYPE* underlying, int capacity){\n\tTYPE* new_type = (TYPE*)malloc(sizeof(TYPE));\n\tnew_type->kind = kind;\n\tnew_type->size = size;\n\tTYPE* underlying_type = NULL;\n\tif(underlying)\n\t{\n\t\tunderlying_type = malloc(sizeof(TYPE));\n\t\t*underlying_type = *underlying;\n\t}\n\tnew_type->underlying = underlying_type;\n\tnew_type->length = capacity;\n\treturn new_type;\n}\n\n";
	fprintf(fout, "%s", make_type);
	char* make_slice = "SLICE make_slice(TYPE* underlying){\n\tSLICE new_slice;\n\tnew_slice.pointer = NULL;\n\tnew_slice.capacity = 0;\n\tnew_slice.refs = malloc(sizeof(int));\n\t*(new_slice.refs) = 1;\n\tnew_slice.length = 0;\n\tnew_slice.underlying = underlying;\n\treturn new_slice;\n}\n\n";
	fprintf(fout, "%s", make_slice);
	char* make_array = "ARRAY make_array(TYPE* underlying, int capacity){\n\tARRAY new_array;\n\tnew_array.pointer = malloc(underlying->size * capacity);\n\tif(underlying->kind == arrayType)\n\t{\n\t\tfor(int i = 0; i < capacity; i++){\n\t\t\t((ARRAY*)new_array.pointer)[i] = make_array((underlying->underlying), underlying->length);\n\t\t}\n\t} else if (underlying->kind == sliceType){\n\t\tfor(int i = 0; i < capacity; i++){\n\t\t\t((SLICE*)new_array.pointer)[i] = make_slice((underlying->underlying));\n\t\t}\n\t} else if (underlying->kind == structType) {\n\t\tfor(int i = 0; i < capacity; i++) {\n\t\t\t//((ARRAY*)new_array.pointer)[i] = make_struct(malloc(sizeof(underlying->size)));\n\t\t}\n\t} else {\n\t\tmemset(new_array.pointer, '\\0', underlying->size * capacity);\n\t}\n\tnew_array.refs = NULL;\n\tnew_array.capacity = capacity;\n\tnew_array.length = capacity;\n\tnew_array.underlying = underlying;\n\treturn new_array;\n}\n\n";
	fprintf(fout, "%s", make_array);


	fprintf(fout, "char * concat(char * str1, char * str2) {\nchar * dest = (char *) malloc(sizeof(*str1)+ sizeof(str1) + 2);\nstrcat(dest, str1);\nstrcat(dest, str2); return dest;\n}\n\n");
	fprintf(fout, "char * boolToString(int b) { if (b) return \"true\"; return \"false\";}\n\n");


	char* append = "SLICE appendv(SLICE slice, void* element)\n{\n\tif(slice.length == slice.capacity)\n\t{\n\t\tif (slice.capacity == 0)\n\t\t{\n\t\t\tslice.capacity = 2;\n\t\t\tslice.pointer = malloc(slice.underlying->size*2);\n\t\t\tmemset(slice.pointer, '\\0', slice.underlying->size*2);\n\t\t}\n\t\telse\n\t\t{\n\t\t\tslice.capacity *= 2;\n\t\t\t*(slice.refs) -= 1;\n\t\t\tvoid* temp = slice.pointer;\n\t\t\tslice.pointer = malloc(slice.underlying->size * slice.capacity);\n\t\t\tmemset(slice.pointer, '\\0', slice.underlying->size * slice.capacity);\n\t\t\tif(slice.underlying->kind == arrayType)\n\t\t\t{\n\t\t\t\tfor(int i = 0; i < slice.length; i++)\n\t\t\t\t{\n\t\t\t\t((ARRAY*)slice.pointer)[i] = array_copy(((ARRAY*)temp)[i]);\n\t\t\t\t}\n\t\t\t}\n\t\t\telse if (slice.underlying->kind == sliceType)\n\t\t\t{\n\t\t\t\tfor(int i = 0; i < slice.length; i++)\n\t\t\t\t{\n\t\t\t\t\t((SLICE*)slice.pointer)[i] = slice_copy(((SLICE*)temp)[i]);\n\t\t\t\t}\n\t\t\t}\n\t\t\telse \n\t\t\t{\n\t\t\t\tmemcpy(slice.pointer, temp, slice.underlying->size * slice.capacity);\n\t\t\t}\n\t\t\tint* temp2 = slice.refs;\n\t\t\tslice.refs = malloc(sizeof(int));\t\t\t\t*(slice.refs) = 1;\n\t\t\tif(slice.pointer != NULL && *(slice.refs) == 0)\n\t\t\t{\n\t\t\t\tfree(temp);\n\t\t\t\tfree(temp2);\n\t\t\t}\n\t\t}\n\t}\n\tif(slice.underlying->kind== arrayType) ((ARRAY*)slice.pointer)[slice.length] = array_copy(*((ARRAY*)element)); else\n\tmemcpy(slice.pointer + slice.length*slice.underlying->size, element, slice.underlying->size);\n\tslice.length += 1;\n\treturn slice;\n}\n\n";
	fprintf(fout, "%s", append);

	char* lens_and_caps = "int cap(struct DATA data){\n\treturn data.capacity;\n}\n\nint len(struct DATA data){\n\treturn data.length;\n}\n\n";
	fprintf(fout, "%s", lens_and_caps);

	char* array_assign = "ARRAY array_assign(ARRAY array1, ARRAY array2){\n\tif(array1.pointer != NULL)free(array1.pointer);\n\tarray1.pointer = array2.pointer;\n\n\tarray1.length = array2.length;\n\tarray1.capacity = array2.capacity;\n\treturn array1;\n}\n\n";
	fprintf(fout, "%s", array_assign);

	char* slice_assign = "SLICE slice_assign(SLICE slice1, SLICE slice2){\n\tSLICE tmp = slice_copy(slice2);\n\t*(tmp.refs) += 1;\n\t*(slice1.refs) -= 1;\n\n\tif(*(slice1.refs) == 0)\n\t{\n\t\tfree(slice1.refs);\n\t\tfree(slice1.pointer);\n\t}\n\n\tslice1 = tmp;\n\n\treturn slice1;\n}\n\n";
	fprintf(fout, "%s", slice_assign);

	char* print_bool = "void print_bool(bool value){\n\tprintf(\"%s\", value?\"true\":\"false\");\n}\n\n";
	fprintf(fout, "%s", print_bool);

	char* print_float = "void print_float(float value){\n\t(value == 0.0)?printf(\"+0.000000e+000\"):printf(\"%+e\", value);\n}\n\n";        // Special case for zero since it was being weird for me
	fprintf(fout, "%s", print_float);

	char* stradd = "char* stradd(char* str1,char* str2){\n\tint length1 = strlen(str1);\n\tint length2 = strlen(str2);\n\tchar* new = (char*)malloc(sizeof(char)*(length1 + length2));\n\tfor(int i = 0; i < length1; i++) new[i] = str1[i];\n\tfor(int i = length1; i < length1 + length2; i++) new[i] = str2[i - length1];\n\tnew[length1 + length2] = '\\0';\n\treturn new;\n}\n\n";
	fprintf(fout, "%s", stradd);

	char* append2 = "SLICE appendSD(SLICE slice, struct DATA element){	return appendv(slice, &element);}\n\nSLICE appendi(SLICE slice, int element){	return appendv(slice, &element);}\n\nSLICE appendf(SLICE slice, float element){return appendv(slice, &element);}\n\nSLICE appendb(SLICE slice, bool element){return appendv(slice, &element);}\n\n";
	fprintf(fout, "%s", append2);

	fprintf(fout, "void* tmp_struct; // I was gonna use this for struct equality, but nvm\n\n");

	STRUCTLIST * currSL = sl;
	while (currSL) {
		findent(0);
		fprintf(fout, "typedef ");
		fprintf(fout, "struct { \n");
		cgParamList(currSL->curr->val.fields, 0, true, 1);
		findent(1);
		fprintf(fout, "\n}");
		fprintf(fout, " __golite_struct_%d;\n\n", currSL->struct_id);
		currSL = currSL->next;
	}

	currSL = sl;
	while(currSL) {
		cgStructCopy(currSL);
		currSL = currSL->next;
	}
}
