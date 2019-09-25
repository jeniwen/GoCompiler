
#include "tree.h"
#include "pretty.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

//int depth;

void indent(int indentation) {
	for (int tabs = 0; tabs < indentation; tabs++) {
		printf("\t");
	}
}
/*
   void unindent() {
        for (int tabs = 0; tabs < depth; tabs++) {
                printf("\b\b\b");
        }
   }
 */

void prettyList(List * l) {
	if (l != NULL) {
		List * curr = l;
		if (curr->kind == listKindExp) {
			while (curr != NULL) {
				prettyEXP(curr->current.expression);
				curr = curr->nextli;
				if (curr!= NULL) {printf(",");};
			}
		} else if (curr->kind == listKindParam) {
			while (curr != NULL) {
				prettyList(curr->current.param.idlist);
				if (curr->current.param.type != NULL) {
					printf(" ");
					prettyTYPE(curr->current.param.type, 0);
				}
				curr = curr->nextli;
				if (curr!= NULL) {printf(",");};
			}
		}
		else if (curr->kind == listKindId) {
			while (curr != NULL) {
				printf("%s", curr->current.id);
				curr = curr->nextli;
				if (curr!= NULL) {printf(",");}
			}
		}
	}
}



void prettyEXP(EXP * e) {
	if (e != NULL) {
		char * sym = "";

		switch (e->kind) {
		case expressionKindIdentifier: printf("%s", e->val.identifier); break;
		case expressionKindIntLiteral:
			printf("%d", e->val.intLiteral);
			break;
		case expressionKindFloatLiteral: printf("%f", e->val.floatLiteral); break;
		case expressionKindRuneLiteral:
			// if (e->val.runeLiteral[0] == '\\') {
			printf("\'");
			if (e->val.runeLiteral[1] == '\\') {
				printf("%c",'\\');
				printf("%c",e->val.runeLiteral[2]);
			} else {
				printf("%c", e->val.runeLiteral[1]);
			}
			printf("\'");
			break;
		case expressionKindBoolLiteral: printf("%s",  e->val.boolLiteral ? "true" : "false"); break;
		case expressionKindStringLiteral: printf("%s", e->val.stringLiteral); break;
		case expressionKindStructFieldAccess: prettyEXP(e->val.structAccess.lhs); printf(".%s", e->val.structAccess.fieldIdent); break;
		case expressionKindUnaryPlus: sym = "+"; printf("(%s", sym); prettyEXP(e->val.unary); printf(")"); break;
		case expressionKindUnaryMinus: sym = "-";  printf("(%s", sym); prettyEXP(e->val.unary); printf(")"); break;
		case expressionKindUnaryNot: sym = "!";  printf("(%s", sym); prettyEXP(e->val.unary); printf(")"); break;
		case expressionKindBitComplement: sym = "^"; printf("(%s", sym); prettyEXP(e->val.unary); printf(")"); break;
		case expressionKindAddition: sym = " + ";  printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindSubtraction: sym = " - "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindMultiplication: sym = " * "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindDivision: sym = " / "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindOr: sym = " || "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindAnd: sym = " && "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindEquals: sym = " == "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindNotEquals: sym = " != "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindLower: sym = " < "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindLeq: sym = " <= "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindGreater: sym = " > "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindGeq:  sym = " >= "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindBitwiseOr: sym = " | "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindBitwiseXor: sym = " ^ "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindModulo: sym = " % "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindLeftShift: sym = " << "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindRightShift: sym = " >> "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindBitwiseAnd: sym = " & "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")");  break;
		case expressionKindBitwiseAndNot: sym = " &^ "; printf("("); prettyEXP(e->val.binary.lhs); printf("%s", sym); prettyEXP(e->val.binary.rhs); printf(")"); break;
		case expressionKindBitClear: break;
		case expressionKindFunctionCall:
			prettyEXP(e->val.funCall.funName);
			printf("(");
			prettyList(e->val.funCall.args);
			printf(")"); break;
		case expressionKindAppend:
			printf("append(");
			prettyEXP(e->val.append.appendto);
			printf(", ");
			prettyEXP(e->val.append.appendfrom);
			printf(")"); break;
		case expressionKindCap:
			printf("cap("); prettyEXP(e->val.expression); printf(")"); break;
		case expressionKindLen:
			printf("len("); prettyEXP(e->val.expression); printf(")"); break;
		case expressionKindArrayAccess:
			prettyEXP(e->val.arrayAccess.identifier);
			printf("[");
			prettyEXP(e->val.arrayAccess.index);
			printf("]");

		case expressionKindTypeCast: break;
		}

	}
}


