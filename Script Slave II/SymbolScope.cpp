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
	case PARAMETER: return name + " : " + paramNode->GetType()->GetTypeInfo()->name;
	case RETURN_VALUE: return name + " : " + retTypeNode->GetTypeInfo()->name;
	default: throw std::invalid_argument("Invalid type of Symbol");
	}
}

std::string Symbol::GetQualifiedSignature() const {
	if (scope)
		return scope->GetQualifiedScopeName() + "." + GetSignature();
	else
		return GetSignature();
}

Symbol const* SymbolScope::GetSymbol( std::string name ){
	auto it = m_symbols.find( name );

	if (it == m_symbols.end()) 
		if (m_parent)	return m_parent->GetSymbol(name);
		else			return nullptr;

	return &it->second;
}

bool SymbolScope::AddSymbol( std::string name, Symbol symbol ){
	if (GetSymbol(name)) //Return false if re-declared symbol
		return false;
	
	symbol.scope = this;
	m_symbols.emplace(name, symbol);
	return true;
}

SymbolScope* SymbolScope::GetSubScope( std::string name ){
	auto it = m_subScopes.find( name );

	if (it == m_subScopes.end())
		return nullptr;

	return &it->second;
}

/*
class SymbolAccum : public BaseVisitor{
public:
	SymbolScope* m_symbolScope;

	std::string scopeName = "";

	bool inNode( ClassField* n, bool last ){ 
		for( auto&& v : n->GetVarList()->GetChildren() )
			std::cout << "\tfield\t" + n->GetType()->GetName() + "\t" + scopeName + "." + v->GetName() + "\n";
		return false; 
	}

	bool inNode( Param* n, bool last ){
		std::cout << "\tparam\t" + n->GetType()->GetName() + "\t" + scopeName + "." + n->GetName()->GetName() + "\n";
		return false;
	}

	bool inNode( StmtVarDecl* n, bool last ){
		for( auto&& v : n->GetVarList()->GetChildren() )
			std::cout << "\tvar\t" + n->GetType()->GetName()  + "\t" + scopeName + "." + v->GetName() + "\n";
		return false;
	}


	void visit( ClassMethod* n, bool last ){
		std::string oldScopeName = scopeName;
		scopeName += "." + n->GetNameRef()->GetName();

		std::cout << "\nmethod scope " + scopeName + "\n";

		std::cout << "\tmethod\t" + n->GetReturnType()->GetName() + "\t" + oldScopeName + "." + n->GetName()->GetNodeAsString() + "\t(";

		for( auto&& v : n->GetParamList()->GetChildren() ){
			std::cout << v->GetType()->GetName() + ",";
		}
		std::cout << + ")\n";

		BaseVisitor::visit( n, last );
		scopeName = oldScopeName;
	}

	void visit( ClassNode* n, bool last ){
		std::string oldScopeName = scopeName;
		scopeName += "." + n->GetNameRef()->GetName();

		SymbolScope* parent = m_symbolScope;
		m_symbolScope = &(parent->SetSubScope( n->GetName()->GetName() ));
		

		//std::cout << "\nclass scope " + scopeName + "\n";
		BaseVisitor::visit( n, last );

		m_symbolScope = parent;
		scopeName = oldScopeName;
	}

public:
	void visit( StartBlock* n, bool last ){
		m_symbolScope = new SymbolScope(); //global scope

		BaseVisitor::visit( n, last );
	}

};

*/