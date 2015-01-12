#include "SecondPass.h"

TypeInfo const* SecondPass::GetResultTypeOf(Expr* p, Symbol const* doNotUse = nullptr){
	NodeType nt = p->GetNodeType();

	switch (nt){
	case NodeType::IntLit:
	case NodeType::FloatLit:
	case NodeType::BoolLit:
	case NodeType::StringLit:
		return p->GetTypeInfo();
	case NodeType::Ident:
	{
							
		Ident* n = (Ident*)p;
		if (n->GetTypeInfo() == nullptr)
			return nullptr;
		if (n->GetSymbol()->type == Symbol::FUNCTION){
			AddError(n->GetToken(), "Symbol '%s' is a function, but used like a variable.", n->GetName().c_str());
			return nullptr;
		}
		if (doNotUse == nullptr) 
			return p->GetTypeInfo();
		else{
			
			if (n->GetSymbol() == doNotUse){
				AddError(n->GetToken(), "Symbol '%s' is used in its definition.", n->GetName().c_str());
				return nullptr;
			}
			return p->GetTypeInfo();
		}
	}

	case NodeType::UnOp:
	{
		UnOp* n = (UnOp*)p;
		UnOp::Types ntype = n->GetType();
		TypeInfo const* ti = GetResultTypeOf(n->GetExpr());

		if (ti == nullptr)
			return nullptr;

		if (ntype == UnOp::Neg && ti->name != "int" && ti->name != "float"){
			AddError(n->GetExpr()->GetToken(), "Type mismatch: Expected 'int' type but found '%s' in operation '%s'.", ti->name.c_str(), UnOp::GetTypeAsString(ntype));
			return nullptr;
		}
		if( ntype == UnOp::Not && ti->name != "bool" ){
			AddError(n->GetExpr()->GetToken(), "Type mismatch: Expected 'bool' type but found '%s' in operation '%s'.", ti->name.c_str(), UnOp::GetTypeAsString(ntype));
			return nullptr;
		}

		p->SetTypeInfo(m_typeTable.Get("bool"));
		return p->GetTypeInfo();
	}
	case NodeType::BinOp:
	{
		BinOp* n = (BinOp*) p;
		TypeInfo const* lt = GetResultTypeOf(n->GetLeft());
		TypeInfo const* rt = GetResultTypeOf(n->GetRight());

		if (lt == nullptr || rt == nullptr)
			return nullptr;

		switch (n->GetType()){
		case BinOp::Equal:
		case BinOp::Unequal:
			{
				if( lt != rt ){
					AddError(n->GetToken(), "Type mismatch: Expected same types but found '%s' and '%s' in operation '%s'.", lt->name.c_str(), rt->name.c_str(), BinOp::GetTypeAsString(n->GetType()));
					return nullptr;
				}
				p->SetTypeInfo(m_typeTable.Get("bool"));
				return p->GetTypeInfo();
			}
		case BinOp::LThan:
		case BinOp::LThanEq:
		case BinOp::GThan:
		case BinOp::GThanEq:
			{
				if( rt != lt || rt->name != "int" || rt->name != "float" ){
					AddError(n->GetToken(), "Type mismatch: Expected matching numeric types but found '%s' and '%s' in operation '%s'.", lt->name.c_str(), rt->name.c_str(), BinOp::GetTypeAsString(n->GetType()));
					return nullptr;
				}
				p->SetTypeInfo(m_typeTable.Get("bool"));
				return p->GetTypeInfo();
			}
		case BinOp::And:
		case BinOp::Or:
		case BinOp::Xor:
			{
				if( lt->name != "bool" || rt->name != "bool" ){
					AddError(n->GetToken(), "Type mismatch: Expected 'bool' types but found '%s' and '%s' in operation '%s'.", lt->name.c_str(), rt->name.c_str(), BinOp::GetTypeAsString(n->GetType()));
					return nullptr;
				}
				p->SetTypeInfo(m_typeTable.Get("bool"));
				return p->GetTypeInfo();
			}
		case BinOp::Sub:
		case BinOp::Mul:
		case BinOp::Div:
			{
				if( lt != rt || lt->name != "int" && lt->name != "float" ){
					AddError(n->GetToken(), "Type mismatch: Expected matching numeric types but found '%s' and '%s' in operation '%s'.", lt->name.c_str(), rt->name.c_str(), BinOp::GetTypeAsString(n->GetType()));
					return nullptr;
				}
				p->SetTypeInfo(lt);
				return p->GetTypeInfo();
			}
		case BinOp::Mod:
			{
				if (lt != rt || lt->name != "int"){
					AddError(n->GetToken(), "Type mismatch: Expected 'int' types but found '%s' and '%s' in operation '%s'.", lt->name.c_str(), rt->name.c_str(), BinOp::GetTypeAsString(n->GetType()));
					return nullptr;
				}
				p->SetTypeInfo(lt);
				return p->GetTypeInfo();
			}
		case BinOp::Add:
			{
				if (lt->name == "string" || rt->name == "string"){ //can concatenate strings with everything
					p->SetTypeInfo(m_typeTable.Get("string"));
					return p->GetTypeInfo();
				}
				if (lt != rt || lt->name != "int" && lt->name != "float"){
					AddError(n->GetToken(), "Type mismatch: Expected matching numeric types or strings but found '%s' and '%s' in operation '%s'.", lt->name.c_str(), rt->name.c_str(), BinOp::GetTypeAsString(n->GetType()));
					return nullptr;
				}
				p->SetTypeInfo(lt);
				return p->GetTypeInfo();
			}
		case BinOp::Subscript:
		{
				if (!lt->isArray){
					AddError(n->GetRight()->GetToken(), "Type mismatch: Expected array type but found '%s' in operation '%s'.", rt->name.c_str(), BinOp::GetTypeAsString(n->GetType()));
					return nullptr;
				}

				if (rt->name != "int"){
					AddError(n->GetRight()->GetToken(), "Type mismatch: Expected numeric type but found '%s' in operation '%s'.", rt->name.c_str(), BinOp::GetTypeAsString(n->GetType()));
					return nullptr;
				}

				auto simpleType = m_typeTable.Get(lt->GetSimpleTypeName());
				_ASSERT(simpleType != nullptr);

				p->SetTypeInfo(simpleType);
				return p->GetTypeInfo();
		}

		default:
			__debugbreak();
			return nullptr;
		}
	}

	case NodeType::FuncCallExpr:
	{
		FuncCallExpr* n = (FuncCallExpr*)p;
		
		Symbol const* calleeSym = n->GetCallee()->GetSymbol();
		
		TypeInfo const* calleeTI = p->GetTypeInfo(); // GetResultTypeOf(n->GetCallee());

		if (calleeSym->type != Symbol::FUNCTION){
			AddError(n->GetCallee()->GetToken(), "Symbol '%s' is not a function, but used as one.", calleeSym->name.c_str());
			return nullptr;
		}
		
		p->SetTypeInfo(calleeSym->GetTypeInfo());

		//check arguments
		const auto& args = n->GetArgs()->GetChildren();
		const auto& params = calleeSym->GetParamList()->GetChildren();

		if (args.size() != params.size()){
			AddError(n->GetCallee()->GetToken(), "Function requires %d arguments, but %d supplied.", params.size(), args.size());
			return p->GetTypeInfo();
		}

		for (size_t i = 0; i < args.size(); ++i){
			TypeInfo const* actual = GetResultTypeOf(args[i].get());
			TypeInfo const* required = params[i]->GetType()->GetTypeInfo();

			if (actual != required){
				AddError(args[i]->GetToken(), "%dth argument is type '%s' but must be of type '%s'.", i+1, required->name.c_str(), actual->name.c_str());
				//return p->GetTypeInfo();
			}
		}

		//All checked
		return p->GetTypeInfo();
	}
		
	default:
		__debugbreak();
		return nullptr;
	}
}

