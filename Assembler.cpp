#include "Assembler.h"
#include "parser/lalrparser.h"
using namespace std;

vector<inst_t> Assembler::code;
stack<int> Assembler::proc_int_backfill;
stack<proc_body_backfill_t> Assembler::proc_body_backfill;
stack<int> Assembler::if_not_backfill;
stack<int> Assembler::while_entry_backfill;
stack<int> Assembler::while_not_backfill;

SymbolManager Assembler::symbol_manager;

void Assembler::attachSemanticAction(Automaton &a)
{
	a.attachCallback({ Grammar::CONSTDEF },
		ACFUNC(r, v) {
			cout << "[def new const " << v[0]->name << "=" << v[2]->desc << "]" << endl;
			symbol_manager.insertConstant(v[0]->name, v[2]->desc);
			return nullptr;
		});

	a.attachCallback({ Grammar::OPTVARID, Grammar::VAR },
		ACFUNC(r, v) {
			cout << "[def new variable " << v[1]->name << "]" << endl;
			symbol_manager.insertVariable(v[1]->name);
			return nullptr;
		});

	a.attachCallback({ Grammar::PROCH },
		ACFUNC(r, v) {
			cout << "[def new procedure " << v[1]->name << " begin]" << endl;
			symbol_manager.insertProcedure(v[1]->name, length());

			insert(inst_t::INT, 0, 0);
			proc_int_backfill.push(length() - 1);
			insert(inst_t::JMP, 0, 0);
			proc_body_backfill.push(proc_body_backfill_t(
				length() - 1, length()));

			return nullptr;
		});

	a.attachCallback({ Grammar::PROC },
		ACFUNC(r, v) {
			cout << "[def new procedure end]" << endl;
			insert(inst_t::OPR, 0, 0);

			// backfill procedure INT instruction
			code[proc_int_backfill.top()].a = symbol_manager.countLayerSize();
			proc_int_backfill.pop();

			// backfill procedure JMP instruction
			code[proc_body_backfill.top().jmp_inst].a =
				proc_body_backfill.top().proc_body;
			proc_body_backfill.pop();

			// update upper layer procedure backfill info
			proc_body_backfill_t pbb(proc_body_backfill.top().jmp_inst, length());
			proc_body_backfill.pop();
			proc_body_backfill.push(pbb);

			symbol_manager.ascend();
			return nullptr;
		});

	a.attachCallback({ Grammar::CALL },
		ACFUNC(r, v) {
			call_proc(v[1]->name);
			return nullptr;
		});

	a.attachCallback({ Grammar::READ, Grammar::OPTRDID },
		ACFUNC(r, v) {
			if(r == Grammar::READ)
				read_id(v[3]->name);
			else
				read_id(v[1]->name);
			return nullptr;
		});

	a.attachCallback({ Grammar::WRITE, Grammar::OPTWRID },
		ACFUNC(r, v) {
			if(r == Grammar::WRITE) {
				if(v[3] != nullptr)
					write_id(v[3]->name);
				else
					insert_opr(OPR_WR);
				insert_opr(OPR_NL);
			}
			else {
				if(v[1] != nullptr)
					write_id(v[1]->name);
				else
					insert_opr(OPR_WR);
			}
			return nullptr;
		});

	a.attachCallback({ Grammar::IDFACTOR, Grammar::NUMFACTOR },
		ACFUNC(r, v) {
			if(r == Grammar::IDFACTOR) {
				load_id(v[0]->name);
			} else {
				load_number(v[0]->desc);
			}
			return nullptr;
		});

	a.attachCallback({ Grammar::AOP, Grammar::SOP, Grammar::MOP, Grammar::DOP, Grammar::NEXPROP, Grammar::PEXPROP, Grammar::EQOP, Grammar::LOP, Grammar::LEOP, Grammar::GOP, Grammar::GEOP, Grammar::NEOP },
		ACFUNC(r, v) {
			int desc;
			switch(r) {
				case Grammar::AOP: desc = OPR_ADD; break;
				case Grammar::SOP: desc = OPR_SUB; break;
				case Grammar::MOP: desc = OPR_MUL; break;
				case Grammar::DOP: desc = OPR_DIV; break;
				case Grammar::EQOP: desc = OPR_CMPE; break;
				case Grammar::LOP: desc = OPR_CMPL; break;
				case Grammar::LEOP: desc = OPR_CMPLE; break;
				case Grammar::GOP: desc = OPR_CMPG; break;
				case Grammar::GEOP: desc = OPR_CMPGE; break;
				case Grammar::NEOP: desc = OPR_CMPNE; break;
				case Grammar::NEXPROP: 
					load_number(0);
					desc = OPR_SUB;
					break;
				case Grammar::PEXPROP:
					desc = OPR_NOP;
					break;
			}
			return symbol_t::construct(SymbolType_Operator, desc, "");
		});

	a.attachCallback({ Grammar::OPTFACTOR, Grammar::OPTEXPRITEM, Grammar::FEXPRITEM, Grammar::EXPR, Grammar::EXPRITEM },
		ACFUNC(r, v) {
			if(v[0] != nullptr)
				insert_opr(v[0]->desc);
			return (r == Grammar::EXPR || r == Grammar::EXPRITEM) ? nullptr :
				symbol_t::construct(SymbolType_Operator, v[2]->desc, "");
		});

	a.attachCallback({ Grammar::ASSERTION, Grammar::RASSERTION },
		ACFUNC(r, v) {
			if(r == Grammar::ASSERTION)
				insert_opr(v[1]->desc);
			else
				insert_opr(OPR_ODD);
		});

	a.attachCallback({ Grammar::ASSIGN },
		ACFUNC(r, v) {
			store_id(v[0]->name);
		});

	a.attachCallback({ Grammar::CONDHEADER },
		ACFUNC(r, v) {
			insert(inst_t::JPC, 0, 0);
			if_not_backfill.push(length() - 1);
		});

	a.attachCallback({ Grammar::CONDITION },
		ACFUNC(r, v) {
			code[if_not_backfill.top()].a = length();
			if_not_backfill.pop();
		});

	a.attachCallback({ Grammar::WHILEFLAG },
		ACFUNC(r, v) {
			while_entry_backfill.push(length());
		});

	a.attachCallback({ Grammar::WHILEHEADER },
		ACFUNC(r, v) {
			insert(inst_t::JPC, 0, 0);
			while_not_backfill.push(length() - 1);
		});

	a.attachCallback({ Grammar::WHILE },
		ACFUNC(r, v) {
			insert(inst_t::JMP, 0, while_entry_backfill.top());
			while_entry_backfill.pop();
			code[while_not_backfill.top()].a = length();
			while_not_backfill.pop();
		});
}