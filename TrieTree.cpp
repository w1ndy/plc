#include "TrieTree.h"

#include <cstdlib>
#include <cstring>

TrieTree::TrieTree()
{
	root = createNode();
}

TrieTree::~TrieTree()
{
	freeNode(root);
}

TrieTree::node_t *TrieTree::match(const char *name)
{
	TrieTree::node_t *p = root;
	int index;

	while(*name) {
		if(validateCharacter(*name)) {
			index = mapCharacterToIndex(*name);
			if(p->next[index] == NULL)
				p->next[index] = createNode();
			p = p->next[index];
		}
		name++;
	}

	return p;
}

TrieTree::node_t *TrieTree::insert(const char *name, Type type, unsigned int subtype)
{
	TrieTree::node_t *p = match(name);
	
	if(extractType(p->node_type) != Type_None)
		return NULL;
	p->node_type = makeNodeType(type, subtype);
	return p;
}

TrieTree::node_t *TrieTree::createNode()
{
	TrieTree::node_t *ret = new TrieTree::node_t;
	memset(ret->next, 0, sizeof(TrieTree::node_t *) * TrieTree::TrieTreeTableSize);
	ret->node_type = makeNodeType(Type_None, 0);
	return ret;
}

void TrieTree::freeNode(TrieTree::node_t *&node)
{
	if(node == NULL)
		return ;
	for(int i = 0; i < TrieTree::TrieTreeTableSize; i++) {
		if(node->next[i])
			freeNode(node->next[i]);
	}
	delete node;
	node = NULL;
}
