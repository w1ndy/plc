#include "SymbolManager.h"

#include <iostream>
using namespace std;

SymbolManager::SymbolManager()
{
	root = new entry;
	root->parent = NULL;
	root->child = NULL;
	root->next = NULL;
	cur = root;
}

SymbolManager::~SymbolManager()
{
	_freeEntry(root);
}

void SymbolManager::_freeEntry(SymbolManager::entry *e)
{
	if(e == NULL) return ;
	_freeEntry(e->next);
	_freeEntry(e->child);
	delete e;
}

bool SymbolManager::insertProcedure(string const &name, int address)
{
	if(_isDuplicated(name)) return false;

	SymbolManager::entry *e = new entry;
	e->name = name;
	e->type = entry::PROCEDURE;
	e->address = address;
	e->parent = cur->parent;
	e->child = new entry;
	e->child->parent = e;
	e->child->next = NULL;
	e->child->child = NULL;

	SymbolManager::entry *h = _getLayerHead();
	e->next = h->next;
	h->next = e;

	cur = e->child;

	return true;
}

bool SymbolManager::insertVariable(string const &name)
{
	if(_isDuplicated(name)) return false;

	SymbolManager::entry *e = new entry;
	e->name = name;
	e->type = entry::VARIABLE;
	e->parent = cur->parent;
	e->next = NULL;
	e->child = NULL;

	if(cur->type != entry::VARIABLE)
		e->address = 3;
	else
		e->address = cur->address + 1;
	cur->next = e;
	cur = e;

	return true;
}

bool SymbolManager::insertConstant(string const &name, int value)
{
	if(_isDuplicated(name)) return false;

	SymbolManager::entry *e = new entry;
	e->name = name;
	e->type = entry::CONSTANT;
	e->value = value;
	e->parent = cur->parent;
	e->child = NULL;

	SymbolManager::entry *h = _getLayerHead();
	e->next = h->next;
	h->next = e;

	cur = e;

	return true;
}

void SymbolManager::ascend()
{
	if(cur->parent) cur = cur->parent;
}

SymbolManager::entry *SymbolManager::findEntry(std::string const &name, int &level)
{
	//cout << "dbg: matching " << name << endl;
	SymbolManager::entry *p = _getLayerHead()->next, *pp;
	if(!p && !(_getLayerHead()->parent)) return NULL;
	pp = _getLayerHead()->parent;
	int l = 0;
	while(p || pp) {
		while(p) {
			//cout << "dbg: attempting " << p->name << endl;
			if(p->name == name) {
				level = l;
				//print();
				//cout << "dbg: matched." << endl;
				return p;
			}
			p = p->next;
		}
		p = pp;
		if(p) pp = p->parent;
		l++;
	}
	return NULL;
}

int SymbolManager::countLayerSize()
{
	int s = 3;
	SymbolManager::entry *p = _getLayerHead()->next;
	while(p) {
		if(p->type == entry::VARIABLE) s++;
		p = p->next;
	}
	return s;
}

void SymbolManager::print()
{
	_printEntry(root->next, 0);
}

void SymbolManager::_printEntry(SymbolManager::entry *e, int tab)
{
	if(!e) return;
	for(int i = 0; i < tab; i++)
		cout << "  ";
	switch(e->type)
	{
		case entry::PROCEDURE:
			cout << "procedure " << e->name << " address " << e->address << " level " << tab << endl;
			_printEntry(e->child->next, tab + 1);
			break;
		case entry::VARIABLE:
			cout << "variable " << e->name << " address " << e->address << " level " << tab << endl;
			break;
		case entry::CONSTANT:
			cout << "constant " << e->name << " value " << e->value << " level " << tab << endl;
			break;
	}
	_printEntry(e->next, tab);
}

bool SymbolManager::_isDuplicated(string const &name)
{
	int l = 255;
	findEntry(name, l);
	return l == 0;
}