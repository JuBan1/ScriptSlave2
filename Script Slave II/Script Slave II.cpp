
#include <iostream>
#include <fstream>
#include <algorithm>

#include "Token.h"
#include "Lexer.h"
#include "TreePrinter.h"
#include "ASTNode.h"
#include "TokenStack.h"
#include "RDParser.h"
#include "SymbolScope.h"
#include "CodeGen.h"
#include "BaseVisitor.h"

#include "TypeTable.h"
#include "FirstPass.h"
#include "CollectTypeInfo.h"
#include "SecondPass.h"

//Removes useless nodes from the AST which are a left-over from parsing phase.
class EmptyStmtRemover : public BaseVisitor{
public:

	virtual void outNode( StmtBlock* n, bool last ){ //Want to perform this check *after* the containing elements have been checked.
		auto& children = n->GetChildren( );
		children.erase( std::remove_if( children.begin( ), children.end( ), 
			[]( const StmtPtr& ptr ) -> bool 
		{
			if( ptr->GetNodeType() == NodeType::StmtEmpty )	//Remove EmptyStmt's
				return true;

			if( ptr->GetNodeType() == NodeType::StmtBlock &&		//Remove StmtBlocks which don't contain any Stmts
				((StmtBlock*) ptr.get())->GetChildren().size() == 0 )
				return true;
			return false;
		}
		
		), children.end( ) );


		return;
	}
};

/*
template<class T>
class Expected {

	union{
		T ham;
		std::exception_ptr spam;
	};
	bool gotHam;

	Expected(){} //used internally 
public:
	Expected(const T& rhs)
		: ham(rhs), gotHam(true)
	{}

	Expected(T&&rhs)
		: ham(std::move(rhs)), gotHam(true)
	{}

	Expected(const Expected& rhs) 
		: gotHam(rhs.gotHam)
	{
		if (gotHam) 
			new (&ham) T(rhs.ham); 
		else
			new(&spam) std::exception_ptr(rhs.spam);

	}

	Expected(Expected&&rhs) 
		: gotHam(rhs.gotHam)
	{
		if (gotHam) 
			new (&ham) T(std::move(rhs.ham)); 
		else
			new(&spam) std::exception_ptr(std::move(rhs.spam));
	}

	void swap(Expected&rhs){
		if (gotHam){
			if (rhs.gotHam){
				using std::swap; 
				swap(ham, rhs.ham);
			}
			else{
				auto t = std::move(rhs.spam); 
				new(&rhs.ham) T(std::move(ham)); 
				new(&spam) std::exception_ptr(t); 
				std::swap(gotHam, rhs.gotHam);
			}
		}
		else{
			if (rhs.gotHam){
				rhs.swap(*this);
			}
			else{
				spam.swap(rhs.spam); 
				std::swap(gotHam, rhs.gotHam);
			}
		}
	}

	bool valid()const{
		return gotHam;
	}

	T& get(){
		if (!gotHam)std::rethrow_exception(spam); returnham;

	} const T& get() const {
		if (!gotHam)
			std::rethrow_exception(spam); 
		return ham;
	}
};

template<classE> static Expected<T> fromException(const E& exception){
	if (typeid(exception) != typeid(E)){
		throw std::invalid_argument("slicingdetected");
	} 
	return fromException(std::make_exception_ptr(exception));
}*/


void GetFormattedTokenStringForError(Token t, std::vector<Token>& tokens){
	auto it = std::find_if(tokens.begin(), tokens.end(), [&t](const Token& tok)
	{ 
		return t.filePosition.line == tok.filePosition.line &&
			t.filePosition.pos == tok.filePosition.pos;
	});

	if (it == tokens.end())
		throw std::invalid_argument("Token not found");

	size_t mid = it - tokens.begin();

	int line = tokens[mid].filePosition.line;

	//Get start and end index of line. So [beg,end) are all tokens on line 'line'.
	size_t beg = [&](){ int it = mid; while (it >= 0 && tokens[it].filePosition.line == line) --it; return it; }() + 1; //counts one too far to the left, so +1
	size_t end = [&](){ int it = mid; while (tokens[it].filePosition.line == line) ++it; return it; }();

	size_t count = 0;
	for (size_t i = beg; i < end; ++i){
		const Token& t = tokens[i];
		std::cout << t.GetTokenValue() << " ";

		if (i<mid)
			count += t.GetTokenValue().size() + 1;
	}
	std::cout << "\n";
	std::string ws(count, ' ');
	std::cout << ws << "^\n";
}


TypeTable CreateNativeTypes(){
	TypeTable tt;

	tt.Add(TypeInfo{ "void", 0, false });
	tt.Add(TypeInfo{ "int", 4, false });
	tt.Add(TypeInfo{ "float", 4, false });
	tt.Add(TypeInfo{ "bool", 1, false });
	tt.Add(TypeInfo{ "string", 4, false });

	tt.Add(TypeInfo{ "int", 4, true });
	tt.Add(TypeInfo{ "float", 4, true });
	tt.Add(TypeInfo{ "bool", 1, true });
	tt.Add(TypeInfo{ "string", 4, true });

	return tt;
}

