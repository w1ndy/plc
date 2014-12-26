#ifndef __KEYWORD_H__
#define __KEYWORD_H__

#include "TrieTree.h"

#define KEYWORD_LIST	\
	DEF_KEYWORD(Const, 		"const"),		\
	DEF_KEYWORD(Var,		"var"),			\
	DEF_KEYWORD(Procedure,	"procedure"),	\
	DEF_KEYWORD(Begin,		"begin"),		\
	DEF_KEYWORD(End,		"end"),			\
	DEF_KEYWORD(Odd,		"odd"),			\
	DEF_KEYWORD(If,			"if"),			\
	DEF_KEYWORD(Then,		"then"),		\
	DEF_KEYWORD(Else,		"else"),		\
	DEF_KEYWORD(Call,		"call"),		\
	DEF_KEYWORD(While,		"while"),		\
	DEF_KEYWORD(Do, 		"do"),			\
	DEF_KEYWORD(Read,		"read"),		\
	DEF_KEYWORD(Write,		"write")

class Keyword
{
public:
	#define DEF_KEYWORD(identifier, name) 	identifier
		enum type_t { KEYWORD_LIST };
	#undef DEF_KEYWORD

	static const char *names[];

public:
	inline static const char *translate(type_t type) {
		return names[static_cast<unsigned>(type)];
	}

	static void insert(TrieTree &tree) {
		#define DEF_KEYWORD(identifier, name) \
				tree.insert(name, TrieTree::Type_Keyword, identifier)
			KEYWORD_LIST;
		#undef DEF_KEYWORD
	}
};

#define DEF_KEYWORD(identifier, name)	name
	const char *Keyword::names[] = { KEYWORD_LIST };
#undef DEF_KEYWORD

#endif // __KEYWORD_H__