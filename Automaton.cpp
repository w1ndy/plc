#include "Automaton.h"
#include "parser/rules.h"
using namespace std;
using namespace Grammar;

#include <iostream>
#include <algorithm>

Automaton::Automaton()
{
	callbacks = new rule_callback_func_t[extent<decltype(rules)>::value];
	for(int i = 0; i < sym_table.size(); i++)
		lookup[sym_table[i]] = i;
	for(int i = 0; i < extent<decltype(rules)>::value; i++) 
		callbacks[i] = ACFUNC(r, v) {
			return nullptr;
		};

	reset();
}

Automaton::~Automaton()
{
	delete[] callbacks;
}

Automaton::automaton_result_t Automaton::feed(symbol_t::ptr sym)
{
	if(error_flag)
		return Automaton::result_error_occured;
	if(succeed_flag)
		return Automaton::result_succeeded;

	string T = _symbolToString(sym);
	if(lookup.find(T) == lookup.end()) {
		cout << "Automaton::feed(): symbol " << T << " not found." << endl;
		error_flag = true;
		return Automaton::result_error_bad_symbol;
	}

	action_t act;
	string new_symbol;
	vector<symbol_t::ptr> values;
	Grammar::ruletype_t reduced_rule;

	while(true) {
		act = goto_table[states.top().state][lookup[T]];

		switch(act.action)
		{
		case action_t::error:
			cout << "Automaton::feed(): syntax error at state " << states.top().state << " when trying to match " << T << endl;
			error_flag = true;
			return Automaton::result_error_syntax;
		case action_t::shift:
			states.push(automaton_state_t(act.ptr, T, sym));
			return Automaton::result_waiting;
		case action_t::reduce:
			reduced_rule = static_cast<Grammar::ruletype_t>(act.ptr);
			values.clear();
			for (auto &rs : rules[act.ptr].rhs) {
				values.push_back(states.top().symbol);
				states.pop();
			}
			new_symbol = string(SYM_PREFIX) + rules[act.ptr].lhs;
			if(lookup.find(new_symbol) == lookup.end() || (act = goto_table[states.top().state][lookup[new_symbol]]).action != action_t::shift) {
				cout << "Automaton::feed(): reduction error at state " << states.top().state << " when trying to match " << new_symbol << endl;
				error_flag = true;
				return result_error_syntax;
			}
			reverse(values.begin(), values.end());
			states.push(automaton_state_t(act.ptr, new_symbol, callbacks[reduced_rule](reduced_rule, values)));
			//states.push(automaton_state_t(act.ptr, new_symbol));
			break;
		case action_t::succeed:
			cout << "Automaton::feed(): accepted." << endl;
			succeed_flag = true;
			return Automaton::result_succeeded;
		}
	}

	error_flag = true;
	return Automaton::result_error_unreachable;
}

void Automaton::attachCallback(std::initializer_list<Grammar::ruletype_t> match_cond, rule_callback_func_t func)
{
	for(auto &type : match_cond)
		callbacks[(size_t)type] = func;
}

const string Automaton::_symbolToString(symbol_t::ptr sym)
{
	switch(sym->type)
	{
	case SymbolType_Identifier:
		return ID_PREFIX;
	case SymbolType_Number:
		return NUM_PREFIX;
	case SymbolType_Keyword:
	case SymbolType_Operator:
		return string(TERM_PREFIX) + sym->name;
	case SymbolType_Echo:
		return (sym->desc == '$') ? "$" : (string(TERM_PREFIX) + (char)sym->desc);
	}
	return "unknown";
}

void Automaton::reset()
{
	error_flag = succeed_flag = false;

	while(!states.empty())
		states.pop();
	states.push(automaton_state_t(0, "$"));
}