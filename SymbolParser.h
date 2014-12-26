#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include <type_traits>
#include <memory>
#include <string>
#include <stack>
#include <cstring>

#include "TrieTree.h"
#include "SourceCode.h"

#define GETSYM_NO_ECHO

#define SYMBOLTYPE_LIST \
	DEF_SYMBOLTYPE(SymbolType_Unknown,		"UNKNOWN") 	SEPARATOR \
	DEF_SYMBOLTYPE(SymbolType_Keyword,		"KEYWORD") 	SEPARATOR \
	DEF_SYMBOLTYPE(SymbolType_Identifier,	"IDENT")	SEPARATOR \
	DEF_SYMBOLTYPE(SymbolType_Number,		"NUMBER")	SEPARATOR \
	DEF_SYMBOLTYPE(SymbolType_Operator,		"OP")		SEPARATOR \
	DEF_SYMBOLTYPE(SymbolType_Echo,			"ECHO")

#define DEF_SYMBOLTYPE(identifier, name) identifier
#define SEPARATOR	,
	enum symbol_type_t { SYMBOLTYPE_LIST };
#undef DEF_SYMBOLTYPE
#undef SEPARATOR

class symbol_t
{
private:
	symbol_t() : type(SymbolType_Unknown) {};

public:
	typedef std::shared_ptr<symbol_t> ptr;
	static const unsigned int MaximumSymbolNameLength = 15;

	symbol_type_t 	type;
	char 			name[MaximumSymbolNameLength + 1];
	int 			desc;

public:	
	static ptr construct(symbol_type_t type, int desc, const char *name) {
		ptr ret(new symbol_t);
		ret->type = type;
		ret->desc = desc;
		strncpy(ret->name, name, MaximumSymbolNameLength);
		return ret;
	}
};

class SymbolParser
{
public:
	SymbolParser(SourceCode &c);
	virtual ~SymbolParser();

	symbol_t::ptr 	nextSymbol();

	inline void 	undoSymbol(symbol_t::ptr sym) {
		buffer.push(sym);
	}

public:
	inline SymbolParser &operator>>(symbol_t::ptr &sym) {
		sym = nextSymbol();
		if(sym == nullptr && buffer.empty())
			parserState = State_Finished;
		return *this;
	}

	inline SymbolParser &operator<<(symbol_t::ptr sym) {
		buffer.push(sym);
		if(parserState == State_Finished)
			parserState = State_BeginMatch;
		return *this;
	}

	operator bool() const {
		return (parserState != State_Finished);
	}

public:
	static const char *translateSymbol(symbol_type_t type) {
		switch(type) {
			#define DEF_SYMBOLTYPE(identifier, name) \
					case identifier: return name
			#define SEPARATOR	;
				SYMBOLTYPE_LIST;
			#undef DEF_SYMBOLTYPE
			#undef SEPARATOR
		}
		return "UNKNOWN";
	}

private:
	enum State {
		State_BeginMatch,
		State_MatchIdentifier,
		State_MatchNumber,
		State_MatchOperator,
		State_MatchEchoLetter,
		State_SymbolFound,
		State_Finished
	} parserState;

	TrieTree 		dict;
	SourceCode &	code;

	std::stack<symbol_t::ptr> buffer;

private:
	static const std::string EchoCharset;
	static const std::string OperatorCharset;
	static const char *	Operators[];
	static const size_t	OperatorsCount;
};

#endif // __SYMBOL_H__