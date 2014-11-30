#pragma once

#include <string>
#include <vector>
#include "TypeTable.h"
#include "SymbolScope.h"
#include "BaseVisitor.h"
#include "StringUtil.h"

class SecondPass : public BaseVisitor
{
public:

	//Enter and leave the scopes
	virtual bool inNode(StmtBlock* n, bool last) override { //Always return true from here so we can keep checking for other errors within the function
		std::string scopeName = "block" + std::to_string(n->GetToken().filePosition.line) + ":" + std::to_string(n->GetToken().filePosition.pos);

		m_currentScope = m_currentScope->GetSubScope(scopeName);
		return true;
	}

	virtual void outNode(StmtBlock* n, bool last){
		m_currentScope = m_currentScope->GetParentScope();
	}

	virtual bool inNode(FuncDef* n, bool last) override { //Always return true from here so we can keep checking for other errors within the function
		m_currentScope = m_currentScope->GetSubScope(n->GetName()->GetName());
		return true;
	}

	virtual void outNode(FuncDef* n, bool last){
		m_currentScope = m_currentScope->GetParentScope();
	}

	//attach symbol and typeinfo to every ident
	virtual bool inNode(Ident* n, bool last) override {
		if (n->GetSymbol() != nullptr)
			return false; //if ident already has a symbol attached, do not look for one again

		FindIdentSymbol(n, SymbolScope::Any);

		return false;
	}


	virtual bool inNode(FuncCallExpr* n, bool last) override {
		auto callee = n->GetCallee();

		FindIdentSymbol(callee, SymbolScope::Function);

		return true;
	}

	virtual bool inNode(StmtFuncCall* n, bool last) override {
		
		auto callee = n->GetName();

		FindIdentSymbol(callee, SymbolScope::Function);

		return true;
	}

	virtual void outNode(StmtAssign* n, bool last);
	virtual void outNode(StmtVarDecl* n, bool last);
	virtual void outNode(StmtWhile* n, bool last);
	virtual void outNode(StmtIfThen* n, bool last);
	virtual void outNode(StmtIfThenElse* n, bool last);
	virtual void outNode(StmtFuncCall* n, bool last);

	virtual void outNode(StmtReturn* n, bool last){
		auto retVal = m_currentScope->GetSymbol(":retVal:");
		TypeInfo const* retType = retVal->GetTypeInfo();

		if (!n->GetExpr() && retType->name != "void"){
			AddError(n->GetToken(), "Expected return type of type '%s'", retType->name.c_str());
			return;
		}
		else if (!n->GetExpr() && retType->name == "void")
			return;
			
		TypeInfo const* exprType = n->GetExpr()->GetTypeInfo();

		if (retType != exprType){
			AddError(n->GetToken(), "Expected return type of type '%s' but found '%s'", retType->name.c_str(), exprType->name.c_str());
			return;
		}

	}

	SecondPass(TypeTable& typeTable, SymbolScope& symbolScope)
		: m_typeTable(typeTable), m_globalScope(symbolScope), m_currentScope(&symbolScope)
	{}

	void Process(StartBlockPtr& start){
		start->accept(this, false);
	}

	const std::vector<std::pair<std::string, Token>>& GetErrors() const {
		return m_errors;
	}

private:

	void FindIdentSymbol(Ident* n, SymbolScope::PreferredSymbol ps){
		Symbol const* sym = m_currentScope->GetSymbol(n->GetName(), ps);

		if (sym == nullptr){ //unknown symbol.
			AddError(n->GetToken(), "Unknown symbol '%s'.", n->GetName().c_str());
		}
		else if (sym->type == Symbol::VARIABLE && sym->varNode->GetToken().filePosition > n->GetToken().filePosition) { //If sym->type == PARAMETER, the symbol can't be used before it's decl.
			AddError(n->GetToken(), "Symbol '%s' used before its declaration in line %d.", n->GetName().c_str(), n->GetToken().filePosition.line);
		}
		else{
			n->SetSymbol(sym);
			n->SetTypeInfo(sym->GetTypeInfo());
		}
	}

	void AddError(const Token& tok, const std::string fmt_str, ...){
		va_list ap;
		va_start(ap, fmt_str);
		std::string v = va_string_format(fmt_str, ap);
		m_errors.emplace_back(v, tok);
		va_end(ap);
	}

	TypeInfo const* GetResultTypeOf(Expr* p, Symbol const* doNotUse);

	std::vector<std::pair<std::string, Token>> m_errors;
	const TypeTable& m_typeTable;			//Since we're using pointers to TypeInfo, SymbolScope and Symbol objects, these collections must not be modified
	const SymbolScope& m_globalScope;		//after this stage! Resizing/reordering of the underlying containers might otherwise invalidate the pointers.
	const SymbolScope* m_currentScope;
};