void compile(std::string fileName, bool printLiveness = false){

	std::vector<Token> tokens;

	//READ FILE
	std::ifstream file(fileName);

	if( !file.good() ){
		std::cout << "Error: Could not read file " << fileName << "\n";
		return;
	}

	std::istreambuf_iterator<char> eos;
	std::string s(std::istreambuf_iterator<char>(file), eos);
	//Turn everything into lower caps
	//std::transform( s.begin(), s.end(), s.begin(), ::tolower );

	//LEXING
	bool lexingSuccessful = Tokenize( s, tokens );
	//Return if lexing was unsuccessful. Tokenize() will output some console message regarding the error.
	if( !lexingSuccessful ){
		return;
	}

	
	

	for( auto&& a : tokens ){
		//std::cout << a.GetTypeAsString() + ":\t" + a.GetTokenValue() + "\t" << a.filePosition.line << ":" << a.filePosition.pos << "\n";

		printf("%-16s%-14.12s%d:%d\n", a.GetTypeAsString().c_str(), a.GetTokenValue().c_str(), a.filePosition.line, a.filePosition.pos);
	}

	//PARSING
	TokenStack ts(tokens);
	
	//Start parsing
	StartBlockPtr start;
	bool success = match_start(ts, start);
	
	//Figure out if something went wrong
	if( success && ts.GetCurrentToken().type == TokenType::Eof ){
#ifdef _DEBUG
		std::cout << "Successfully matched.\n\n";
#endif
	}
	else{
		if (!success && ts.GetFurthestToken().type == TokenType::Eof)
			std::cout << "Unexpected end of file.\n";
		else if( success ){
			const auto& err = ts.GetErrorInfo(); //Error info is a tuple of <previous token, erroneous token, production rule>
			std::cout << "End of program found, but not all input was consumed. Last Token read is \"" << std::get<1>(err)->GetTokenValue() << "\".\n";
			GetFormattedTokenStringForError(*std::get<1>(err), tokens);
		}else{
			const auto& err = ts.GetErrorInfo();
			if( std::get<0>(err) == 0 )
				std::cout << "Cannot parse Token \"" << std::get<1>(err)->GetTokenValue() << "\" in " << std::get<2>(err) << ".\n";
			else
				std::cout << "Cannot parse Token \"" << std::get<1>(err)->GetTokenValue() << "\" after Token \"" << std::get<0>(err)->GetTokenValue() << "\" in " << std::get<2>(err) << ".\n";

			GetFormattedTokenStringForError(*std::get<1>(err), tokens);
		}
			
		return;
	}	

	//Clean up AST - remove empty statements. Should probably be integrated into the AST construction in the first place.
	EmptyStmtRemover esr;
	start->accept( &esr, false );

#ifdef _DEBUG
	//PRINT AST
	TreePrinter tv;
	start->accept( &tv, true );
	std::cout << "\n\n";
#endif

	//GENERATE SYMBOLS

	TypeTable typeTable = CreateNativeTypes();
	SymbolScope globalScope("global", nullptr);

	//First pass:
	// Register all functions and store in symbol table
	// Check if break stmts are correctly placed
	// Check if all Type-nodes are actually types
	// Add variables to symbol table, make sure no double definitions

	FirstPass firstPass(typeTable, globalScope);
	firstPass.Process(start);

	for (auto&& error : firstPass.GetErrors()){
		std::cout << error.second.filePosition.ToString() << " " << error.first << "\n";
		GetFormattedTokenStringForError(error.second, tokens);
	}

	if (firstPass.GetErrors().size() > 0)
		return;

	CollectTypeInfo cti(typeTable);
	cti.Process(start);

	for (auto&& error : cti.GetErrors()){
		std::cout << error.second.filePosition.ToString() << " " << error.first << "\n";
		GetFormattedTokenStringForError(error.second, tokens);
	}

	if (cti.GetErrors().size() > 0)
		return;

	//Second pass:
	// Check if all ident-nodes are known variables
	// Deduce types of all expression nodes
	SecondPass secondPass(typeTable, globalScope);
	secondPass.Process(start);

	for (auto&& error : secondPass.GetErrors()){
		std::cout << error.second.filePosition.ToString() << " " << error.first << "\n";
		GetFormattedTokenStringForError(error.second, tokens);
	}


	std::cout << std::endl;

	globalScope.PrintAll();

	std::cout << std::endl;


	std::cout << "\n\n";
	start->accept(&tv, true);
	std::cout << std::endl;

	//GetFormattedTokenStringForError(tokens[25], tokens);




	//CodeGen cg;
	//cg.Generate( start, st );

//#ifndef _DEBUG
	//ALL GOOD, NO ERRORS.
	//Will go haywire if something like "folder./file" is the input name. The first period isn't in the actual file name, so the path will be cut off. 

	auto it = std::find_if( fileName.rbegin(), fileName.rend(), []( const char& c ){ return c == '.'; } );
	std::string outName = fileName.substr( 0, it.base() - fileName.begin() );
	
	if( outName == "" )	//Use the full filename if no ".xyz" ending has been found.
		outName = fileName+".";


	//cg.WriteToFile( outName + "j" );

//#endif
}


int main(int argc, char* argv[])
{
#ifdef _DEBUG
	compile("code.jack");
	std::cin.get();
#else
	if( argc != 3 ){
		std::cout << "Error: Unknown command line input.\n Valid input is:\n"
			<< "\tStupsCompiler -compile [filename.pas]\n"
			<< "\tStupsCompiler -liveness [filename.pas]\n";
		return 0;
	}
	//Commandline switches
	if( std::string("-compile") == argv[1] ){
		compile( argv[2] );
		return 0;
	}

	if( std::string("-liveness") == argv[1] ){
		compile( argv[2], true);
		return 0;
	}

	std::cout << "Error: Unknown command line input.\n Valid input is:\n"
		<< "\tStupsCompiler -compile [filename.pas]\n"
		<< "\tStupsCompiler -liveness [filename.pas]\n";
	return 0;
#endif
}
