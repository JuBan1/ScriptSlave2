#include "ExprParser.h"
#include <stdexcept>
#include <map>
#include <iostream>
#include "Token.h"
#include "ASTNode.h"
#include "Util.h"



ExprPtr ExprParser::default_nud( Token self ){
	std::string str = "default_nud: Unexpected end of expression: ";
	str += self.GetTokenValue();

	throw std::runtime_error( str.c_str() );
}
ExprPtr ExprParser::default_led( ExprPtr left ){
	throw std::runtime_error( "default_led: Impossible exception thrown." );
}

//TODO: add the tokens like in ident_nud. change constructors?
ExprPtr ExprParser::integer_nud( Token self ){
	auto ptr = std::make_unique<IntLit>(strtol(self.GetTokenValue().c_str(), nullptr, 10));
	ptr->SetToken(self);
	return std::move(ptr);
}
ExprPtr ExprParser::float_nud(Token self){
	auto ptr = std::make_unique<FloatLit>(strtof(self.GetTokenValue().c_str(), nullptr));
	ptr->SetToken(self);
	return std::move(ptr);
}
ExprPtr ExprParser::boolean_nud( Token self ){
	auto ptr = std::make_unique<BoolLit>((self.GetTokenValue() == "true") ? true : false);
	ptr->SetToken(self);
	return std::move(ptr);
}
ExprPtr ExprParser::string_nud( Token self ){
	auto ptr = std::make_unique<StringLit>(self.GetTokenValue());
	ptr->SetToken(self);
	return std::move(ptr);
}

ExprPtr ExprParser::ident_nud( Token self ){
	auto ptr = std::make_unique<Ident>(self.GetTokenValue());
	ptr->SetToken(self);
	return std::move(ptr);
}

ExprPtr ExprParser::lthan_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::LThan, std::move( left ), ParseExpression( 5 ));
}
ExprPtr ExprParser::lthaneq_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::LThanEq, std::move( left ), ParseExpression( 5 ));
}
ExprPtr ExprParser::equal_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::Equal, std::move( left ), ParseExpression( 5 ));
}
ExprPtr ExprParser::unequal_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::Unequal, std::move( left ), ParseExpression( 5 ));
}
ExprPtr ExprParser::gthan_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::GThan, std::move( left ), ParseExpression( 5 ));
}
ExprPtr ExprParser::gthaneq_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::GThanEq, std::move( left ), ParseExpression( 5 ));
}

// Boolean operators
ExprPtr ExprParser::not_nud( Token self ){
	return std::make_unique<UnOp>(UnOp::Not, ParseExpression( 100 ));
}
ExprPtr ExprParser::and_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::And, std::move( left ), ParseExpression( 20 ));
}
ExprPtr ExprParser::or_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::Or, std::move( left ), ParseExpression( 10 ));
}
ExprPtr ExprParser::xor_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::Xor, std::move( left ), ParseExpression( 10 ));
}

//Arithmetic operators
ExprPtr ExprParser::add_nud( Token self ){
	return ParseExpression( 100 );
}

ExprPtr ExprParser::add_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::Add, std::move( left ), ParseExpression( 10 ));
}
ExprPtr ExprParser::sub_nud( Token self ){
	return std::make_unique<UnOp>(UnOp::Neg, ParseExpression( 100 ));
}
ExprPtr ExprParser::sub_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::Sub, std::move( left ), ParseExpression( 10 ));
}
ExprPtr ExprParser::mul_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::Mul, std::move( left ), ParseExpression( 20 ));
}
ExprPtr ExprParser::div_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::Div, std::move( left ), ParseExpression( 20 ));
}
ExprPtr ExprParser::mod_led( ExprPtr left ){
	return std::make_unique<BinOp>(BinOp::Mod, std::move( left ), ParseExpression( 20 ));
}

//Parenthesis 
ExprPtr ExprParser::lparen_nud( Token self ){
	ExprPtr n = ParseExpression();

	if (!AdvanceToken(TokenType::RParen))
		throw std::runtime_error( "lparen_nud: Expected right parenthesis of expression." );

	return n;
}

ExprPtr ExprParser::lparen_led( ExprPtr left ){

	if( left->GetNodeType() != NodeType::Ident )
		throw std::runtime_error("lparen_led: Expected identifier for function call on left side.");

	std::unique_ptr<ArgList> list = std::make_unique<ArgList>( );
	std::unique_ptr<FuncCallExpr> n = std::make_unique<FuncCallExpr>((std::unique_ptr<Ident>&&)std::move(left), std::move(list));
	

	if (AdvanceToken(TokenType::RParen))
		return std::move( n );

	while( true ){
		((ArgList*)n->GetArgs())->Push() = ParseExpression();

		if (!AdvanceToken(TokenType::Comma))
			break;
	}
	
	if (!AdvanceToken(TokenType::RParen))
		throw std::runtime_error("lparen_led: Expected right parenthesis of expression.");

	return std::move( n );
}

ExprPtr ExprParser::lbracket_led( ExprPtr left ){
	std::unique_ptr<BinOp> n = std::make_unique<BinOp>( BinOp::Subscript, std::move( left ), ParseExpression() );

	//if( n->GetLeftChild()->GetNodeType() != NodeType::NameNode )
	//	throw std::runtime_error( "lbracket_led: Expected name for subscript on left side." );

	if (!AdvanceToken(TokenType::RBracket))
		throw std::runtime_error("lbracket_led: Expected right bracket of subscript.");

	return std::move( n );
}

