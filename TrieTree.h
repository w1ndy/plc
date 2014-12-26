#ifndef __TRIETREE_H__
#define __TRIETREE_H__

class TrieTree
{
public:
	TrieTree();
	virtual ~TrieTree();

public:
	const static unsigned int TrieTreeTableSize = 36;
	
	typedef unsigned int node_type_t;

	struct node_t {
		node_t *	next[TrieTreeTableSize];
		node_type_t node_type;
	};

	enum Type {
		Type_None 		= 0x00,
		Type_Keyword 	= 0x01,
		Type_Identifier = 0x02,

		Type_Maximum	= 0x03,
		Type_ForceInt	= 0xffffffff
	};

public:
	inline static node_type_t 	makeNodeType(Type type, unsigned int subtype) {
		return (static_cast<int>(type) << 24) + (subtype & 0xffffff);
	}

	inline static Type 			extractType(node_type_t type) {
		return (type < (static_cast<int>(Type_Maximum) << 24))
			? static_cast<Type>((type & 0xff000000) >> 24)
			: Type_None;
	}

	inline static unsigned int 	extractSubtype(node_type_t type) {
		return type & 0xffffff;
	}

	inline static bool 			validateCharacter(char c) {
		return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
	}

	inline static unsigned int 	mapCharacterToIndex(char c) {
		return (c <= '9') 
			? static_cast<unsigned>(c - '0') 
			: static_cast<unsigned>(c - 'a' + 10);
	}

public:
	node_t *	match(const char *name);
	node_t *	insert(const char *name, Type type, unsigned int subtype);

private:
	node_t *	createNode();
	void		freeNode(node_t *&node);

private:
	node_t *root;

private:
};

#endif // __TRIETREE_H__