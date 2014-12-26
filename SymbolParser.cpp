#include "SymbolParser.h"
#include "Keyword.h"
#include "SyntaxError.h"

#include <cctype>
#include <cstdio>

using namespace std;

const std::string 	SymbolParser::EchoCharset 		= "();,.";
const std::string 	SymbolParser::OperatorCharset 	= "<>+-=*/:#";

const char *	SymbolParser::Operators[] 		= {":=", "<>", "<=", ">=", "=", "<", ">", "+", "-", "*", "/" , "#"};
const size_t	SymbolParser::OperatorsCount 	= std::extent<decltype(SymbolParser::Operators)>::value;

SymbolParser::SymbolParser(SourceCode &c) : code(c)
{
	Keyword::insert(dict);
}

SymbolParser::~SymbolParser()
{
}


symbol_t::ptr SymbolParser::nextSymbol()
{
	symbol_t::ptr ret;
	TrieTree::node_t *node_ptr;
	stack<symbol_t::ptr> ops;

	if(!buffer.empty()) {
		ret = buffer.top();
		buffer.pop();
		return ret;
	}

	if(parserState == State_Finished)
		return symbol_t::construct(SymbolType_Echo, '$', "eof");

	char cache[80];
	int pc = 0;
	parserState = State_BeginMatch;
	while(parserState != State_SymbolFound) {
		switch(parserState)
		{
		case State_BeginMatch:
			code >> cache[pc];
			if(EchoCharset.find(cache[pc]) != string::npos)
			{
				parserState = State_MatchEchoLetter;
			} else if(cache[pc] == 0) {
				parserState = State_Finished;
				return symbol_t::construct(SymbolType_Echo, '$', "eof");
			} else if(isalpha(cache[pc])) {
				parserState = State_MatchIdentifier;
			} else if(isdigit(cache[pc])) {
				parserState = State_MatchNumber;
			} else if(OperatorCharset.find(cache[pc]) != string::npos) {
				parserState = State_MatchOperator;
			} else {
				break;
			}
			pc++;
			break;
		case State_MatchEchoLetter:
			ret = symbol_t::construct(SymbolType_Echo, cache[pc - 1], "");
			parserState = State_SymbolFound;
			break;
		case State_MatchIdentifier:
			code >> cache[pc];
			if(!isalpha(cache[pc]) && !isdigit(cache[pc])) {
				code << cache[pc];
				cache[pc] = '\0';
				node_ptr = dict.match(cache);
				if(TrieTree::extractType(node_ptr->node_type) == TrieTree::Type_Keyword) {
					ret = symbol_t::construct(
						SymbolType_Keyword,
						TrieTree::extractSubtype(node_ptr->node_type),
						Keyword::translate(static_cast<Keyword::type_t>(
							TrieTree::extractSubtype(node_ptr->node_type))));
				} else {
					ret = symbol_t::construct(
						SymbolType_Identifier,
						TrieTree::extractSubtype(node_ptr->node_type),
						cache);
				}
				parserState = State_SymbolFound;
			} else {
				pc++;
			}
			break;
		case State_MatchNumber:
			code >> cache[pc];
			if(!isdigit(cache[pc])) {
				code << cache[pc];
				cache[pc] = '\0';
				ret = symbol_t::construct(
					SymbolType_Number,
					atoi(cache),
					"");
				parserState = State_SymbolFound;
			} else {
				pc++;
			}
			break;
		case State_MatchOperator:
			code >> cache[pc];
			if(OperatorCharset.find(cache[pc]) == string::npos) {
				code << cache[pc];
				cache[pc] = '\0';
				for(int i = 0, j; i < pc; ) {
					for(j = 0; j < OperatorsCount; j++) {
						if(strncmp(cache + i, Operators[j], strlen(Operators[j])) == 0)
							break;
					}
					if(j >= OperatorsCount) {
						throw SyntaxError(code, pc - i, "Unknown operator %c", *(cache + i));
					} else {
						ops.push(symbol_t::construct(
							SymbolType_Operator,
							j,
							Operators[j]));
						i += strlen(Operators[j]);
					}
				}
				while(ops.size() > 1) {
					buffer.push(ops.top());
					ops.pop();
				}
				ret = ops.top();
				ops.pop();
				parserState = State_SymbolFound;
			} else {
				pc++;
			}
			break;
		}
	}

	#ifndef GETSYM_NO_ECHO
	if(ret->type == SymbolType_Echo) {
		printf("%c", ret->desc);
	} else {
		printf("<%s,%s,%d>", translateSymbol(ret->type), ret->name, ret->desc);
	}
	#endif
	
	return ret;
}