void prettyTYPE(TYPE * t, int indentation) {
	if (t != NULL) {
		switch (t->kind) {
		case unknownType: printf("%s", t->val.ident); break;
		case intType: printf("int"); break;
		case floatType: printf("float64"); break;
		case runeType: printf("rune"); break;
		case stringType: printf("string"); break;
		case boolType: printf("bool"); break;
		case structType:
			printf("struct { \n");
			prettyList(t->val.fields);
			indent(indentation);
			printf("\n}");
			break;
		case arrayType:
			printf("[%d]", t->val.arrayData.capacity);
			prettyTYPE(t->val.arrayData.entryType, indentation);
			break;
		case sliceType:
			printf("[]");
			prettyTYPE(t->val.arrayData.entryType, indentation);
			break;
		default: break;
		}
	}

}

void prettySTMT(STMT * s, int indentation, bool line_end) {
	while (s != NULL) {
		STMT * currS;
		switch (s->kind) {
		case statementKindEmpty: if (line_end) printf("\n"); break;
		case statementKindExpression: if (line_end) indent(indentation); prettyEXP(s->val.expression); if (line_end) printf("\n"); break;
		case statementKindAssign:
			if (line_end) indent(indentation);
			prettyList(s->val.assignStmt.lhs);
			printf(" = ");
			prettyList(s->val.assignStmt.rhs);
			if (line_end) printf("\n");
			break;
		case statementKindAssignOp:
			if (line_end) indent(indentation);
			prettyEXP(s->val.assignopStmt.identifier);
			char * sym = "";
			switch (s->val.assignopStmt.aop) {
			case assignopKindRemainder: sym = "%="; break;
			case assignopKindMultiplication: sym = "*="; break;
			case assignopKindDivision: sym = "/="; break;
			case assignopKindPlus: sym = "+="; break;
			case assignopKindMinus: sym = "-="; break;
			case assignopKindLShift: sym = "<<="; break;
			case assignopKindRShift: sym = ">>="; break;
			case assignopKindBWXor: sym = "^="; break;
			case assignopKindBWAndNot: sym = "&^="; break;
			case assignopKindBWAnd: sym = "&="; break;
			case assignopKindBWOr: sym = "|="; break;
			}
			printf(" %s ", sym);
			prettyEXP(s->val.assignopStmt.expression);
			if (line_end) printf("\n");
			break;
		case statementKindShortAssign:
			if (line_end) indent(indentation);
			prettyList(s->val.assignStmt.lhs);
			printf(" := ");
			prettyList(s->val.assignStmt.rhs);
			if (line_end) printf("\n");
			break;
		case statementKindIncrement:
		case statementKindDecrement:
			if (line_end) indent(indentation);
			prettyEXP(s->val.incdec.exp);
			if (s->val.incdec.inc) printf("++"); else printf("--");
			if (line_end) printf("\n");
			break;
		case statementKindPrint:
			if (line_end) indent(indentation);
			printf("print(");
			prettyList(s->val.printList);
			printf(");");
			if (line_end) printf("\n");
			break;
		case statementKindPrintln:
			if (line_end) indent(indentation);
			printf("println(");
			prettyList(s->val.printList);
			printf(");");
			if (line_end) printf("\n");
			break;
		case statementKindTypeDeclaration:
			if (line_end) indent(indentation);
			currS = s;
			while (currS != NULL) {
				printf("type %s ", currS->val.typeDecl.identifier);
				prettyTYPE(currS->val.typeDecl.type, indentation);
				printf("\n");
				currS = currS->val.typeDecl.nextT;
			}
			break;
		case statementKindVarDeclaration:
			currS = s;
			//unindent();
			while (currS != NULL && currS->kind == statementKindVarDeclaration && currS->val.declStmt.identifiers != NULL) {
				//indent();
				if (line_end) indent(indentation);
				printf("var ");
				prettyList(currS->val.declStmt.identifiers);
				// I changed this, double check it's right - Nathan
				printf(" ");   // This
				if (currS->val.declStmt.type != NULL) prettyTYPE(currS->val.declStmt.type, indentation);
				if (currS->val.declStmt.values != NULL) {
					printf(" = ");   // And this
					prettyList(currS->val.declStmt.values);
				}
				printf("\n");
				currS = currS->val.declStmt.nextd;
			}
			break;
		case statementKindIf:
			if (line_end) indent(indentation);
			printf("if");
			if (s->val.ifStmt.initStmt != NULL) {prettySTMT(s->val.ifStmt.initStmt, 0, false); printf(";");}
			if (s->val.ifStmt.condition != NULL) prettyEXP(s->val.ifStmt.condition);
			prettySTMT(s->val.ifStmt.ifBody, indentation, false);
			if (s->val.ifStmt.elseBody != NULL) {
				printf(" else ");
				prettySTMT(s->val.ifStmt.elseBody, indentation, false);
			}
			if (s->val.ifStmt.elseif != NULL) {
				//indent();
				printf(" else ");
				prettySTMT(s->val.ifStmt.elseif, indentation, false);
			}
			printf("\n");
			break;
		case statementKindSwitch:
			if (line_end) indent(indentation);
			printf("switch ");
			if (s->val.switchStmt.initStmt != NULL) {prettySTMT(s->val.switchStmt.initStmt, 0, false); printf(";");}
			if (s->val.switchStmt.switchExp != NULL) prettyEXP(s->val.switchStmt.switchExp);
			printf("{\n");
			if (s->val.switchStmt.caseStmts != NULL) prettySWITCHCASE(s->val.switchStmt.caseStmts, indentation+1);
			indent(indentation);
			printf("}\n");
			break;

		case statementKindReturn:
			if (line_end) indent(indentation);
			printf("return ");
			prettyEXP(s->val.expression);
			printf(";\n"); break;
		case statementKindBreak: if (line_end) indent(indentation); printf("break;\n"); break;
		case statementKindContinue: if (line_end) indent(indentation); printf("continue;\n"); break;
		case statementKindFor:
			if (line_end) indent(indentation);
			printf("for ");
			if (s->val.forStmt.lhs != NULL) {prettySTMT(s->val.forStmt.lhs, 0, false); printf(";");}
			if (s->val.forStmt.cond != NULL) prettyEXP(s->val.forStmt.cond);
			if (s->val.forStmt.rhs != NULL) {printf(";"); prettySTMT(s->val.forStmt.rhs, 0, false);}
			prettySTMT(s->val.forStmt.body, indentation, true);

			break;
		case statementKindBlock:
			//unindent();
			printf(" {\n");
			//depth++;
			prettySTMT(s->val.stmtlist, indentation+1, true);
			// printf("\n");
			//depth--;
			indent(indentation);
			printf("}");
			if(line_end) printf("\n");
			break;
		}
		s = s->next;
	}
}

