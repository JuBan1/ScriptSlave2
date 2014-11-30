#pragma once

#include "BaseVisitor.h"
#include "TypeTable.h"
#include "SymbolScope.h"
#include "StringUtil.h"

class FirstPass : public BaseVisitor
{
public:
	FirstPass(TypeTable& typeTable, SymbolScope& symbolScope)
		: m_typeTable(typeTable), m_globalScope(symbolScope), m_currentScope(&symbolScope)
	{}

	void Process(StartBlockPtr& start){
		start->accept(this, false);
	}

	const std::vector<std::pair<std::string, Token>>& GetErrors() const {
		return m_errors;
	}

	virtual bool inNode(GlobVarDef* n, bool last) override {
		auto type = m_typeTable.Get(n->GetType()->GetName());

		if (!type){
			AddError(n->GetToken(), "Unknown type '%s'.", n->GetType()->GetName().c_str());
			return false;
		}
		else if (type->size == 0){
			AddError(n->GetToken(), "Type '%s' can not be used.", n->GetType()->GetName().c_str());
			return false;
		}

		n->GetType()->SetTypeInfo(type);

		bool success = m_currentScope->AddSymbol(n->GetName()->GetName(), Symbol::CreateGlobalVariableSymbol(n));

		if (!success)
			AddError(n->GetToken(), "Symbol '%s' is already in use.", n->GetType()->GetName().c_str());

		return true; //return true so we also walk through the children of n
	}

	virtual bool inNode(StmtVarDecl* n, bool last) override {
		auto type = m_typeTable.Get(n->GetType()->GetName());

		if (!type){
			AddError(n->GetToken(), "Unknown type '%s'.", n->GetType()->GetName().c_str());
			return false;
		}
		else if (type->size == 0){
			AddError(n->GetToken(), "Type '%s' can not be used.", n->GetType()->GetName().c_str());
			return false;
		}

		n->GetType()->SetTypeInfo(type);
		
		bool success = m_currentScope->AddSymbol(n->GetName()->GetName(), Symbol::CreateVariableSymbol(n));

		if (!success)
			AddError(n->GetToken(), "Symbol '%s' is already in use.", n->GetType()->GetName().c_str());

		return true; //return true so we also walk through the children of n
	}

	virtual bool inNode(StmtBlock* n, bool last) override { //Always return true from here so we can keep checking for other errors within the function
		std::string scopeName = "block" + std::to_string(n->GetToken().filePosition.line) + ":" + std::to_string(n->GetToken().filePosition.pos);

		m_currentScope->AddSubScope(scopeName);
		m_currentScope = m_currentScope->GetSubScope(scopeName);

		return true;
	}

	virtual void outNode(StmtBlock* n, bool last){
		m_currentScope = m_currentScope->GetParentScope();
	}

	virtual bool inNode(FuncDef* n, bool last) override { //Always return true from here so we can keep checking for other errors within the function
		std::string retTypeName = n->GetRetType()->GetName();
		auto retType = m_typeTable.Get(n->GetRetType()->GetName());

		if (!retType){
			AddError(n->GetToken(), "Unknown return type '%s'.", n->GetRetType()->GetName().c_str());
			return true;
		}

		n->GetRetType()->SetTypeInfo(retType);
		
		bool success = m_globalScope.AddSymbol(n->GetName()->GetName(), Symbol::CreateFunctionSymbol(n));

		if (!success){
			AddError(n->GetName()->GetToken(), "Symbol '%s' is already in use.", n->GetName()->GetName().c_str());
			return false; //Do not continue if function name is redeclared.
		}
			

		m_currentScope->AddSubScope(n->GetName()->GetName());
		m_currentScope = m_currentScope->GetSubScope(n->GetName()->GetName());
		
		std::string retValName = ":retVal:";
		success = m_currentScope->AddSymbol(retValName, Symbol::CreateReturnValueSymbol(retValName, n->GetRetType()));

		if (!success){
			AddError(n->GetToken(), "Could not generate symbol for return value in '%s'.", n->GetName()->GetName().c_str());
			return false; //Do not continue if function name is redeclared.
		}

		for (const auto& param : n->GetParamList()->GetChildren()){
			auto paramType = m_typeTable.Get(param->GetType()->GetName());

			if (!paramType){
				AddError(n->GetToken(), "Unknown parameter type '%s'.", n->GetRetType()->GetName());
				return true;
			}

			param->GetType()->SetTypeInfo(paramType);
			bool success = m_currentScope->AddSymbol(param->GetName()->GetName(), Symbol::CreateParamSymbol(param.get()));

			if (!success)
				AddError(n->GetToken(), "Symbol '%s' already in use.", n->GetName()->GetName());
		}

		return true;
	}

	virtual void outNode(FuncDef* n, bool last){
		m_currentScope = m_currentScope->GetParentScope();
	}

	virtual bool inNode(IntLit* n, bool last) override {
		n->SetTypeInfo(m_typeTable.Get("int"));
		return false;
	}

	virtual bool inNode(FloatLit* n, bool last) override {
		n->SetTypeInfo(m_typeTable.Get("float"));
		return false; 
	}

	virtual bool inNode(StringLit* n, bool last) override {
		n->SetTypeInfo(m_typeTable.Get("string"));
		return false;
	}

	virtual bool inNode(BoolLit* n, bool last) override {
		n->SetTypeInfo(m_typeTable.Get("bool"));
		return false;
	}

	virtual bool inNode(StmtBreak* n, bool last){ if (m_breakDepth == 0) AddError(n->GetToken(), "Break outside of loop."); return false; }
	virtual void outNode(StmtBreak* n, bool last){ }

	virtual bool inNode(StmtWhile* n, bool last){ m_breakDepth++; return true; }
	virtual void outNode(StmtWhile* n, bool last){ m_breakDepth--; }

private:

	void AddError(const Token& tok, const std::string fmt_str, ...){
		va_list ap;
		va_start(ap, fmt_str);
		std::string v = va_string_format(fmt_str, ap);
		m_errors.emplace_back(v, tok);
		va_end(ap);
	}


	int m_breakDepth = 0;

	std::vector<std::pair<std::string, Token>> m_errors;
	TypeTable& m_typeTable;
	SymbolScope& m_globalScope;
	SymbolScope* m_currentScope;
};

