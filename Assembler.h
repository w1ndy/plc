#ifndef __ASSEMBLER_H__
#define __ASSEMBLER_H__

#include <vector>
#include <stack>
#include <iostream>
#include <string>
#include <fstream>

#include "SymbolManager.h"
#include "Automaton.h"

#define OPR_RET		0x00
#define OPR_NOT		0x01
#define OPR_ADD		0x02
#define OPR_SUB		0x03
#define OPR_MUL		0x04
#define OPR_DIV		0x05
#define OPR_ODD		0x06
#define OPR_NOP		0x07
#define OPR_CMPE	0x08
#define OPR_CMPNE	0x09
#define OPR_CMPL	0x0a
#define OPR_CMPGE	0x0b
#define OPR_CMPG 	0x0c
#define OPR_CMPLE	0x0d
#define OPR_WR		0x0e
#define OPR_NL		0x0f
#define OPR_RD		0x10



#define FIND_ID(name, level, p) \
	int level; \
	SymbolManager::entry *p = symbol_manager.findEntry(name, level); \
	if(p == NULL) { \
		std::cout << "[error: identifier " << name << " not found in current context]" << std::endl; \
		return false; \
	} \


struct inst_t {
	enum inst_type_t { LIT, LOD, STO, CAL, INT, JMP, JPC, OPR } type;
	int l, a;

	inst_t(inst_type_t _t, int _l, int _a) : type(_t), l(_l), a(_a) {};
};

struct proc_body_backfill_t
{
	int jmp_inst;
	int proc_body;

	proc_body_backfill_t(int j, int p) : jmp_inst(j), proc_body(p) {}
};

class Assembler
{
public:
	static std::vector<inst_t> code;
	static std::stack<int> proc_int_backfill;
	static std::stack<proc_body_backfill_t> proc_body_backfill;

	static std::stack<int> if_not_backfill;
	static std::stack<int> while_entry_backfill;
	static std::stack<int> while_not_backfill;

	static SymbolManager symbol_manager;

	static void attachSemanticAction(Automaton &a);

	static inline size_t length() {
		return code.size();
	}

	static inline void write(std::string const &filename) {
		std::ofstream fout(filename);
		int address = 0;
		for(auto const &i : code) {
			printf("0x%08X: ", address++);
			std::cout << inst_type_to_string(i.type) << " " << i.l << " " << i.a << std::endl;
			fout << inst_type_to_string(i.type) << " " << i.l << " " << i.a << std::endl;
		}
	}

	static const char *inst_type_to_string(inst_t::inst_type_t t) {
		switch(t) {
			case inst_t::LIT: return "LIT";
			case inst_t::LOD: return "LOD";
			case inst_t::STO: return "STO";
			case inst_t::CAL: return "CAL";
			case inst_t::INT: return "INT";
			case inst_t::JMP: return "JMP";
			case inst_t::JPC: return "JPC";
			case inst_t::OPR: return "OPR";
		}
		return "";
	}

	static inline void insert(inst_t::inst_type_t t, int l, int a) {
		code.push_back(inst_t(t, l, a));
	}

	static inline bool call_proc(std::string const &name) {
		FIND_ID(name, level, p);
		if(p->type != SymbolManager::entry::PROCEDURE) {
			std::cout << "[error: identifier " << name << " is not a procedure]" << std::endl;
			return false;
		}
		insert(inst_t::CAL, level, p->address);
		std::cout << "[call procedure " << p->name << "]" << std::endl;
		return true;
	}

	static inline bool load_id(std::string const &name) {
		FIND_ID(name, level, p);
		if(p->type == SymbolManager::entry::CONSTANT) {
			insert(inst_t::LIT, 0, p->value);
			std::cout << "[move constant " << p->name << " to stack]" << std::endl;
		} else if(p->type == SymbolManager::entry::VARIABLE) {
			insert(inst_t::LOD, level, p->address);
			std::cout << "[move variable " << p->name << " to stack]" << std::endl;
		} else {
			std::cout << "[error: identifier " << p->name << " is a procedure]" << std::endl;
			return false;
		}
		return true;
	}

	static inline bool store_id(std::string const &name) {
		FIND_ID(name, level, p);
		if(p->type == SymbolManager::entry::VARIABLE) {
			insert(inst_t::STO, level, p->address);
			std::cout << "[store to " << p->name << "]" << std::endl;
			return true;
		}
			std::cout << "[error: identifier " << p->name << " is not a variable]" << std::endl;
		return false;
	}

	static inline bool load_number(int number) {
		insert(inst_t::LIT, 0, number);
		std::cout << "[move number " << number << " to stack]" << std::endl;
		return true;
	}

	static inline bool read_id(std::string const &name) {
		FIND_ID(name, level, p);
		if(p->type != SymbolManager::entry::VARIABLE) {
			std::cout << "[error: identifier " << name << " is not a variable]" << std::endl;
			return false;
		}
		insert(inst_t::OPR, 0, OPR_RD);
		insert(inst_t::STO, level, p->address);
		std::cout << "[read variable " << p->name << "]" << std::endl;
		return true;
	}

	static inline bool write_id(std::string const &name) {
		FIND_ID(name, level, p);
		if(p->type != SymbolManager::entry::VARIABLE) {
			std::cout << "[error: identifier " << name << " is not a variable]" << std::endl;
			return false;
		}
		Assembler::insert(inst_t::LOD, level, p->address);
		Assembler::insert(inst_t::OPR, 0, OPR_WR);
		std::cout << "[write variable " << p->name << "]" << std::endl;
		return true;
	}

	static inline bool insert_opr(int id) {
		switch(id) {
			case OPR_NOP: break;
			default:
				insert(inst_t::OPR, 0, id);
		}
		std::cout << "[perform arithmetic operation " << id << "]" << std::endl;
		return true;
	}
};

#endif // __ASSEMBLER_H__