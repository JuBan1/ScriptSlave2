#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include "FilePosition.h"

enum class TokenType{
//Keywords
	Var,
	If,
	Then,
	Else,
	While,
	Do,
	Let,
	Break,
	Return,
	Class,
	Method,
	Function,
	Constructor,
	Static,
	Field,
	Null,
	This,
//Mutables
	BoolLit,
	IntLit,
	FloatLit,
	StringLit,
	Ident,
//Control characters
	LParen,
	RParen,
	LBracket,
	RBracket,
	LBrace,
	RBrace,
	Not,		//~
	And,
	Or,
	Comma,
	Semicolon,
	Period,
	Equal,
	Assign,
	Unequal,
	LThan,
	LThanEq,
	GThan,
	GThanEq,
	Plus,
	Minus,
	Mul,
	Div,
//Eof
	Eof
};

class Token{
public:
	TokenType type;

	FilePosition filePosition;

	//std::string::const_iterator start, end;
	std::string GetTokenValue() const {
		if (type == TokenType::StringLit)
			return "\"" + m_string + "\"";
		else
			return m_string;
	}

	void SetTokenValue(const std::string& string){
		m_string = string;
	}

	std::string GetTypeAsString() const{
		switch( type ){

#define CASE_RETURN(x) case TokenType::x: return #x;		
			CASE_RETURN(Var);
			CASE_RETURN(If);
			CASE_RETURN(Then);
			CASE_RETURN(Else);
			CASE_RETURN(While);
			CASE_RETURN(Do);
			CASE_RETURN(BoolLit);
			CASE_RETURN(IntLit);
			CASE_RETURN(StringLit);
			CASE_RETURN(FloatLit);
			CASE_RETURN(Ident);
			CASE_RETURN(LParen);
			CASE_RETURN(RParen);
			CASE_RETURN(Comma);
			CASE_RETURN(Semicolon);
			CASE_RETURN(Period);
			CASE_RETURN(Equal);
			CASE_RETURN(Assign);
			CASE_RETURN(Unequal);
			CASE_RETURN(LThan);
			CASE_RETURN(LThanEq);
			CASE_RETURN(GThan);
			CASE_RETURN(GThanEq);
			CASE_RETURN(Plus);
			CASE_RETURN(Minus);
			CASE_RETURN(Mul);
			CASE_RETURN(Eof);
#undef CASE_RETURN

default: return "TokenError"; //Should never happen
		}
	}

	Token(TokenType type, std::string word, FilePosition filePos = FilePosition{ 0, 0 })
		: type( type ),
		m_string( word ),
		filePosition(filePos)
	{}

	Token( TokenType type )
		: type( type )
	{}

	Token()
	{}

private:
	std::string m_string;
};

#endif