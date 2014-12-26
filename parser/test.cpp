#include <iostream>
#include <map>
#include <stack>
#include "lalrparser.h"
#include "rules.h"
using namespace std;
using namespace Grammar;

class Automaton
{
public:
	map<string, int> lookup;
	stack<string> proc;
	stack<int> state;

	Automaton() : state({ 0 }), proc({ "$" }) {
		for(int i = 0; i < sym_table.size(); i++) {
			lookup[sym_table[i]] = i;
		}
	};

	void feed(const string &terminal) {
		cout << state.top() << " -> ";
		string T;
		if(terminal == "id") T = ID_PREFIX;
		else if(terminal == "num") T = NUM_PREFIX;
		else if (terminal == "$") T = EOF_PREFIX;
		else T = string(TERM_PREFIX) + terminal;
		if(lookup.find(T) == lookup.end()) {
			cout << "symbol " << terminal << " not found." << endl;
			return ;
		}

		while (true) {
			action_t act = goto_table[state.top()][lookup[T]];
			if (act.action == action_t::error) {
				cout << "ERROR ";
				break;
			}
			else if (act.action == action_t::shift) {
				cout << "SHIFT ";
				state.push(act.ptr);
				proc.push(T);
				break;
			}
			else if (act.action == action_t::reduce) {
				cout << "REDUCE ";
				for (auto &rs : rules[act.ptr].rhs) {
					proc.pop();
					state.pop();
				}
				proc.push(string(SYM_PREFIX) + rules[act.ptr].lhs);
				state.push(goto_table[state.top()][lookup[string(SYM_PREFIX) + rules[act.ptr].lhs]].ptr);
			}
			else if (act.action == action_t::succeed) {
				cout << "SUCCESS ";
				break;
			}
		}
		cout << "-> " << state.top() << endl;
	}
};

int main()
{
	Automaton a;
	a.feed("const"); a.feed("id"); a.feed("="); a.feed("num"); a.feed(";");
	a.feed("procedure"); a.feed("id"); a.feed(";");
	a.feed("var"); a.feed("id"); a.feed(";");
	a.feed("begin");
		a.feed("id"); a.feed(":="); a.feed("id"); a.feed("+"); a.feed("id"); a.feed("*"); a.feed("num");
	a.feed("end"); a.feed(";");
	a.feed("begin");
		a.feed("call"); a.feed("id"); a.feed(";");
		a.feed("write"); a.feed("("); a.feed("id"); a.feed(","); a.feed("id"); a.feed(")");
	a.feed("end"); a.feed("."); a.feed("$");
	return 0;
}