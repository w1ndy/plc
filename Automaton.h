#ifndef __AUTOMATON_H__
#define __AUTOMATON_H__

#include <string>
#include <stack>
#include <vector>
#include <map>
#include <functional>
#include <initializer_list>

#include "parser/lalrparser.h"
#include "SymbolParser.h"

#define ACFUNC(rule, v) [](Grammar::ruletype_t (rule), std::vector<symbol_t::ptr> const &(v)) -> symbol_t::ptr

struct automaton_state_t {
	int state;
	std::string symbol_name;
	symbol_t::ptr symbol;

	automaton_state_t(int s, std::string name, symbol_t::ptr sym = nullptr) : state(s), symbol_name(name), symbol(sym) {}
};

typedef std::function<symbol_t::ptr (Grammar::ruletype_t rule, std::vector<symbol_t::ptr> const &)> rule_callback_func_t;

class Automaton
{
public:
	enum automaton_result_t {
		result_succeeded,
		result_waiting,
		result_error_occured,
		result_error_bad_symbol,
		result_error_syntax,
		result_error_unreachable
	};

public:
	Automaton();
	virtual ~Automaton();

	automaton_result_t feed(symbol_t::ptr sym);
	void reset();

	void attachCallback(std::initializer_list<Grammar::ruletype_t> match_cond, rule_callback_func_t func);

private:
	const std::string _symbolToString(symbol_t::ptr sym);

private:
	std::map<std::string, int> 			lookup;
	std::stack<automaton_state_t> 		states;
	rule_callback_func_t				*callbacks;

	bool error_flag, succeed_flag;
};

#endif //__AUTOMATON_H__