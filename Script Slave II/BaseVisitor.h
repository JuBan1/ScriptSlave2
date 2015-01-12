#ifndef TREEVISITOR_H
#define TREEVISITOR_H

#define ACCEPT(Child, Last) if(n->Child != nullptr) n->Child->accept(this, Last); 
#define ACCEPT_ARRAY(FuncName) const auto& children = n->FuncName; \
const auto end = children.end(); \
for( auto it = children.begin(); it != children.end(); ++it ){ \
	(*it)->accept( this, it == end - 1 ); \
}
#define INNODE if( !inNode(n,last) ) return; //{ outNode(n,last); return; }
#define OUTNODE outNode(n,last);

class ASTNode;
//Expr
class Expr;
class UnOp;
class BinOp;
class Ident;
class IntLit;
class FloatLit;
class BoolLit;
class StringLit;
class ArgList;
class FuncCallExpr;
//Stmt
class Stmt;
class StmtBreak;
class StmtAssign;
class StmtWhile;
class StmtIfThen;
class StmtReturn;
class StmtIfThenElse;
class StmtFuncCall;
class StmtVarDecl;
class StmtBlock;
//Class
class ClassBody;
class ClassMember;
class ClassVar;
//GlobalStmt
class GlobalStmt;
class GlobVarDef;
class FuncDef;
class ClassDef;
//Others
class StartBlock;
class IdentList;
class Param;
class ParamList;

#define ADD_VISITEE(Type, Parent) virtual void visit( Type* n, bool last); \
virtual bool inNode( Type* n, bool last){ return inNode((Parent*) n, last); } \
virtual void outNode( Type* n, bool last ){ outNode( (Parent*) n, last ); }

class BaseVisitor{
public:
	
	//ASTNode
	virtual void visit( ASTNode* n, bool last );
	virtual bool inNode( ASTNode* n, bool last ){ return true; }
	virtual void outNode( ASTNode* n, bool last ){ }

	//Expr
	ADD_VISITEE(Expr, ASTNode);
	ADD_VISITEE(UnOp, Expr);
	ADD_VISITEE(BinOp, Expr);
	ADD_VISITEE(Ident, Expr);
	ADD_VISITEE(IntLit, Expr);
	ADD_VISITEE(FloatLit, Expr);
	ADD_VISITEE(BoolLit, Expr);
	ADD_VISITEE(StringLit, Expr);
	ADD_VISITEE(FuncCallExpr, Expr);

	//Stmt
	ADD_VISITEE(Stmt, ASTNode);
	ADD_VISITEE(StmtBreak, Stmt);
	ADD_VISITEE(StmtAssign, Stmt);
	ADD_VISITEE(StmtWhile, Stmt);
	ADD_VISITEE(StmtIfThen, Stmt);
	ADD_VISITEE(StmtReturn, Stmt);
	ADD_VISITEE(StmtIfThenElse, Stmt);
	ADD_VISITEE(StmtFuncCall, Stmt);
	ADD_VISITEE(StmtVarDecl, Stmt);
	ADD_VISITEE(StmtBlock, Stmt);

	//Class
	ADD_VISITEE(ClassBody, ASTNode);
	ADD_VISITEE(ClassMember, ASTNode);
	ADD_VISITEE(ClassVar, ClassMember);

	//GlobalStmt
	ADD_VISITEE(GlobalStmt, ASTNode);
	ADD_VISITEE(GlobVarDef, GlobalStmt);
	ADD_VISITEE(FuncDef, GlobalStmt);
	ADD_VISITEE(ClassDef, GlobalStmt);

	//Others
	ADD_VISITEE(ArgList, ASTNode);
	ADD_VISITEE(StartBlock, ASTNode);
	ADD_VISITEE(IdentList, ASTNode);
	ADD_VISITEE(Param, ASTNode);
	ADD_VISITEE(ParamList, ASTNode);

};

#endif