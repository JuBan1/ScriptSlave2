#include "CodeGen.h"

#include <iostream>
#include <algorithm>
#include <fstream>
#include <stack>
#include "BaseVisitor.h"
/*
class CodeWalker : public BaseVisitor{
public:
	//EXPRESSIONS
	virtual void outNode( UnOp* n, bool last ){
		std::string op;

		switch( n->GetType() ){
		case UnOp::Not:
			AddLine( "  iconst_1" );
			AddLine( "  ixor" );
			IncStackDepth( 1 );	//Inc stack by one to account for the iconst_1
			IncStackDepth( -1 ); //dec because of ixor
			break;
		case UnOp::Neg:
			AddLine( "  ineg" );
			break;
		}
	}

	//private
	std::string makeBranch( std::string op ){
		const auto label = "L" + std::to_string( m_labelCounter++ );
		const auto lc1 = label + "_true";
		const auto lc2 = label + "_end";
		std::string out =
			"  " + op + " " + lc1 + "\n"
			"  iconst_0\n"
			"  goto " + lc2 + "\n"
			+ lc1 + ":\n"
			"  iconst_1\n"
			+ lc2 + ":";


		//IncStackDepth( -1 ); //push iconst_0 or iconst_1, but pop two operands because of 'op'. Will be added in outNode.

		return out;
	}

	virtual void outNode( BinOp* n, bool last ){
		std::string op;

		switch( n->GetType() ){
		case BinOp::Equal:		op = makeBranch( "if_icmpeq" ); break;
		case BinOp::Unequal:	op = makeBranch( "if_icmpne" ); break;
		case BinOp::LThan:		op = makeBranch( "if_icmplt" ); break;
		case BinOp::LThanEq:	op = makeBranch( "if_icmple" ); break;
		case BinOp::GThan:		op = makeBranch( "if_icmpgt" ); break;
		case BinOp::GThanEq:	op = makeBranch( "if_icmpge" ); break;
		case BinOp::And:		op = "  iand"; break;
		case BinOp::Or:			op = "  ior"; break;
		case BinOp::Xor:		op = "  ixor"; break;
		case BinOp::Add:		op = "  iadd"; break;
		case BinOp::Sub:		op = "  isub"; break;
		case BinOp::Mul:		op = "  imul"; break;
		case BinOp::Div:		op = "  idiv"; break;
		case BinOp::Mod:		op = "  irem"; break;
		}
		AddLine( op );

		IncStackDepth( -1 ); //2 popped, result pushed
	}


	virtual bool inNode( Ident* n, bool last ){
		//auto var = m_table.GetVariable( n->GetName() );

		//AddLine( "  iload " + std::to_string( var.Get().m_varIndex ), "load " + n->GetName() );

		IncStackDepth( 1 ); //value pushed
		return true;
	}

	virtual bool inNode( IntLit* n, bool last ){
		int i = n->GetValue();
		std::string instr;

		//use best instruction depending on value of i: iconst_x < bipush < sipush < ldc
		if( i == -1 )
			instr = "  iconst_m1";
		else if( i >= 0 && i <= 5 )
			instr = "  iconst_" + std::to_string( i );
		else if( i >= -128 && i <= 127 )
			instr = "  bipush " + std::to_string( i );
		else if( i >= -32768 && i <= 32767 )
			instr = "  sipush " + std::to_string( i );
		else
			instr = "  ldc " + std::to_string( i );

		AddLine( instr, "push " + std::to_string( i ) );
		IncStackDepth( 1 ); //value pushed

		return true;
	}

	virtual bool inNode( BoolLit* n, bool last ){
		std::string lit = (n->GetValue() == true) ? "true" : "false";
		AddLine( "  iconst_" + std::to_string( n->GetValue() ), "push " + lit );
		IncStackDepth( 1 ); //value pushed
		return true;
	}




	virtual bool inNode( StmtBreak* n, bool last ){
		AddLine( "  goto " + m_labelStack.top(), "break" );
		return true;
	}

	virtual bool inNode( StmtLet* n, bool last ){
		n->GetExprRef()->accept( this, true );
		return false;
	}
	virtual void outNode( StmtLet* n, bool last ){
		//const auto& name = n->GetVar()->GetName();
		//auto var = m_table.GetVariable( name );
		//AddLine( "  istore " + std::to_string( var.Get().m_varIndex ), "store in " + name );
		//IncStackDepth( -1 );//value popped
	}

	void visit( StmtWhile* n, bool last ){
		const auto label = "L" + std::to_string( m_labelCounter++ );
		const auto lstart = label + "_start";
		const auto lend = label + "_end";

		AddLine( lstart + ":", "start of while" );
		ACCEPT( GetExpr(), false );

		AddLine( "  ifeq " + lend, "while-loop condition jump" );
		IncStackDepth( -1 );

		m_labelStack.push( lend );
		ACCEPT( GetBody(), true );
		m_labelStack.pop();

		AddLine( "  goto " + lstart );
		AddLine( lend + ":", "end of while" );
	}

	void visit( StmtIfThen* n, bool last ){
		ACCEPT( GetExpr(), false );

		auto label = "L" + std::to_string( m_labelCounter++ ) + "_skip";
		AddLine( "  ifeq " + label, "start of if-then" );
		IncStackDepth( -1 );

		ACCEPT( GetThen(), true );

		AddLine( label + ":", "end of if-then" );
	}

	void visit( StmtIfThenElse* n, bool last ){
		ACCEPT( GetExpr(), false );


		const auto label = "L" + std::to_string( m_labelCounter++ );
		const auto lelse = label + "_else";
		const auto lend = label + "_end";
		AddLine( "  ifeq " + lelse, "start of if-then-else" );
		IncStackDepth( -1 );

		ACCEPT( GetThen(), true );

		AddLine( "  goto " + lend );
		AddLine( lelse + ":" );

		ACCEPT( GetElse(), true );

		AddLine( lend + ":", "end of if-then-else" );
	}

	const std::vector<std::string>& GetCode(){
		return m_code;
	}

	int GetMaximumStackDepth(){
		return m_maxStackDepth;
	}

	CodeWalker( const SymbolScope& symbolTable )
		: m_table( symbolTable ), m_labelCounter( 0 ), m_currStackDepth( 0 ), m_maxStackDepth( 0 )
	{
		//Shadow initialization, there's only actual need to initialize all variables which are in the in-set of the start node during liveness flow analysis.
		AddLine( "  ; variable initialization" );
		const auto& vars = symbolTable.GetAllVariables( );
		for( auto&& v : vars ){
			AddLine( "  iconst_0" );
			//AddLine( "  istore " + std::to_string( v.second.m_varIndex ), "init " + v.first );
		}
		AddLine( "  ; code" );
	}

private:

	void AddLine( std::string line, std::string comment = "" ){
		const int tabLength = 4;
		int commentPadding;
		const int length = line.length();

		if( comment == "" ){
			m_code.push_back( line );
			return;
		}

		commentPadding = 8 * tabLength;

		int tabNum = (commentPadding - length) / tabLength;
		while( tabNum < 0 ) tabNum += 2;

		if( line.length() % tabLength == 0 ) --tabNum;
		if( tabNum <= 0 ) tabNum = 1;

		m_code.push_back( line + std::string( tabNum, '\t' ) + "; " + comment );
	}

	void IncStackDepth( int amount ){
		m_currStackDepth += amount;

#ifdef DEBUG_BUILD
		if( m_currStackDepth < 0 )
			__debugbreak();
#endif

		m_maxStackDepth = std::max( m_maxStackDepth, m_currStackDepth );
	}

	const SymbolScope& m_table;
	std::vector<std::string> m_code;
	std::stack<std::string> m_labelStack;	//For nested while-statements, needed for Break;
	int m_labelCounter;
	int m_currStackDepth;	//Try to determine the estimated maximum stack depth.
	int m_maxStackDepth;
};


bool CodeGen::Generate( StartBlockPtr& node, const SymbolScope& table ){
	if( !m_code.empty() )
		m_code.clear();

	//Add boilerplate code
	m_code.push_back( ".bytecode 50.0" );				//m_code[0]
	m_code.push_back( ".class public <ERROR>" );		//m_code[1]
	m_code.push_back( ".super java/lang/Object\n\n"
		".method public <init>()V\n"
		"  .limit stack 1\n"
		"  .limit locals 1\n"
		"  aload_0\n"
		"  invokespecial java/lang/Object/<init>()V\n"
		"  return\n"
		".end method\n\n"

		".method public static main([Ljava/lang/String;)V"
		);												//m_code[2]
	m_code.push_back( "  .limit stack <ERROR>" );		//m_code[3]
	m_code.push_back( "  .limit locals <ERROR>" );		//m_code[4]

	//CodeWalker cw( table );
	//node->GetStmtBlock()->accept( &cw, false ); //Only generate code for the Statement Block.

	m_code.push_back(
		"  return\n"
		".end method"
		);

	return true;
}

bool CodeGen::WriteToFile( std::string fileName ){
	std::ofstream outfile( fileName );

	for( auto && s : m_code ){
		outfile << s << "\n";
	}

	return true;
}*/