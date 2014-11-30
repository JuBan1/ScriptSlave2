#include "BaseVisitor.h"

#include "ASTNode.h"

#define DEFINE_DEFAULT_VISIT(Type) void BaseVisitor::visit( Type* n, bool last ){\
INNODE;\
OUTNODE;\
}

#define DEFINE_VISIT_ARRAY(Type, Name1) void BaseVisitor::visit( Type* n, bool last ){\
INNODE;\
ACCEPT_ARRAY( Name1 );\
OUTNODE;\
}

#define DEFINE_VISIT_1(Type, Name1) void BaseVisitor::visit( Type* n, bool last ){\
INNODE;\
ACCEPT( Name1, true);\
OUTNODE;\
}

#define DEFINE_VISIT_2(Type, Name1, Name2) void BaseVisitor::visit( Type* n, bool last ){\
INNODE;\
ACCEPT( Name1, false);\
ACCEPT( Name2, true);\
OUTNODE;\
}

DEFINE_DEFAULT_VISIT( ASTNode )

//Expr
DEFINE_DEFAULT_VISIT( Expr )
DEFINE_VISIT_1( UnOp, GetExpr() )
DEFINE_VISIT_2( BinOp, GetLeft(), GetRight() )
DEFINE_VISIT_2(FuncCallExpr, GetCallee(), GetArgs())

DEFINE_DEFAULT_VISIT(Ident)
DEFINE_DEFAULT_VISIT(IntLit)
DEFINE_DEFAULT_VISIT(FloatLit)
DEFINE_DEFAULT_VISIT(BoolLit)
DEFINE_DEFAULT_VISIT(StringLit)


//Stmt

DEFINE_DEFAULT_VISIT(Stmt)
DEFINE_DEFAULT_VISIT(StmtBreak)

DEFINE_VISIT_2( StmtAssign, GetName(), GetExpr() )
DEFINE_VISIT_2( StmtWhile, GetExpr(), GetBody() )
DEFINE_VISIT_2( StmtIfThen, GetExpr(), GetThen() )
DEFINE_VISIT_1( StmtReturn, GetExpr() )
DEFINE_VISIT_ARRAY(ArgList, GetChildren())

void BaseVisitor::visit( StmtIfThenElse* n, bool last ){
	INNODE;
	ACCEPT( GetExpr(), false );
	ACCEPT( GetThen(), false );
	ACCEPT( GetElse(), true );
	OUTNODE;
}

DEFINE_VISIT_ARRAY( StmtBlock, GetChildren() )
DEFINE_VISIT_2( StmtFuncCall, GetName(), GetArgList() )

void BaseVisitor::visit(StmtVarDecl* n, bool last){
	INNODE;
	ACCEPT(GetType(), false);
	ACCEPT(GetName(), false);
	ACCEPT(GetExpr(), true);
	OUTNODE;
}

// GlobalStmt
DEFINE_DEFAULT_VISIT(GlobalStmt);
DEFINE_VISIT_2(GlobVarDef, GetType(), GetName())

void BaseVisitor::visit(FuncDef* n, bool last){
	INNODE;
	ACCEPT(GetRetType(), false);
	ACCEPT(GetName(), false);
	ACCEPT(GetParamList(), false);
	ACCEPT(GetStmtBlock(), true);
	OUTNODE;
}


//Others

DEFINE_VISIT_ARRAY( StartBlock, GetChildren() )
DEFINE_VISIT_ARRAY( IdentList, GetChildren() )

DEFINE_VISIT_2( Param, GetType(), GetName() )
DEFINE_VISIT_ARRAY( ParamList, GetChildren() )
