#ifndef __SYMBOLMANAGER_H__
#define __SYMBOLMANAGER_H__

#include <string>

class SymbolManager
{
public:
	struct entry {
		std::string name;
		enum { PROCEDURE, VARIABLE, CONSTANT } type;
		int value;
		int address;

		entry *parent;
		entry *next;
		entry *child;
	};

public:
	SymbolManager();
	virtual ~SymbolManager();

	bool insertProcedure(std::string const &name, int address);
	bool insertVariable(std::string const &name);
	bool insertConstant(std::string const &name, int value);
	void ascend();

	entry *findEntry(std::string const &name, int &level);
	int  countLayerSize();
	void print();

private:
	entry *root;
	entry *cur;

	bool _isDuplicated(std::string const &name);
	void _freeEntry(entry *e);

	inline entry *_getLayerHead() {
		return (cur->parent) ? cur->parent->child : root;
	}

	void _printEntry(entry *e, int tab);
};

#endif // __SYMBOLMANAGER_H__