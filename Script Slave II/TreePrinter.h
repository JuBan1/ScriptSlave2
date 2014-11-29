#ifndef TREEPRINTER_H
#define TREEPRINTER_H

#include "BaseVisitor.h"
#include <string>

class TreePrinter : public BaseVisitor
{
public:

	bool inNode(Expr* n, bool last);

	bool inNode( ASTNode* n, bool last );
	void outNode( ASTNode* n, bool last );

	//bool inNode(Ident* n, bool last);

private:
	std::string indent;
};

#endif