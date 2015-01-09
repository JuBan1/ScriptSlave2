#ifndef RDPARSER_H
#define RDPARSER_H

#include "TokenStack.h"
#include "ASTNode.h"
#include "Util.h"
#include "ExprParser.h"
;
//#define PREPARE_NODE(nodeType) nodeType ## Ptr node = std::make_unique<nodeType>(); bool noTrace = false; g_prodName = #nodeType;
#define PREPARE_NODE(nodeType) nodeType ## Ptr node = std::make_unique<nodeType>(); bool noTrace = false; g_prodName = #nodeType; node->SetToken(g_tokenStack->GetCurrentToken());

#define TRY_MATCH(x) g_tokenStack->PushIndex(); if( x ) { g_tokenStack->DiscardIndex(); in = std::move(node); return true; } g_tokenStack->PopIndex(); if(noTrace) return false;

#define NO_TRACEBACK (noTrace = true)

#define MATCH_RETURN return false;

#define CONT_MATCH(x) size_t numAlloc; while(true){numAlloc=node->NumChildren(); g_tokenStack->PushIndex(); if(!(x)){g_tokenStack->PopIndex();break;}g_tokenStack->DiscardIndex();} node->PopChildren(node->NumChildren() - numAlloc);
#define SINGLE_MATCH(x) g_tokenStack->PushIndex(); if(!(x)){g_tokenStack->PopIndex();} else g_tokenStack->DiscardIndex();


#define STAR(x) [&]() -> bool { CONT_MATCH( x ); return true; }()

#define OPTIONAL(x) [&]() -> bool { SINGLE_MATCH( x ); return true; }()

ExprParser* g_exprParser;
TokenStack* g_tokenStack;
std::string g_prodName;

bool _match(TokenType ttype){
	return g_tokenStack->GetNextToken(g_prodName).type == ttype;
}

#define match(x) _match(TokenType::x)

bool match_type(TypePtr& node){
	const Token& t = g_tokenStack->GetNextToken(g_prodName);

	if (t.type == TokenType::Ident){
		node = std::make_unique<Type>(t.GetTokenValue());
		node->SetToken(t);

		g_tokenStack->PushIndex();
		if (match(LBracket) && match(RBracket)){
			node->SetIsArray(true);
			g_tokenStack->DiscardIndex();
		}
		else{
			node->SetIsArray(false);
			g_tokenStack->PopIndex();
		}

		return true;
	}
	return false;
}

bool match_ident( IdentPtr& node ){
	const Token& t = g_tokenStack->GetNextToken( g_prodName );

	if (t.type == TokenType::Ident){
		node = std::make_unique<Ident>(t.GetTokenValue());
		node->SetToken( t );
		return true;
	}
	return false;
}

bool match_ident( TokenStack& ts ){
	return g_tokenStack->GetNextToken(g_prodName).type == TokenType::Ident;
}

bool match_expression(ExprPtr& in){
	return g_exprParser->MatchExpression(in);
}

bool match_named_expr(ExprPtr& in){
	return g_exprParser->MatchNamedExpression(in);
}

bool match_stmtblock( StmtBlockPtr& in );

bool match_ident_list( IdentListPtr& in ){
	PREPARE_NODE( IdentList );
	TRY_MATCH(
		match_ident( node->Push() ) &&
		STAR(
			match( Comma ) &&
			match_ident( node->Push() )
		)
	);

	MATCH_RETURN;
}

bool match_arg_list( ArgListPtr& in ){
	//arg_list -> expr (, expr)*
	PREPARE_NODE( ArgList );
	TRY_MATCH(
		match_expression( node->Push() ) &&
		STAR(
			match( Comma ) &&
			match_expression( node->Push() )
		)
	);

	//arg_list -> e
	in = std::make_unique<ArgList>();
	return true;
}

