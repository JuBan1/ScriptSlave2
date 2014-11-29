#ifndef LEXER_H
#define LEXER_H

#include <vector>
#include <string>
#include "Token.h"

bool Tokenize( std::string inString, std::vector<Token>& tokens );

#endif