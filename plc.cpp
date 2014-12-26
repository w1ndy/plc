#include <iostream>

#include "SymbolParser.h"
#include "SourceCode.h"
#include "SyntaxError.h"
#include "Automaton.h"
#include "SymbolManager.h"
#include "Assembler.h"

#include "parser/lalrparser.h"

using namespace std;

int main(int argc, const char *argv[])
{
	SourceCode code;
	if(argc != 2) {
		cout << "Usage: " << argv[0] << " source_file " << endl;
		return 1;
	}
	if(!code.open(argv[1])) {
		cout << "Failed to open \"" << argv[1] << "\"" << endl;
		return -1;
	}

	// todo: backfill proc body.
	SymbolParser parser(code);
	symbol_t::ptr sym;
	Automaton a;
	bool done = false;
	while(true) {
		try {
			Assembler::attachSemanticAction(a);

			Assembler::insert(inst_t::INT, 0, 0);
			Assembler::proc_int_backfill.push(Assembler::length() - 1);
			Assembler::insert(inst_t::JMP, 0, 0);
			Assembler::proc_body_backfill.push(proc_body_backfill_t(
				Assembler::length() - 1, Assembler::length()));

			while(!done) {
				parser >> sym;
				switch(a.feed(sym))
				{
					case Automaton::result_succeeded:
						Assembler::code.push_back(inst_t(inst_t::OPR, 0, 0));
						Assembler::code[0].a = Assembler::symbol_manager.countLayerSize();
						Assembler::code[1].a = Assembler::proc_body_backfill.top().proc_body;
						done = true;
						break;
					case Automaton::result_error_occured:
					case Automaton::result_error_bad_symbol:
					case Automaton::result_error_syntax:
					case Automaton::result_error_unreachable:
						cout << "Automaton error!" << endl;
						done = true;
						break;
				}
			}
			break;
		} catch(SyntaxError &e) {
			cout << e.what() << endl;
		}
	}
	cout << endl;
	Assembler::symbol_manager.print();
	cout << endl;
	Assembler::write("output.asm");
	cout << endl;
	return 0;
}