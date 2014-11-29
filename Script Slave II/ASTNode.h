#ifndef ASTNODE_H
#define ASTNODE_H

#include <vector>
#include <memory>

#include "Token.h"
#include "BaseVisitor.h"


class TypeInfo;
class Symbol;

/*
	New node types must have:
	- an enum value inside NodeType
	- inNode/outNode/visit overloads in BaseVisitor
	- A subclass of ASTNode or one of it's descendents.
	- A std::unique_ptr typedef called <name of node>Ptr
*/

enum class NodeType{
	ASTNode,
	Type,
	Expr,
	BinOp,
	UnOp,
	Ident,
	IntLit,
	FloatLit,
	BoolLit,
	StringLit,
	FuncDef,
	FuncBody,
	FuncCallExpr,

	StmtBreak,
	StmtWhile,
	StmtIfThen,
	StmtIfThenElse,
	StmtBlock,
	StmtAssign,
	StmtReturn,
	StmtVarDecl,
	StmtFuncCall,

	ArgList,
	Arg,
	ParamList,
	Param,
	StmtEmpty,
	StartBlock,
	IdentList,
	Name
};

#define CASE_RETURN(x) case NodeType::x: return #x;
static const char* NodeTypeAsString( NodeType op ){
	switch( op ){
		CASE_RETURN(ASTNode);
		CASE_RETURN(Type);
		CASE_RETURN(Expr);
		CASE_RETURN(BinOp);
		CASE_RETURN(UnOp);
		CASE_RETURN(Ident);
		CASE_RETURN(IntLit);
		CASE_RETURN(BoolLit);
		CASE_RETURN(StringLit);
		CASE_RETURN(FloatLit);
		CASE_RETURN(FuncDef);
		CASE_RETURN(FuncBody);
		CASE_RETURN(FuncCallExpr);
		CASE_RETURN(StmtBreak);
		CASE_RETURN(StmtWhile);
		CASE_RETURN(StmtIfThen);
		CASE_RETURN(StmtIfThenElse);
		CASE_RETURN(StmtAssign);
		CASE_RETURN(StmtFuncCall);
		CASE_RETURN(StmtBlock);
		CASE_RETURN(StmtEmpty);
		CASE_RETURN(StmtReturn);
		CASE_RETURN(StmtVarDecl);
		CASE_RETURN(StartBlock);
		CASE_RETURN(IdentList);
		CASE_RETURN(ArgList);
		CASE_RETURN(Param);
		CASE_RETURN(ParamList);
		CASE_RETURN(Name);
	default: return "Error";
	}
}
#undef CASE_RETURN

#define DEFAULT_ACCEPT virtual void accept( BaseVisitor* v, bool last){ v->visit(this, last); }
#define DEFAULT_CONSTRUCT(Name, Parent) Name() : Parent( NodeType::Name ){}
#define ONE_PARAM_CONSTRUCT(Name, Parent, AType, AName) Name(AType AName) : Parent( NodeType::Name ), m_ ## AName(std::move(AName)) {}
#define TWO_PARAM_CONSTRUCT(Name, Parent, AType, AName, BType, BName) Name(AType AName, BType BName) : Parent( NodeType::Name ), m_ ## AName(std::move(AName)), m_ ## BName(std::move(BName)) {}
#define THREE_PARAM_CONSTRUCT(Name, Parent, AType, AName, BType, BName, CType, CName) Name(AType AName, BType BName, CType CName) \
	: Parent( NodeType::Name ), m_ ## AName(std::move(AName)), m_ ## BName(std::move(BName)), m_ ## CName(std::move(CName)) {}
#define DEFAULT_TYPEDEF(Name) typedef std::unique_ptr<Name> Name ## Ptr;
#define FOUR_PARAM_CONSTRUCT(Name, Parent, AType, AName, BType, BName, CType, CName, DType, DName) Name(AType AName, BType BName, CType CName) \
	: Parent( NodeType::Name ), m_ ## AName( std::move( AName ) ), m_ ## BName( std::move( BName ) ), m_ ## CName( std::move( CName ) ), m_ ## DName(DName) {}
