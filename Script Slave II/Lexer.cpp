#include "Lexer.h"

#include <algorithm>
#include <string.h>
#include <stdexcept>

struct TokenPair{
	TokenType type;
	const char* literal;
};

TokenPair g_wordPairs[] = {
	{ TokenType::Var, "var" },
	{ TokenType::If, "if" },
	{ TokenType::Then, "then" },
	{ TokenType::Else, "else" },
	{ TokenType::While, "while" },
	{ TokenType::Do, "do" },
	{ TokenType::Let, "let" },
	{ TokenType::Break, "break" },
	{ TokenType::Return, "return" },
	{ TokenType::Class, "class" },
	{ TokenType::Method, "method" },
	{ TokenType::Function, "function" },
	{ TokenType::Constructor, "constructor" },
	{ TokenType::Static, "static" },
	{ TokenType::Field, "field" },
	{ TokenType::Null, "null" },
	{ TokenType::This, "this" },
};

/*
	Do not add keywords that start with a letter (or number) into g_controlPairs!
	If an operator here does not work, it might start with a character that is not covered in 
	isControlChar(char) which is called for the first character.
*/
TokenPair g_controlPairs[]{
	{ TokenType::LParen, "(" },
	{ TokenType::RParen, ")" },
	{ TokenType::LBrace, "{" },
	{ TokenType::RBrace, "}" },
	{ TokenType::LBracket, "[" },
	{ TokenType::RBracket, "]" },
	{ TokenType::Not, "~" },
	{ TokenType::Comma, "," },
	{ TokenType::Semicolon, ";" },
	{ TokenType::Period, "." },
	
	{ TokenType::And, "&&" },
	{ TokenType::Or, "||" },

	{ TokenType::Equal, "==" },
	{ TokenType::Assign, "=" },
	{ TokenType::Unequal, "!=" },
	{ TokenType::LThanEq, "<=" },
	{ TokenType::LThan, "<" },
	{ TokenType::GThanEq, ">=" },
	{ TokenType::GThan, ">" },

	{ TokenType::Plus, "+" },
	{ TokenType::Minus, "-" },
	{ TokenType::Mul, "*" },
	{ TokenType::Div, "/" }
};

bool isControlChar( char c ){
	return (c > 32 && c < 48) || (c > 57 && c < 65) || (c > 90 && c < 97) || (c > 122 && c < 127);
}

Token lookupWord( const char* start, int length ){
	//Look for true and false
	const std::string lexeme( start, length );

	if (lexeme == "true" || lexeme == "false"){
		//True and False could also be two different token types. Here it's hardcoded.
		return Token(TokenType::BoolLit, lexeme);
	}

	auto f = std::find_if( std::begin( g_wordPairs ), std::end( g_wordPairs ),
		[=]( const TokenPair& in ) {
			return strlen( in.literal ) == length && strncmp( in.literal, start, length ) == 0;
	}
	);

	if( std::end( g_wordPairs ) != f )
		return Token( f->type, f->literal );
	else
		return Token( TokenType::Ident, lexeme ); //return lexeme as identifier if it's not a known keyword
}

//returns the number of characters consumed.
int lookupControl( const char* start, size_t length, Token& token ){

	auto f = std::find_if( std::begin( g_controlPairs ), std::end( g_controlPairs ),
		[=]( const TokenPair& in ) {
			return strlen( in.literal ) <= length && strncmp( in.literal, start, std::min( strlen( in.literal ), length ) ) == 0;
	}
	);

	if( std::end( g_controlPairs ) != f ){
		token.type = f->type;
		token.SetTokenValue(f->literal);

		return strlen( f->literal );
	}
	else{
		return 0;
	}
}

//Counts number of lines between src and pos in a string.
int lineCount(const char* src, const char* pos){
	return std::count( src, pos, '\n' ) + 1; //0-based, so +1
}

FilePosition GetFilePosition(const char* src, const char* pos){
	size_t line = std::count(src, pos, '\n') + 1; //0-based, so +1
	
	auto rbegin = std::reverse_iterator<const char*>(pos);
	auto rend = std::reverse_iterator<const char*>(src) + 1;
	auto last = std::find(rbegin, rend, '\n');

	int posc;
	if (last == rend)
		posc = pos - src;
	else
		posc = last - rbegin;

	int a = (int) src;
	int b = (int) pos;
	int c = *last;

	return FilePosition{ (unsigned short) line, (unsigned short) (posc + 1) };
}

