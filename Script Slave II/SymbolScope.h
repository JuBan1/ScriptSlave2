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
	const std::string name;

	SymbolScope const* scope;

	//TODO: Add a ISymbol interface to specific descendants of ASTNode to get rid of most of the switch boilerplate
	enum SymbolType{
		FUNCTION,
		VARIABLE,
		PARAMETER,
		GLOBAL_VAR,
		CLASS_VAR,
		RETURN_VALUE,
		CLASS
	};

	SymbolType type;

	ASTNode const* GetNode() const {
		return node;
	}

	TypeInfo const* GetTypeInfo() const {
		switch (type){
		case FUNCTION:		return funcNode->GetRetType()->GetTypeInfo();
		case VARIABLE:		return varNode->GetType()->GetTypeInfo();
		case GLOBAL_VAR:	return globVarNode->GetType()->GetTypeInfo();
		case CLASS_VAR:		return classVarNode->GetType()->GetTypeInfo();
		case PARAMETER:		return paramNode->GetType()->GetTypeInfo();
		case RETURN_VALUE:	return retTypeNode->GetTypeInfo();
		case CLASS:			return nullptr;
		default: throw std::invalid_argument("Invalid type of Symbol");
		}
	}

	ParamList const* GetParamList() const {
		if (type == FUNCTION)
			return funcNode->GetParamList();
		else
			return nullptr;
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

	static Symbol CreateGlobalVariableSymbol(GlobVarDef const* globVarNode){
		return Symbol(GLOBAL_VAR, globVarNode->GetName()->GetName(), globVarNode);
	}

	static Symbol CreateClassVariableSymbol(ClassVar const* cVarNode){
		return Symbol(CLASS_VAR, cVarNode->GetName()->GetName(), cVarNode);
	}

	static Symbol CreateClassSymbol(ClassDef const* cNode){
		return Symbol(CLASS, cNode->GetName()->GetName(), cNode);
	}

	static Symbol CreateParamSymbol(Param const* paramNode){
		return Symbol(PARAMETER, paramNode->GetName()->GetName(), paramNode);
	}

	static Symbol CreateReturnValueSymbol(std::string name, Type const* retTypeNode){
		return Symbol(RETURN_VALUE, name, retTypeNode);
	}

private:

	union{
		ASTNode const* node;
		FuncDef const* funcNode;
		StmtVarDecl const* varNode;
		GlobVarDef const* globVarNode;
		Type const* retTypeNode;
		Param const* paramNode;
		ClassVar const* classVarNode;
		ClassDef const* classNode;
	};

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

	enum PreferredSymbol{
		Function,
		Variable,
		Any
	};

	Symbol const*	GetSymbol( std::string name, PreferredSymbol ps = Any ) const;
	bool			AddSymbol( std::string name, Symbol );

	SymbolScope* GetParentScope() const { return m_parent; }

	const SymbolScope* GetSubScope( std::string name ) const;
	SymbolScope* GetSubScope(std::string name);

	bool AddSubScope(std::string name){
		if (GetSubScope(name)) //Return if scope already exists
			return false;

		auto k = m_subScopes.emplace(name, SymbolScope(name, this));
		return true;
	}

	void PrintAll() const {
		//std::cout << "== " << GetQualifiedScopeName() << " ==\n";
		for (auto&& sym : m_symbols)
			std::cout << sym.second.GetQualifiedSignature() << "\n";

		for (auto&& scope : m_subScopes)
			scope.second.PrintAll();
	}

};

#endif