bool match_stmt( StmtPtr& in ){
	g_tokenStack;

	{//stmt -> while ( expr ) stmt
		PREPARE_NODE( StmtWhile );
		TRY_MATCH( match( While ) &&
			NO_TRACEBACK &&
			match( LParen ) &&
			match_expression( node->GetExprRef() ) &&
			match( RParen ) &&
			match_stmt( node->GetBodyRef() )
		);
	}

	{//stmt -> ident ( argList )
		PREPARE_NODE( StmtFuncCall );
		TRY_MATCH(
			match_ident( node->GetNameRef() ) &&
			match( LParen ) &&
			match_arg_list( node->GetArgListRef() ) &&
			match( RParen ) &&
			match( Semicolon )
		);
	}

	{//stmt -> return ;
		PREPARE_NODE(StmtBreak);
		TRY_MATCH(match(Break) &&
			match(Semicolon)
		);
	}

	{//stmt -> return ;
		PREPARE_NODE( StmtReturn );
		TRY_MATCH( match( Return ) &&
			match( Semicolon )
		);
	}

	{//stmt -> return expr ;
		PREPARE_NODE( StmtReturn );
		TRY_MATCH( match( Return ) &&
			match_expression( node->GetExprRef() ) &&
			match( Semicolon )
		);
	}

	{//stmt -> type ident ( = expr )? ;
		PREPARE_NODE( StmtVarDecl );
		TRY_MATCH(
			match_type( node->GetTypeRef() ) &&
			match_ident( node->GetNameRef() ) &&
			OPTIONAL(
				match(Assign) &&
				match_expression(node->GetExprRef())
			) &&
			match( Semicolon )
		);
	}

	{//stmt -> ident = expr ;
		PREPARE_NODE(StmtAssign);
		TRY_MATCH(
			match_ident(node->GetNameRef()) &&
			match(Assign) &&
			match_expression(node->GetExprRef()) &&
			match(Semicolon)
			);
	}

	if( g_tokenStack->GetCurrentToken().type == TokenType::If ){ //if-then currently parses the entire block twice. Ugly, but not an error.
		{//stmt -> if '(' expr ')' stmt else stmt
			PREPARE_NODE( StmtIfThenElse );
			TRY_MATCH( match( If ) &&
				match( LParen ) &&
				match_expression( node->GetExprRef() ) &&
				match( RParen ) &&
				match_stmt( node->GetThenRef() ) &&
				match( Else ) &&
				NO_TRACEBACK &&
				match_stmt( node->GetElseRef() )
			);
		}

		{//stmt -> if '(' expr ')' stmt
			PREPARE_NODE( StmtIfThen );
			TRY_MATCH( match( If ) &&
				match( LParen ) &&
				NO_TRACEBACK &&
				match_expression( node->GetExprRef() ) &&
				match( RParen ) &&
				match_stmt( node->GetThenRef() )
			);
		}
		return false;
	}

	{//stmt -> stmt_block
		PREPARE_NODE( StmtBlock );
		TRY_MATCH(
			match_stmtblock( node )
		);
	}

	MATCH_RETURN
}

bool match_stmtblock( StmtBlockPtr& in ){
	//stmt_block -> stmt*
	PREPARE_NODE( StmtBlock );	
	TRY_MATCH( 
		match(LBrace) &&
		STAR(
			match_stmt( node->Push() )
		) &&
		match(RBrace)
	);

	MATCH_RETURN
}

bool match_param( ParamPtr& in ){
	//param -> ident ident
	PREPARE_NODE( Param );
	TRY_MATCH(
		match_type( node->GetTypeRef() ) &&
		match_ident( node->GetNameRef() )
	);
	
	MATCH_RETURN;
}

bool match_param_list( ParamListPtr& in ){
	//param_list -> param (, param)*
	PREPARE_NODE( ParamList );
	TRY_MATCH(
		match_param( node->Push() ) &&
		STAR(
			match( Comma ) &&
			match_param( node->Push() )
		)
	);

	//param_list -> e
	in = std::make_unique<ParamList>();
	return true;
}
/*
bool match_func_def(FuncDefPtr& in){
	PREPARE_NODE(FuncDef);

	//func_def -> ident ident ( param_list ) { stmt_block }
	TRY_MATCH(
		match_type(node->GetRetTypeRef()) &&
		match_ident(node->GetNameRef()) &&
		match(LParen) &&
		match_param_list(node->GetParamListRef()) &&
		match(RParen) &&
		match_stmtblock(node->GetStmtBlockRef())
		);

	MATCH_RETURN;
}*/

bool match_global_stmt(GlobalStmtPtr& in){

	{
		PREPARE_NODE(FuncDef);
		TRY_MATCH(
			match_type(node->GetRetTypeRef()) &&
			match_ident(node->GetNameRef()) &&
			match(LParen) &&
			match_param_list(node->GetParamListRef()) &&
			match(RParen) &&
			match_stmtblock(node->GetStmtBlockRef())
			);
	}

	{
		PREPARE_NODE(GlobVarDef);
		TRY_MATCH(
			match_type(node->GetTypeRef()) &&
			match_ident(node->GetNameRef()) &&
			match(Semicolon)
		);
	}
	
	MATCH_RETURN;
}

bool match_start( TokenStack& ts,  StartBlockPtr& in ){
	g_exprParser = new ExprParser( ts );
	g_tokenStack = &ts;
	g_tokenStack->SetFurthestProductionName( "StartBlock" );

	//start -> func_def*
	PREPARE_NODE( StartBlock );

	TRY_MATCH(
		STAR(
			match_global_stmt(node->Push())
		)
	);

	//TODO: delete this, doesn't get called most of the time
	delete g_exprParser;
	MATCH_RETURN;
}

#endif