bool lex( const char *src, std::vector<Token>& tokens ) {
	using std::abs;
	
	const char *beyond = src;
	const char *lim = src + strlen( src );

	for( ;; ) {
		// until EOL
		const char* begin = beyond;
		if( beyond >= lim ) 
			return true;

		if( *begin == 0 )
			return true;

		//Comments
		//if remaining length at least 2 and starts with "//"
		if( lim - begin >= 2 && *begin == '/' && *(begin + 1) == '/' ){
			while( ++beyond < lim && *beyond != '\n' && *beyond != 0 );
			continue;
		}

		/*Comments*/
		//if remaining length at least 2 and starts with "/*"
		if( abs( begin - lim ) >= 2 && * begin == '/' && *(begin + 1) == '*' ){
			while( ++beyond < lim && !(*(beyond - 1) == '*' && *beyond == '/') ); //while current part is not "*/"
			beyond += 1;
			continue;
		}

		//Whitespace
		if( isspace( *begin ) ){
			while( ++beyond < lim && isspace( *beyond ) );
			continue;
		}

		//Names or word-identifier
		if( isalpha( *begin ) || *begin == '$' || *begin == '_' ){
			while( ++beyond < lim && (isalnum( *beyond ) || *beyond == '$' || *beyond == '_') );	 // L(L|D)* and L=letter|$|_
			auto token = lookupWord(begin, beyond - begin);
			token.filePosition = GetFilePosition(src, begin);
			tokens.push_back( token );
			continue;
		}

		//String literals
		if( *begin == '"' ){
			while( *beyond == '\\' || *(++beyond) != '"' );
			auto token = Token(TokenType::StringLit, std::string(begin + 1, beyond - begin - 1));
			token.filePosition = GetFilePosition(src, begin);
			tokens.push_back( token );
			beyond++;
			continue;
		}

		//Any control characters: + , . - / ( ) [ ] etc
		while( isControlChar( *beyond ) ) { ++beyond; }

		if( begin != beyond ) {
			Token t;
			int readChars = lookupControl( begin, beyond - begin, t );

			//Also sends error if character is read that turns out to be used elsewhere (e.g. ")
			if( readChars == 0 ){
				auto beg = std::find( std::reverse_iterator<const char*>(begin), std::reverse_iterator<const char*>(src), '\n' );
				auto end = std::find( begin, lim, '\n' );

				printf( "Error, unknown token found during lexing: %s\n", std::string(begin,beyond-begin).c_str() );
				printf( "Line %d: %s\n", lineCount(src, begin), std::string( beg.base(), end ).c_str() );

				return false;
			}

			t.filePosition = GetFilePosition(src, begin);
			tokens.push_back( t );
			beyond = begin + readChars;
			continue;
		}

		//Numbers
		if( isdigit( *begin ) ){
			while( ++beyond < lim && isdigit( *beyond ) );	 // D+

			const char* decimals = nullptr; //look for '.' followed by more numbers
			if (*beyond == '.'){
				decimals = beyond;
				while (++decimals < lim && isdigit(*decimals));	 // D+
				beyond = decimals;
			}

			if( isalpha( *beyond ) || *beyond == '$' || *beyond == '_' ){
				//letters directly after number, throw error
				auto beg = std::find( std::reverse_iterator<const char*>(begin), std::reverse_iterator<const char*>(src), '\n' );
				auto end = std::find( begin, lim, '\n' );

				printf( "Error, invalid integral suffix: %c\n", *beyond );
				printf( "Line %d: %s\n", lineCount( src, begin ), std::string( beg.base(), end ).c_str() );

				return false;
			}

			Token t;
			if (decimals)
				t = Token(TokenType::FloatLit, std::string(begin, beyond - begin));
			else
				t = Token(TokenType::IntLit, std::string(begin, beyond - begin));

			t.filePosition = GetFilePosition(src, begin);
			tokens.push_back(t);

			continue;
		}

		//Error
		throw std::runtime_error("The Lexer has encountered a critical problem."); //Shit's going down!

	}

	return true;
}

bool Tokenize( std::string inString, std::vector<Token>& tokens ){
	bool retVal = lex( inString.c_str(), tokens );


	Token eof(TokenType::Eof, "Eof", GetFilePosition(inString.c_str(), inString.c_str() + inString.length()));
	tokens.push_back( eof );

	return retVal;
}