#define DEFAULT_TYPEDEF(Name) typedef std::unique_ptr<Name> Name ## Ptr;
#define GET_CHILD(Type, Name) private: Type ## Ptr m_ ## Name; public: Type* Get ## Name () const { return m_ ## Name.get(); } Type ## Ptr& Get ## Name ## Ref(){ return m_ ## Name; }
#define GET_MEMBER(Type, Name) private: Type m_ ## Name; public: Type Get ## Name (){ return m_ ## Name; }
#define GETSET_MEMBER(Type, Name) private: Type m_ ## Name; public: Type Get ## Name () const { return m_ ## Name; } void Set ## Name( Type t){ m_ ## Name = t; }

#define GET_CHILD_ARRAY(Type,Name) private:\
std::vector<Type ## Ptr> m_ ## Name;\
public:\
	Type ## Ptr& Push(){ m_ ## Name.push_back( nullptr ); return m_ ## Name[m_ ## Name.size() - 1]; }\
	void Pop(){ m_ ## Name.pop_back(); }\
	void Add( Type ## Ptr n ){ m_ ## Name.push_back( std::move( n ) ); }\
	std::vector<Type ## Ptr>& GetChildren(){ return m_ ## Name; }\
	bool IsEmpty(){ return m_ ## Name.empty(); }\
	size_t NumChildren(){ return m_ ## Name.size(); }\
	void PopChildren( size_t n ){ while( n > 0 ){ --n; m_ ## Name.pop_back(); } }




class ASTNode{
	//abstract, do not instantiate
public:
	DEFAULT_ACCEPT;
	const Token& GetToken() const {
		return m_token;
	}
	void SetToken( Token t ){
		m_token = t;
	}

	virtual std::string GetNodeAsString() { return NodeTypeAsString(m_nodeType); };

	NodeType GetNodeType() const {
		return m_nodeType;
	}

	ASTNode( NodeType type )
		: m_nodeType( type )
	{}

	virtual ~ASTNode(){}

protected:
	NodeType m_nodeType;
	Token m_token;
};
DEFAULT_TYPEDEF( ASTNode );

class Type : public ASTNode{
public:
	DEFAULT_ACCEPT;
	GET_MEMBER(std::string, Name);
	GETSET_MEMBER(TypeInfo const*, TypeInfo);

	virtual std::string GetNodeAsString() { return m_Name; };

	Type(std::string Name) : ASTNode(NodeType::Type), m_Name(Name), m_TypeInfo(nullptr) {}
};
DEFAULT_TYPEDEF(Type);


class Expr : public ASTNode {
public:
	DEFAULT_ACCEPT;

	GETSET_MEMBER(TypeInfo const*, TypeInfo);

	Expr( NodeType type ) : ASTNode( type ), m_TypeInfo(nullptr){}
};
DEFAULT_TYPEDEF( Expr );

class BinOp : public Expr{
public:
	enum Types{
		//Arithmetic
		Add,
		Sub,
		Div,
		Mul,
		Mod,
		//Order
		LThan,
		GThan,
		LThanEq,
		GThanEq,
		//Equality
		Equal,
		Unequal,
		//Logical
		And,
		Or,
		Xor,
		//access
		Subscript,
		MemberAccess,
		FunctionCall,
	};

#define CASE_RETURN(x) case x: return #x;
	static const char* GetTypeAsString( Types t ){
		switch( t ){
			CASE_RETURN( Add );
			CASE_RETURN( Sub );
			CASE_RETURN( Div );
			CASE_RETURN( Mul );
			CASE_RETURN( Mod );
			CASE_RETURN( LThan );
			CASE_RETURN( GThan );
			CASE_RETURN( LThanEq );
			CASE_RETURN( GThanEq );
			CASE_RETURN( Equal );
			CASE_RETURN( Unequal );
			CASE_RETURN( And );
			CASE_RETURN( Or );
			CASE_RETURN( Xor );
			CASE_RETURN( Subscript );
			CASE_RETURN( MemberAccess );
			CASE_RETURN( FunctionCall );
			default: return "Error";
		}
	}
#undef CASE_RETURN

	virtual std::string GetNodeAsString() { return GetTypeAsString( m_Type ); };

	DEFAULT_ACCEPT;

	GET_MEMBER( Types, Type );
	GET_CHILD( Expr, Left );
	GET_CHILD( Expr, Right );
	
	BinOp( Types type, ExprPtr left, ExprPtr right )
		: Expr( NodeType::BinOp ), m_Type( type ), m_Left( std::move( left ) ), m_Right( std::move( right ) ) {}
};
DEFAULT_TYPEDEF( BinOp );

class UnOp : public Expr{
public:
	enum Types{
		Neg, //Arithmetic
		Not //Logical
	};

#define CASE_RETURN(x) case x: return #x;
	static const char* GetTypeAsString( Types t ){
		switch( t ){
			CASE_RETURN( Neg );
			CASE_RETURN( Not );
			default: return "Error";
		}
	}
#undef CASE_RETURN

	virtual std::string GetNodeAsString() { return GetTypeAsString( m_Type ); };

	DEFAULT_ACCEPT;
	GET_MEMBER( Types, Type );
	GET_CHILD( Expr, Expr );

	UnOp( Types type, ExprPtr expr ) : Expr( NodeType::UnOp ), m_Type( type ), m_Expr( std::move( expr ) ) {}
};
DEFAULT_TYPEDEF( UnOp );

class Ident : public Expr{
public:
	DEFAULT_ACCEPT;
	GET_MEMBER( std::string, Name );
	GETSET_MEMBER(Symbol const*, Symbol);

	virtual std::string GetNodeAsString() { return m_Name; }

	Ident( std::string Name ) : Expr( NodeType::Ident ), m_Name( Name ), m_Symbol(nullptr) {}
};
DEFAULT_TYPEDEF( Ident );

class IdentList : public ASTNode{
public:
	DEFAULT_ACCEPT;

	GET_CHILD_ARRAY(Ident, Idents)

	DEFAULT_CONSTRUCT( IdentList, ASTNode );
};
DEFAULT_TYPEDEF( IdentList );

//TODO: ArgList should inherit from ASTNode
class ArgList : public ASTNode {
public:
	DEFAULT_ACCEPT;

	GET_CHILD_ARRAY(Expr, Exprs)

		DEFAULT_CONSTRUCT(ArgList, ASTNode);
};
DEFAULT_TYPEDEF(ArgList);

class FuncCallExpr : public Expr {

	DEFAULT_ACCEPT;
	GET_CHILD(Ident, Callee);
	GET_CHILD(ArgList, Args);

	FuncCallExpr(IdentPtr callee, ArgListPtr args) : Expr(NodeType::FuncCallExpr), m_Callee(std::move(callee)), m_Args(std::move(args)) {}

};
DEFAULT_TYPEDEF(FuncCallExpr);

class IntLit : public Expr{
public:
	DEFAULT_ACCEPT;
	GET_MEMBER(int, Value);

	virtual std::string GetNodeAsString() { return std::to_string(m_Value); };

	IntLit(int Value) : Expr(NodeType::IntLit), m_Value(Value) {}
};
DEFAULT_TYPEDEF(IntLit);

class FloatLit : public Expr{
public:
	DEFAULT_ACCEPT;
	GET_MEMBER(float, Value);

	virtual std::string GetNodeAsString() { return std::to_string(m_Value); };

	FloatLit(float Value) : Expr(NodeType::FloatLit), m_Value(Value) {}
};
DEFAULT_TYPEDEF(FloatLit);

class BoolLit : public Expr{
public:
	DEFAULT_ACCEPT;
	GET_MEMBER( bool, Value );

	virtual std::string GetNodeAsString() { return m_Value? "True" : "False"; };

	BoolLit( bool Value ) : Expr( NodeType::BoolLit ), m_Value( Value ) {}
};
DEFAULT_TYPEDEF( BoolLit );

class StringLit : public Expr{
public:
	DEFAULT_ACCEPT;
	GET_MEMBER( std::string, Value );

	virtual std::string GetNodeAsString() { return m_Value; };

	StringLit( std::string Value ) : Expr( NodeType::StringLit ), m_Value( Value ) {}
};
DEFAULT_TYPEDEF( StringLit );

class Stmt : public ASTNode{
public:
	Stmt( NodeType type ) : ASTNode( type ){}
};
DEFAULT_TYPEDEF( Stmt );

class StmtAssign : public Stmt{
public:
	DEFAULT_ACCEPT;

	GET_CHILD(Ident, Name);
	GET_CHILD(Expr, Expr);
	

	DEFAULT_CONSTRUCT(StmtAssign, Stmt);
};
DEFAULT_TYPEDEF(StmtAssign);

class StmtFuncCall : public Stmt{
public:
	DEFAULT_ACCEPT;

	GET_CHILD(Ident, Name);
	GET_CHILD(ArgList, ArgList);


	DEFAULT_CONSTRUCT(StmtFuncCall, Stmt);
};
DEFAULT_TYPEDEF(StmtFuncCall);

class StmtBreak : public Stmt{
public:
	DEFAULT_ACCEPT;
	DEFAULT_CONSTRUCT( StmtBreak, Stmt );
};
DEFAULT_TYPEDEF( StmtBreak );

class StmtWhile : public Stmt{
public:
	DEFAULT_ACCEPT;
	GET_CHILD( Expr, Expr );
	GET_CHILD( Stmt, Body );

	DEFAULT_CONSTRUCT( StmtWhile, Stmt );
};
DEFAULT_TYPEDEF( StmtWhile );

class StmtIfThen : public Stmt{
public:
	DEFAULT_ACCEPT;
	GET_CHILD( Expr, Expr );
	GET_CHILD( Stmt, Then );
	DEFAULT_CONSTRUCT( StmtIfThen, Stmt );
};
DEFAULT_TYPEDEF( StmtIfThen );

class StmtReturn : public Stmt{
public:
	DEFAULT_ACCEPT;
	GET_CHILD( Expr, Expr );
	DEFAULT_CONSTRUCT( StmtReturn, Stmt );
};
DEFAULT_TYPEDEF( StmtReturn );

class StmtIfThenElse : public Stmt{
public:
	DEFAULT_ACCEPT;
	GET_CHILD( Expr, Expr );
	GET_CHILD( Stmt, Then );
	GET_CHILD( Stmt, Else );

	DEFAULT_CONSTRUCT( StmtIfThenElse, Stmt );
};
DEFAULT_TYPEDEF( StmtIfThenElse );


class Param : public ASTNode{
public:
	DEFAULT_ACCEPT;
	GET_CHILD( Type, Type );
	GET_CHILD( Ident, Name );

	DEFAULT_CONSTRUCT( Param, ASTNode );
};
DEFAULT_TYPEDEF( Param );

class ParamList : public ASTNode{
public:
	DEFAULT_ACCEPT;

	GET_CHILD_ARRAY(Param, Param)
	DEFAULT_CONSTRUCT( ParamList, ASTNode );
};
DEFAULT_TYPEDEF( ParamList );

class StmtVarDecl : public Stmt{
public:
	DEFAULT_ACCEPT;
	GET_CHILD( Type, Type );
	GET_CHILD( Ident, Name );
	GET_CHILD(Expr, Expr);
	DEFAULT_CONSTRUCT( StmtVarDecl, Stmt );
};
DEFAULT_TYPEDEF( StmtVarDecl );


class StmtBlock : public Stmt{
public:
	DEFAULT_ACCEPT;

	GET_CHILD_ARRAY(Stmt, Stmts)

	DEFAULT_CONSTRUCT( StmtBlock, Stmt );
};
DEFAULT_TYPEDEF( StmtBlock );

class FuncDef : public ASTNode{
public:
	DEFAULT_ACCEPT;

	GET_CHILD(Type, RetType);
	GET_CHILD(Ident, Name);
	GET_CHILD(ParamList, ParamList);
	GET_CHILD(StmtBlock, StmtBlock);

	DEFAULT_CONSTRUCT(FuncDef, ASTNode);
};
DEFAULT_TYPEDEF( FuncDef );

class StartBlock : public ASTNode{
public:
	DEFAULT_ACCEPT;

	GET_CHILD_ARRAY(FuncDef, Funcs)

	DEFAULT_CONSTRUCT( StartBlock, ASTNode );
};
DEFAULT_TYPEDEF( StartBlock );


#endif