ExprPtr ExprParser::period_led( ExprPtr left ){
	if (m_tokens.GetCurrentToken().type != TokenType::Ident){
		throw std::runtime_error("period_led: Expected identifier right of period.");
	}

	return std::make_unique<BinOp>( BinOp::MemberAccess, std::move( left ), ParseExpression( 100 - 1 ) );
}

ExprParser::TokenRule::TokenRule( int lbp, nudPtr nud, ledPtr led )
	: mLeftBindingPower( lbp ), mNud( nud ), mLed( led )
{}

ExprPtr ExprParser::ParseExpression( int rbp ){ //right binding power
	auto t = m_tokens.GetCurrentToken();
	m_tokens.GetNextToken( "Expression" );
	ExprPtr left = nud( t ); //throws exception if left == nullptr

	while( rbp < lbp( m_tokens.GetCurrentToken() ) ){
		t = m_tokens.GetCurrentToken();
		m_tokens.GetNextToken( "Expression" );
		left = led( t, std::move( left ) );
	}

	return left;
}

ExprPtr ExprParser::nud( Token self ){
	auto it = m_rules.find( self.type );

	if( it != m_rules.end() )
		return (this->*it->second.mNud)(self);
	else
		throw std::runtime_error( "nud: Unrecognized token." );
}

ExprPtr ExprParser::led( Token self, ExprPtr left ){
	auto it = m_rules.find( self.type );

	if( it != m_rules.end() )
		return (this->*it->second.mLed)(std::move( left ));
	else
		throw std::runtime_error( "led: Unrecognized token." );
	
}

int ExprParser::lbp( Token self ){
	auto it = m_rules.find( self.type );

	if( it != m_rules.end() )
		return it->second.mLeftBindingPower;
	else
		return -1;
	//throw std::exception( "lbp: Unrecognized token." );
}


ExprParser::ExprParser( TokenStack& tokens )
	: m_tokens( tokens )
{
#define ADD_RULE(type, lbp, nud, led) m_rules.insert( std::make_pair( TokenType::type, TokenRule( lbp, nud, led ) ) );
	ADD_RULE( IntLit, 0, &ExprParser::integer_nud, &ExprParser::default_led );
	ADD_RULE( FloatLit, 0, &ExprParser::float_nud, &ExprParser::default_led );
	ADD_RULE( BoolLit, 0, &ExprParser::boolean_nud, &ExprParser::default_led );
	ADD_RULE( StringLit, 0, &ExprParser::string_nud, &ExprParser::default_led );
	ADD_RULE(Ident, 0, &ExprParser::ident_nud, &ExprParser::default_led);

	ADD_RULE( LThan, 5, &ExprParser::default_nud, &ExprParser::lthan_led );
	ADD_RULE( LThanEq, 5, &ExprParser::default_nud, &ExprParser::lthaneq_led );
	ADD_RULE( Equal, 5, &ExprParser::default_nud, &ExprParser::equal_led );
	ADD_RULE( Unequal, 5, &ExprParser::default_nud, &ExprParser::unequal_led );
	ADD_RULE( GThan, 5, &ExprParser::default_nud, &ExprParser::gthan_led );
	ADD_RULE( GThanEq, 5, &ExprParser::default_nud, &ExprParser::gthaneq_led );

	ADD_RULE( And, 20, &ExprParser::default_nud, &ExprParser::and_led );
	ADD_RULE( Or, 10, &ExprParser::default_nud, &ExprParser::or_led );
	ADD_RULE( Not, 0, &ExprParser::not_nud, &ExprParser::default_led );

	ADD_RULE( Plus, 10, &ExprParser::add_nud, &ExprParser::add_led );
	ADD_RULE( Minus, 10, &ExprParser::sub_nud, &ExprParser::sub_led );
	ADD_RULE( Mul, 20, &ExprParser::default_nud, &ExprParser::mul_led );
	ADD_RULE( Div, 20, &ExprParser::default_nud, &ExprParser::div_led );

	ADD_RULE( LParen, 100, &ExprParser::lparen_nud, &ExprParser::lparen_led );
	//ADD_RULE( RParen, 0, &ExprParser::default_nud, &ExprParser::default_led );
	ADD_RULE( LBracket, 100, &ExprParser::default_nud, &ExprParser::lbracket_led );
	//ADD_RULE( RBracket, 0, &ExprParser::default_nud, &ExprParser::default_led );
	ADD_RULE( Period, 100, &ExprParser::default_nud, &ExprParser::period_led );
	ADD_RULE( Comma, 0, &ExprParser::default_nud, &ExprParser::default_led );
	ADD_RULE( Eof, 0, &ExprParser::default_nud, &ExprParser::default_led );
#undef ADD_RULE
}

bool ExprParser::AdvanceToken( TokenType type ){
	if( m_tokens.GetCurrentToken().type == type ){
		m_tokens.GetNextToken( "Expression" );
		return true;
	}
	return false;
}

bool ExprParser::MatchExpression( ExprPtr& in ){
	m_tokens.PushIndex();

	try{
		in = ParseExpression();
		m_tokens.DiscardIndex();
		return true;
	}
	catch( std::runtime_error ex ){
		std::cout << "Error: " << ex.what() << "\n";
		m_tokens.PopIndex();
		return false;
	}
}