#include "weed.h"


bool weed_source(PROG* program)
{
	TOPSTMT* top = program->body;
	bool success = true;
	if(strcmp(program->packageName,"_") == 0)
	{
		fprintf(stderr, "Error: %s not a valid package name\n", program->packageName);
		success = false;
	}
	
	while(success && top != NULL)
	{
		if(top->isFunDecl)
		{
			success &= weed_statement(top->val.funDecl.body, false, false, false);
		}
		else
		{
			success &= weed_statement(top->val.orig, false, false, false);
		}

		top = top->next;
	}

	return success;
}

bool weed_statement(STMT* statement, bool break_allowed, bool continue_allowed, bool default_allowed)
{
	STMT* current = statement;
	bool success = true;
	int check_1;
	
	
	while(success && current != NULL)
	{
			
		switch(current->kind)
		{
		case statementKindEmpty:
			// Blank
			break;
		case statementKindBlock:
			success &= weed_statement(current->val.stmtlist, break_allowed, continue_allowed, default_allowed);
			break;
		case statementKindExpression:
			//success &= weed_expression(current->val.expression);
			check_1 = (current->val.expression->kind == expressionKindFunctionCall) ? true : false;
			if(check_1 == false)
			{
				fprintf(stderr, "Error: (line %d) Only a function may be an expression statement\n", current->lineno);
			}
			success &= check_1;
			break;
		case statementKindAssign:
			if(list_count(current->val.assignStmt.lhs) == list_count(current->val.assignStmt.rhs))
			{
				//success &= weed_expressionList(current->val.assignStmt.rhs);
			}
			else
			{
				fprintf(stderr, "Error: (line %d) Number of identifiers does not match number of expressions in assignment\n", current->lineno);
				success = false;
			}
			break;
		case statementKindAssignOp:
			//success &= weed_expression(current->val.assignopStmt.expression);
			break;
		case statementKindShortAssign:

			if(list_count(current->val.assignStmt.lhs) == list_count(current->val.assignStmt.rhs))
			{
				success &= weed_identlist(current->val.assignStmt.lhs, current->lineno);
			}
			else
			{
				fprintf(stderr, "Error: (line %d) Number of identifiers does not match number of expressions in assignment\n", current->lineno);
				success = false;
			}
			//Makes sure all LHS expressions are actually identifiers
			List * currLHS = current->val.assignStmt.lhs;
			while (currLHS) {
				if (currLHS->kind != listKindExp  ||currLHS->current.expression->kind != expressionKindIdentifier) {
					fprintf(stderr, "Error: (line %d) Left hand side of a short declaration must be identifier(s)\n", current->lineno);
					success = false;
				}
				currLHS = currLHS->nextli;
			}
			break;
		case statementKindVarDeclaration:
			if(current->val.declStmt.values != NULL && list_count(current->val.declStmt.identifiers) == list_count(current->val.declStmt.values))
			{
				//success &= weed_expressionList(current->val.assignStmt.rhs);
				weed_statement(current->val.declStmt.nextd, break_allowed, continue_allowed, default_allowed);
			}
			else if(current->val.declStmt.values != NULL)
			{
				fprintf(stderr, "Error: (line %d) Number of identifiers does not match number of expressions\n", current->lineno);
				success = false;
			}
			break;
		case statementKindTypeDeclaration:
			// Not sure I need anything here
			break;
		case statementKindIncrement:
			// Nor here
			break;
		case statementKindDecrement:
			// Nor here too
			break;
		case statementKindPrint:
			// success &= weed_expressionList(current->val.printList);
			break;
		case statementKindPrintln:
			// success &= weed_expressionList(current->val.printList);
			break;
		case statementKindReturn:
			// success &= weed_expressionList(current->val.expression);
			break;
		case statementKindIf:
			success &= weed_statement(current->val.ifStmt.initStmt, break_allowed, continue_allowed, default_allowed);
			// success &= weed_expression(current->val.ifStmt.condition);
			success &= weed_statement(current->val.ifStmt.ifBody, break_allowed, continue_allowed, default_allowed);
			success &= weed_statement(current->val.ifStmt.elseBody, break_allowed, continue_allowed, default_allowed);
			success &= weed_statement(current->val.ifStmt.elseif, break_allowed, continue_allowed, default_allowed);
			break;
		case statementKindSwitch:
			success &= weed_statement(current->val.switchStmt.initStmt, false, false, false);
			//success &= weed_expression(current->val.switchStmt.switchExp);
			success &= weed_switch(current->val.switchStmt.caseStmts, true, continue_allowed, true);
			break;
		//case statementKindCase:
		//    break;
		case statementKindFor:
			//struct { EXP* cond; STMT* lhs; STMT* rhs;  STMT* body; } forStmt;
			//success &= weed_expression(current->val.switchStmt.switchExp);
			success &= weed_statement(current->val.forStmt.lhs, false, false, false);
			if (current->val.forStmt.rhs && current->val.forStmt.rhs->kind == statementKindShortAssign) {
				fprintf(stderr, "Error: (line %d)for loop post statement may not be a short declaration\n", current->lineno);
				success = false;
			}
			else
				success &= weed_statement(current->val.forStmt.rhs, false, false, false);
			success &= weed_statement(current->val.forStmt.body, true, true, default_allowed);
			break;
		case statementKindBreak:
			if(!break_allowed)
			{
				fprintf(stderr, "Error: (line %d) break statements are not allowed in this context\n", current->lineno);
				success = false;
			}
			break;
		case statementKindContinue:
			if(!continue_allowed)
			{
				fprintf(stderr, "Error: (line %d) continue statements are not allowed in this context\n", current->lineno);
				success = false;
			}
			break;
		}
		current = current->next;
	}
	return success;
}

bool weed_switch(SWITCHCASE* caseStmts, bool break_allowed, bool continue_allowed, bool default_allowed)
{
	SWITCHCASE* current = caseStmts;
	bool success = true;
	bool allow_defaults = default_allowed;

	while(success && current != NULL)
	{
		if(current->isDefault && allow_defaults)
		{
			allow_defaults = false;
			success &= weed_statement(current->statement, break_allowed, continue_allowed, allow_defaults);
			// success &= weed_expressionList(current->exprlist);
		}
		else if(current->isDefault && (!allow_defaults))
		{
			fprintf(stderr, "Error: (line %d) Only one default allowed per switch/case\n", current->lineno);
			success = false;
		}
		else
		{
			success &= weed_statement(current->statement, break_allowed, continue_allowed, allow_defaults);
			// success &= weed_expressionList(current->exprlist);
		}
		current = current->next;
	}
	return success;
}

bool weed_identlist(List * list, int lineno){
	while (list) {
		if (list->current.expression && list->current.expression->kind == expressionKindIdentifier && !list->current.expression->paren)
			list = list->nextli;
		else{
			fprintf(stderr, "Error: (line %d) Left hand sides of short declarations have to be pure identifier lists", lineno);
			return false;
		}
	}
	return true;
}

int list_count(List* list)
{
	int i;
	List* current = list;
	for(i = 0; current != NULL; current = current->nextli,  i++);
	return i;
}
