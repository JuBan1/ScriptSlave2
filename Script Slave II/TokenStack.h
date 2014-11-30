#ifndef TOKENSTACK_H
#define TOKENSTACK_H

#include <vector>
#include <stack>
#include <string>
#include <tuple>
#include "Token.h"

class TokenStack{
public:
	TokenStack( const std::vector<Token>& tokens )
		: m_tokens( tokens ),
		m_currIndex( 0 ),
		m_highestIndex( 0 )
	{}

	void PushIndex(){
		m_indices.push( m_currIndex );
	}

	void PopIndex(){
		m_currIndex = m_indices.top();
		m_indices.pop();
	}

	void DiscardIndex(){
		m_indices.pop();
	}

	const Token& GetNextToken( std::string prodName ){
		if( m_highestIndex < m_currIndex ){
			m_highestProductionName = prodName;
			m_highestIndex = m_currIndex;
		}
		return m_tokens[m_currIndex++];
	}

	const Token& GetCurrentToken(){
		return m_tokens[m_currIndex];
	}

	const Token& GetFurthestToken(){
		return m_tokens[m_highestIndex];
	}
	std::string GetFurthestProductionName(){
		return m_highestProductionName;
	}

	void SetFurthestProductionName( std::string name ){ //Good for initialisation.
		m_highestProductionName = name;
	}

	std::tuple<const Token*,const Token*, std::string> GetErrorInfo(){
		if( m_highestIndex > 0 )
			return std::make_tuple( &m_tokens[m_highestIndex - 1], &m_tokens[m_highestIndex], m_highestProductionName );
		else
			return std::make_tuple( nullptr, &m_tokens[m_highestIndex], m_highestProductionName );
	}

	bool IsEmpty(){
		return m_currIndex == m_tokens.size() - 1;
	}

private:
	int m_highestIndex;
	std::string m_highestProductionName;
	int m_currIndex;
	std::stack<int> m_indices;
	const std::vector<Token>& m_tokens;
};

#endif