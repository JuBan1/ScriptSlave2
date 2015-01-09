#ifndef EXPRPARSER_H
#define EXPRPARSER_H

#include <map>
#include "Token.h"
#include "TokenStack.h"
#include "ASTNode.h"
#include <memory>

class ExprParser{

public:

	ExprParser( TokenStack& tokens );

	bool MatchExpression(ExprPtr& in);
	bool MatchNamedExpression(ExprPtr& in);

private:

	struct TokenRule{
		typedef ExprPtr( ExprParser::*nudPtr )(Token self);
		typedef ExprPtr( ExprParser::*ledPtr )(Token self, ExprPtr left);

		int mLeftBindingPower;
		nudPtr mNud;
		ledPtr mLed;

		TokenRule( int lbp, nudPtr nud, ledPtr led );
	};

	ExprPtr ParseExpression( int rbp = 0 );
	ExprPtr nud( Token self );
	ExprPtr led( Token self, ExprPtr left );
	int lbp( Token self );

#define REGISTER_NUD(name) ExprPtr name(Token self)
#define REGISTER_LED(name) ExprPtr name(Token self, ExprPtr left)

	REGISTER_NUD(default_nud);
	REGISTER_NUD(integer_nud);
	REGISTER_NUD(float_nud);
	REGISTER_NUD(string_nud);
	REGISTER_NUD(boolean_nud);
	REGISTER_NUD(ident_nud);
	REGISTER_NUD(add_nud);
	REGISTER_NUD(sub_nud);
	REGISTER_NUD(lparen_nud);

	REGISTER_NUD(not_nud);

	REGISTER_LED(default_led);
	REGISTER_LED(lthan_led);
	REGISTER_LED(lthaneq_led);
	REGISTER_LED(equal_led);
	REGISTER_LED(unequal_led);
	REGISTER_LED(gthan_led);
	REGISTER_LED(gthaneq_led);

	REGISTER_LED(lparen_led);
	REGISTER_LED(lbracket_led);
	REGISTER_LED(period_led);

	REGISTER_LED(and_led);
	REGISTER_LED(or_led);
	REGISTER_LED(xor_led);

	REGISTER_LED(add_led);
	REGISTER_LED(sub_led);
	REGISTER_LED(mul_led);
	REGISTER_LED(div_led);
	REGISTER_LED(mod_led);

#undef REGISTER_NUD
#undef REGISTER_LED

private:

	bool AdvanceToken( TokenType type );

	TokenStack& m_tokens;
	std::map<TokenType, TokenRule> m_rules;

};


#endif