#include "SymbolScope.h"

#include <iostream>

std::string Symbol::GetQualifiedName() const {
	if (scope)
		return scope->GetQualifiedScopeName() + "." + name;
	else
		return name;
}

std::string Symbol::GetSignature() const {
	switch (type){
	case FUNCTION:
	{
		std::string sig;
		sig = funcNode->GetRetType()->GetTypeInfo()->name + " " + name + "( ";
		for (auto&& param : funcNode->GetParamList()->GetChildren()){
			sig += param->GetName()->GetName() + " : " + param->GetType()->GetTypeInfo()->name + ", ";
		}
		sig += ")";
		return sig;
	}
	case VARIABLE:	return name + " : " + varNode->GetType()->GetTypeInfo()->name;
	case GLOBAL_VAR: return  name + " : " + globVarNode->GetType()->GetTypeInfo()->name;
	case CLASS_VAR: return  name + " : " + classVarNode->GetType()->GetTypeInfo()->name;
	case PARAMETER: return name + " : " + paramNode->GetType()->GetTypeInfo()->name;
	case RETURN_VALUE: return name + " : " + retTypeNode->GetTypeInfo()->name;
	case CLASS: return name + " : " + "class";
	default: throw std::invalid_argument("Invalid type of Symbol");
	}
}

std::string Symbol::GetQualifiedSignature() const {
	if (scope)
		return scope->GetQualifiedScopeName() + "." + GetSignature();
	else
		return GetSignature();
}

Symbol const* SymbolScope::GetSymbol(std::string name, PreferredSymbol ps) const {
	auto it = m_symbols.find(name);

	if (it == m_symbols.end())
		if (m_parent)	return m_parent->GetSymbol(name, ps);
		else			return nullptr;

	//check if symbol is of preferred type
	if (ps == Function && it->second.type == Symbol::FUNCTION)
		return &it->second;
	else if (ps == Variable &&
		(it->second.type == Symbol::VARIABLE ||
		it->second.type == Symbol::PARAMETER ||
		it->second.type == Symbol::GLOBAL_VAR ||
		it->second.type == Symbol::CLASS_VAR ||
		it->second.type == Symbol::RETURN_VALUE))
		return &it->second;
	else if (ps == Any)
		return &it->second;

	auto alternative = m_parent->GetSymbol(name, ps);
	if (alternative != nullptr)
		return alternative;

	return &it->second;
}

bool SymbolScope::AddSymbol( std::string name, Symbol symbol ){
	if (GetSymbol(name)) //Return false if re-declared symbol
		return false;
	
	symbol.scope = this;
	m_symbols.emplace(name, symbol);
	return true;
}

const SymbolScope* SymbolScope::GetSubScope( std::string name ) const {
	auto it = m_subScopes.find( name );

	if (it == m_subScopes.end())
		return nullptr;

	return &it->second;
}

SymbolScope* SymbolScope::GetSubScope(std::string name) {
	auto it = m_subScopes.find(name);

	if (it == m_subScopes.end())
		return nullptr;

	return &it->second;
}