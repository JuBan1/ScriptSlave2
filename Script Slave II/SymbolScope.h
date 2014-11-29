#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <map>
#include "ASTNode.h"
#include "Optional.h"
#include "TypeInfo.h"

#include <iostream>

class SymbolScope;

class Symbol{
public:
	//TODO: Make these things const/unmodifyable. Is const ok?
	std::string name;

	SymbolScope const* scope;

	enum SymbolType{
		FUNCTION,
		VARIABLE,
		PARAMETER,
		RETURN_VALUE
	};

	SymbolType type;

	union{
		ASTNode const* node;
		FuncDef const* funcNode;
		StmtVarDecl const* varNode;
		Type const* retTypeNode;
		Param const* paramNode;
	};

	TypeInfo const* GetTypeInfo() const {
		switch (type){
		case FUNCTION:		return funcNode->GetRetType()->GetTypeInfo();
		case VARIABLE:		return varNode->GetType()->GetTypeInfo();
		case PARAMETER:		return paramNode->GetType()->GetTypeInfo();
		case RETURN_VALUE:	return retTypeNode->GetTypeInfo();
		default: throw std::invalid_argument("Invalid type of Symbol");
		}
	}

	std::string GetQualifiedName() const;
	std::string GetSignature() const;
	std::string GetQualifiedSignature() const;

	static Symbol CreateFunctionSymbol(FuncDef const* funcNode){
		return Symbol(FUNCTION, funcNode->GetName()->GetName(), funcNode);
	}

	static Symbol CreateVariableSymbol(StmtVarDecl const* varNode){
		return Symbol(VARIABLE, varNode->GetName()->GetName(), varNode);
	}

	static Symbol CreateParamSymbol(Param const* paramNode){
		return Symbol(PARAMETER, paramNode->GetName()->GetName(), paramNode);
	}

	static Symbol CreateReturnValueSymbol(std::string name, Type const* retTypeNode){
		return Symbol(RETURN_VALUE, name, retTypeNode);
	}

private:

	Symbol(SymbolType t, std::string name, ASTNode const* node)
		: type(t), name(name), node(node)
	{};
};

class SymbolScope{
	std::string m_scopeName;
	SymbolScope* m_parent;

	std::map<std::string, SymbolScope> m_subScopes;
	std::map<std::string, Symbol> m_symbols;

public:

	SymbolScope(SymbolScope const &) = delete;
	SymbolScope &operator=(SymbolScope const &) = delete;

	SymbolScope& operator=(SymbolScope &&o)
	{
		if (this == &o) return *this;

		m_scopeName = std::move(o.m_scopeName);
		m_parent = std::move(o.m_parent);
		m_subScopes = std::move(o.m_subScopes);
		m_symbols = std::move(o.m_symbols);

		return *this;
	}

	SymbolScope(std::string name, SymbolScope* parent)
		: m_scopeName(name), m_parent(parent)
	{}

	SymbolScope(SymbolScope &&o)
		: m_scopeName(std::move(o.m_scopeName)),
		m_parent(std::move(o.m_parent)),
		m_subScopes(std::move(o.m_subScopes)),
		m_symbols(std::move(o.m_symbols))
	{}

	std::string		GetScopeName() const { return m_scopeName; }

	std::string	GetQualifiedScopeName() const {
		SymbolScope const* current = m_parent;
		std::string fullName = m_scopeName;

		while (current != nullptr){
			fullName = current->GetScopeName() + "." + fullName;
			current = current->GetParentScope();
		}

		return fullName;
	}

	Symbol const*	GetSymbol( std::string name );
	bool			AddSymbol( std::string name, Symbol );

	//const std::map<std::string, std::unique_ptr<Symbol>>& GetAllVariables() const { return m_symbols; };

	SymbolScope* GetParentScope() const { return m_parent; }

	SymbolScope* GetSubScope( std::string name );

	bool AddSubScope(std::string name){
		if (GetSubScope(name)) //No redeclared scopes
			return false;

		auto k = m_subScopes.emplace(name, SymbolScope(name, this));
		return true;
	}

	void PrintAll(){
		//std::cout << "== " << GetQualifiedScopeName() << " ==\n";
		for (auto&& sym : m_symbols)
			std::cout << sym.second.GetQualifiedSignature() << "\n";

		for (auto&& scope : m_subScopes)
			scope.second.PrintAll();
	}

};

#endif