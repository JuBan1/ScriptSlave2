#include "TreePrinter.h"

#include "ASTNode.h"

#include <iostream>
#include "SymbolScope.h"

bool TreePrinter::inNode(Expr* n, bool last){

	std::string ti = n->GetTypeInfo() ? n->GetTypeInfo()->name : "<none>";

	std::cout << indent << "+-" << n->GetNodeAsString() << " : " << ti << "\n";
	if (last)
		indent += "  ";
	else
		indent += "| ";

	return true;
}

bool TreePrinter::inNode( ASTNode* n, bool last ){

	std::cout << indent << "+-" << n->GetNodeAsString() << "\n";
	if( last )
		indent += "  ";
	else
		indent += "| ";
	
	return true;
}
void TreePrinter::outNode( ASTNode* n, bool last ){
	indent.erase( indent.end() - 2, indent.end() );	//C++11 fix, doesn't have pop_back yet.
	//indent.pop_back();
	//indent.pop_back();
}

//
//bool TreePrinter::inNode(Ident* n, bool last){
//	if (n->GetSymbol())
//		std::cout << indent << "+-" << n->GetSymbol()->GetSignature() << "\n";
//	else
//		std::cout << indent << "+-" << n->GetNodeAsString() << "\n";
//
//	if (last)
//		indent += "  ";
//	else
//		indent += "| ";
//
//	return true;
//}