void SecondPass::outNode(StmtAssign* n, bool last){
	
	auto varType = GetResultTypeOf(n->GetLHS());  //n->GetLHS()->GetTypeInfo();
	auto exprType = GetResultTypeOf( n->GetExpr() );


	if( exprType == nullptr || varType == nullptr )
		return; //GetResultTypeOf( ... ) has already added an error for this. Return without doing anything.

	else if( varType != exprType ){
		AddError(n->GetToken(), "Attempting to assign a '%s' to a variable of type '%s'.", exprType->name.c_str(), varType->name.c_str());
	}

}

void SecondPass::outNode(StmtVarDecl* n, bool last){

	if (n->GetExpr() == nullptr) //No type checking to be done if there is no initialization
		return;

	auto varType = n->GetName()->GetTypeInfo();
	auto exprType = GetResultTypeOf(n->GetExpr(), n->GetName()->GetSymbol());

	if (exprType == nullptr || varType == nullptr)
		return; //GetResultTypeOf( ... ) has already added an error for this. Return without doing anything.

	else if (varType != exprType){
		AddError(n->GetToken(), "Attempting to initialize an '%s' to a variable of type '%s'.", varType->name.c_str(), exprType->name.c_str());
	}

}

void SecondPass::outNode(StmtWhile* n, bool last){
	auto exprType = GetResultTypeOf( n->GetExpr() );

	if( exprType == nullptr )
		return;

	if( exprType->name != "bool" ){
		AddError(n->GetToken(), "Result type of while-condition must be 'bool' but is '%s'.", exprType->name.c_str());
	}
}

void SecondPass::outNode(StmtIfThen* n, bool last){
	auto exprType = GetResultTypeOf( n->GetExpr() );

	if (exprType == nullptr)
		return;

	if (exprType->name != "bool"){
		AddError(n->GetToken(), "Result type of if-condition must be 'bool' but is '%s'.", exprType->name.c_str());
	}
}

void SecondPass::outNode(StmtIfThenElse* n, bool last){
	auto exprType = GetResultTypeOf(n->GetExpr());

	if (exprType == nullptr)
		return;

	if (exprType->name != "bool"){
		AddError(n->GetToken(), "Result type of if-else-condition must be 'bool' but is '%s'.", exprType->name.c_str());
	}
}

void SecondPass::outNode(StmtFuncCall* n, bool last){
	TypeInfo const* calleeTI = n->GetName()->GetTypeInfo(); // GetResultTypeOf(n->GetName());
	Symbol const* calleeSym = n->GetName()->GetSymbol();

	if (calleeSym->type != Symbol::FUNCTION){
		AddError(n->GetToken(), "Symbol '%s' is not a function but used as such.", calleeSym->name.c_str());
		return;
	}

	//check arguments
	const auto& args = n->GetArgList()->GetChildren();
	const auto& params = calleeSym->GetParamList()->GetChildren();

	if (args.size() != params.size()){
		AddError(n->GetToken(), "Function requires %d arguments, but %d supplied.", params.size(), args.size());
		return;
	}

	for (size_t i = 0; i < args.size(); ++i){
		TypeInfo const* actual = GetResultTypeOf(args[i].get());
		TypeInfo const* required = params[i]->GetType()->GetTypeInfo();

		if (actual != required){
			AddError(args[i]->GetToken(), "%dth argument is type '%s' but must be of type '%s'.", i + 1, required->name.c_str(), actual->name.c_str());
			//return;
		}
	}

	//All checked
}