void prettySWITCHCASE(SWITCHCASE *s, int indentation){
	if (s != NULL) {
		SWITCHCASE *curr = s;
		while (curr != NULL) {
			indent(indentation);
			//depth++;
			if (curr->isDefault) printf("default: \n");
			else {
				printf("case ");
				prettyList(curr->exprlist);
				printf(": \n");
			}
			//depth++;
			prettySTMT(curr->statement, indentation+1, true);
			//depth--;
			curr = curr->next;
			//depth--;
			//printf("\n");
		}
	}
}

void prettyPROG(PROG *p) {
	if (p != NULL) {
		printf("package %s\n\n", p->packageName);
		prettyTOPSTMT(p->body);
	}
}

void prettyTOPSTMT(TOPSTMT * ts) {
	if (ts != NULL) {
		TOPSTMT *curr = ts;
		while (curr != NULL) {
			//depth = 0;
			if (curr->isFunDecl) {
				printf("func ");
				printf("%s (", curr->val.funDecl.funName);
				if (curr->val.funDecl.paramList != NULL) {
					prettyList(curr->val.funDecl.paramList);
				}
				printf(") ");
				if (curr->val.funDecl.result != NULL) {
					prettyTYPE(curr->val.funDecl.result, 0);
				}
				if (curr->val.funDecl.body != NULL ) {
					prettySTMT(curr->val.funDecl.body, 0, true);
				}
				printf("\n\n");
			}
			else {
				prettySTMT(curr->val.orig, 0, true);
			}
			curr = curr->next;
		}
	}
}
