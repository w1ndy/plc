#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <map>
#include <set>
#include <functional>
#include <sstream>
using namespace std;

#include "lalrparser.h"
using namespace Grammar;

const int rules_count = extent<decltype(rules)>::value;

/// GENERATE HEADER FILE
#define LITERAL(x)			#x
#define EXPAND_LITERAL(x)	LITERAL(x)

#define INCLUDE(x)																							\
	"#include <" EXPAND_LITERAL(x) ">\n"
#define INCLUDE_LOCAL(x)																					\
	"#include \"" EXPAND_LITERAL(x) "\"\n"

#define EXPORT_FILE_PREFIX																					\
	"// This is an automatically generated header file by LALR parser.\n"									\
	"#ifndef _RULES_H_\n"																					\
	"#define _RULES_H_\n"																					\
	"\n"

#define EXPORT_FILE_INCLUDES																				\
	INCLUDE(string)																							\
	INCLUDE(vector)																							\
	INCLUDE(type_traits)																					\
	INCLUDE_LOCAL(lalrparser.h)																				\
	"\n"

#define EXPORT_FILE_RULE_LIST_PREFIX																		\
	"#define RULE_LIST "

#define EXPAND_RULE_LIST(...) #__VA_ARGS__
#define EXPAND_RULE_LIST_TWICE(x) x

#define EXPORT_FILE_RULE_LIST																				\
	EXPORT_FILE_RULE_LIST_PREFIX																			\
	RULE_LIST																		\
	"\n\n"

#define EXPORT_FILE_NAMESPACE_BEGIN																			\
	"namespace Grammar {\n\n"

#define EXPORT_FILE_NAMESPACE_END																			\
	"\n}; // namespace Grammar\n\n"

#define EXPORT_FILE_RULE_DEF																				\
	"#define DEF_RULE(id, lhs, rhs) { lhs, rhs }\n"															\
	"rule_t rules[] = { RULE_LIST };\n"																		\
	"#undef DEF_RULE\n"																						\
	"\n"																									\
	"#define DEF_RULE(id, lhs, rhs) id\n"																	\
	"enum ruletype_t { RULE_LIST };\n"																		\
	"#undef DEF_RULE\n"																						\
	"\n"																									\
	"const int rules_count = extent<decltype(rules)>::value;\n"												\
	"\n"

#define EXPORT_FILE_SYMBOL_TABLE_PREFIX																		\
	"const std::vector<std::string> sym_table = {\n"

#define EXPORT_FILE_SYMBOL_TABLE_SUFFIX																		\
	"}; // sym_table\n\n"

#define EXPORT_FILE_GOTO_TABLE_PREFIX																		\
	"static const std::vector<action_t> goto_table[] = {\n"

#define EXPORT_FILE_GOTO_TABLE_SUFFIX																		\
	"}; // goto_table\n\n"

#define EXPORT_FILE_SUFFIX																					\
	"#endif // _RULES_H_\n"

#define EXPORT_FILE(kernel)																					\
	string(																									\
		EXPORT_FILE_PREFIX																					\
		EXPORT_FILE_INCLUDES																				\
		EXPORT_FILE_NAMESPACE_BEGIN																			\
		EXPORT_FILE_SYMBOL_TABLE_PREFIX																		\
		) + sym_table_to_string() + string(																	\
		EXPORT_FILE_SYMBOL_TABLE_SUFFIX																		\
		EXPORT_FILE_GOTO_TABLE_PREFIX																		\
		) + generate_goto_table(kernel) + string(															\
		EXPORT_FILE_GOTO_TABLE_SUFFIX																		\
		EXPORT_FILE_NAMESPACE_END																			\
		EXPORT_FILE_SUFFIX																					\
		)

#undef DEF_RULE
//////////////////////////


struct closure_elem_t {
	int rule;
	int position;
	mutable set<string> look_aheads;

	closure_elem_t(int r, int p, set<string> la_set = set<string>()) : rule(r), position(p) 
	{
		if (la_set.size() != 0)
			look_aheads = la_set;
	}

	bool operator==(const closure_elem_t &e) const
	{
		return rule == e.rule && position == e.position;
	}

	closure_elem_t &operator=(closure_elem_t const &r)
	{
		rule = r.rule;
		position = r.position;
		look_aheads = r.look_aheads;
	}

	bool operator<(closure_elem_t const &r) const
	{
		return (rule == r.rule) ? (position < r.position) : (rule < r.rule);
	}
};

typedef map<string, action_t> transition_cond_t;

struct closure_t {
	set<closure_elem_t>		elem;
	transition_cond_t		transit;

	closure_t() { reset(); }

	void reset()
	{
		elem.clear();
		transit.clear();
		id = 0;
	}

	bool operator==(closure_t const &c) const
	{
		return id == c.id;
	}

	bool operator<(closure_t const &c) const {
		return id < c.id;
	}

	bool empty() const
	{
		return elem.empty();
	}

	void link_to(closure_t const &c, string const &input)
	{
		transition_cond_t::iterator it;
		if ((it = transit.find(input)) != transit.end() && it->second.ptr != c.id) {
			cerr << "closure_t::link_to(): shift-shift conflicts" << endl;
			return;
		}

		transit[input] = action_t(action_t::shift, c.id);
	}

	void insert_and_expand(int rule, int position, bool use_LR1 = false, int prev_rule = -1, int prev_position = -1)
	{
		if(rule == prev_rule && position == prev_position)
			return ;
		if (use_LR1)
			_insert_LR1(rule, position, prev_rule, prev_position);
		else
			_insert_LR0(rule, position);
		if (position < rules[rule].rhs.size() && rules[rule].rhs[position][0] == SYM_C) {
			string match_lhs = rules[rule].rhs[position].substr(1);
			for (int i = 0; i < rules_count; i++) {
				if (rules[i].lhs == match_lhs)
					insert_and_expand(i, 0, use_LR1, rule, position);
			}
		}
	}

	void expand_all_by_LR1()
	{
		bool done;
		do {
			done = true;
			for (auto const &e : elem) {
				if (e.position < rules[e.rule].rhs.size() && rules[e.rule].rhs[e.position][0] == SYM_C) {
					string match_lhs = rules[e.rule].rhs[e.position].substr(1);
					for (int i = 0; i < rules_count; i++) {
						if (rules[i].lhs == match_lhs) {
							int before_size = elem.size();
							insert_and_expand(i, 0, true, e.rule, e.position);
							if (elem.size() != before_size)
								done = false;
						}
					}
				}
			}
		} while (!done);
	}

	//bool insert_and_expand(int rule, int position, bool use_LR1 = false, const closure_elem_t *prev = NULL)
	//{
	//	if (position >= rules[rule].rhs.size()) {
	//		if (position == rules[rule].rhs.size()) {
	//			if (use_LR1)
	//				_insert_LR1(rule, position, prev);
	//			else
	//				_insert_LR0(rule, position);
	//		}
	//		return false;
	//	}

	//	const closure_elem_t *inserted;
	//	do {
	//		inserted = (use_LR1) ? _insert_LR1(rule, position, prev) : _insert_LR0(rule, position);
	//	} while (_expand(use_LR1, inserted) && ++position <= rules[rule].rhs.size());

	//	return position < rules[rule].rhs.size();
	//}

	int id;

private:
	set<closure_elem_t>::iterator _find_closure_elem(int rule, int position)
	{
		for (auto it = elem.begin(); it != elem.end(); ++it) {
			if (it->rule == rule && it->position == position)
				return it;
		}
		return elem.end();
	}

	//inline bool _expand(bool LR1, const closure_elem_t *item)
	//{
	//	bool adv = false;
	//	if (item->position < rules[item->rule].rhs.size()) {
	//		if (rules[item->rule].rhs[item->position][0] == SYM_C) {
	//			string next = rules[item->rule].rhs[item->position].substr(1);
	//			for (int i = 0; i < rules_count; i++) {
	//				if ((item->rule != i || item->position != 0) && rules[i].lhs == next && !insert_and_expand(i, 0, LR1, item))
	//					adv = true;
	//			}
	//		}
	//	}
	//	return adv;
	//}

	bool _find_first(int rule, int position, set<string> &sym)
	{
		if (position >= rules[rule].rhs.size())
			return false;

		bool advance;
		do {
			advance = false;
			if (rules[rule].rhs[position][0] == SYM_C) {
				string match = rules[rule].rhs[position].substr(1);
				for (int i = 0; i < rules_count; i++) {
					if ((i != rule || position != 0) && rules[i].lhs == match && !_find_first(i, 0, sym))
						advance = true;
				}
			}
			else {
				sym.insert(rules[rule].rhs[position]);
			}
		} while (advance && ++position < rules[rule].rhs.size());

		return (position < rules[rule].rhs.size());
	}

	void _insert_LR0(int rule, int position)
	{
		elem.insert(closure_elem_t(rule, position));
	}

	void _insert_LR1(int rule, int position, int prev_rule, int prev_position)
	{
		auto it = (prev_rule == -1) ? elem.end() : _find_closure_elem(prev_rule, prev_position);

		set<string> first_set;
		if (it == elem.end())
			first_set.insert("@");
		else if (!_find_first(it->rule, it->position + 1, first_set)) {
			first_set.insert(it->look_aheads.begin(), it->look_aheads.end());
		}

		auto existing = _find_closure_elem(rule, position);
		if (existing != elem.end())
			existing->look_aheads.insert(first_set.begin(), first_set.end());
		else
			elem.insert(closure_elem_t(rule, position, first_set));
	}
};

struct propagate_info_t {
	int closure_index;
	int rule;
	int position;

	propagate_info_t(int i, int r, int p) : closure_index(i), rule(r), position(p) {}
};

typedef map<closure_elem_t, vector<propagate_info_t>> propagate_info_map_t;
typedef vector<closure_t>				closure_set_t;
typedef map<closure_elem_t, set<int>> 	closure_map_t;

set<string>	sym_table;
vector<propagate_info_map_t> propagations;

void make_tables()
{
	sym_table.insert(ID_PREFIX);
	sym_table.insert(NUM_PREFIX);
	sym_table.insert(EOF_PREFIX);

	for (auto &r : rules) {
		sym_table.insert(string(SYM_PREFIX) + r.lhs);
		for (auto &s : r.rhs) {
			sym_table.insert(s);
		}
	}
}

void print_closures(closure_set_t const &cset, bool LR1 = false)
{
	int count = 0;
	for (auto const &c : cset) {
		cout << "CLOSURE No." << count++ << endl;
		for (auto const &e : c.elem) {
			cout << "  ";
			print_rule(rules[e.rule], e.position);
			if (LR1) {
				for (auto const &s : e.look_aheads) {
					cout << ", " << get_output_symbol_without_prefix(s);
				}
			}
			cout << endl;
		}
		cout << endl;
	}
}

closure_set_t::const_iterator is_closure_duplicated(closure_t const &c, closure_set_t const &cset)
{
	return find_if(cset.cbegin(), cset.cend(), [&](closure_t const &r) {
		//vector<closure_elem_t> intersection;
		//set_intersection(c.elem.begin(), c.elem.end(), r.elem.begin(), r.elem.end(), back_inserter(intersection));
		//if (intersection.size() == c.elem.size())
		//	return true;
		//return false;
		return c.elem == r.elem;
	});
}

void kernelify(closure_set_t &kernel)
{
	for (auto &c : kernel) {
		for (auto iter = c.elem.cbegin(); iter != c.elem.cend(); ) {
			if (iter->position == 0 && iter->rule != 0)
				iter = c.elem.erase(iter);
			else
				++iter;
		}
	}
}

closure_t find_goto_set(closure_t const &c, string const &sym)
{
	closure_t ret;
	int prev_size;
	do {
		prev_size = ret.elem.size();
		for (auto e : c.elem) {
			if (e.position != rules[e.rule].rhs.size() && rules[e.rule].rhs[e.position] == sym) {
				ret.insert_and_expand(e.rule, e.position + 1);
				if(prev_size != ret.elem.size())
					break;
			}
		}
	} while (prev_size != ret.elem.size());
	return ret;
}

void find_LR0_kernels(closure_set_t &kernel, closure_map_t &kernel_map)
{
	kernel_map.clear();
	kernel.clear();

	closure_t c;
	closure_set_t::const_iterator it;
	c.id = 0;
	c.insert_and_expand(0, 0);
	kernel.push_back(c);

	bool done = false;
	while (!done) {
		done = true;
		for (int i = 0; i < kernel.size(); i++) {
			for (auto const &s : sym_table) {
				c = find_goto_set(kernel[i], s);
				if (!c.empty()) {
					if ((it = is_closure_duplicated(c, kernel)) == kernel.cend()) {
						c.id = kernel.size();
						kernel.push_back(c);
						kernel[i].link_to(c, s);
						done = false;
					}
					else {
						kernel[i].link_to(*it, s);
					}
				}
			}
		}
	}

	kernelify(kernel);

	for (auto const &c : kernel) {
		for (auto const &e : c.elem) {
			kernel_map[e].insert(c.id);
		}
	}
}

void assign_lookaheads(closure_set_t &kernel, closure_map_t &kernel_map)
{
	closure_t tmp;
	map<closure_elem_t, vector<propagate_info_t>> pi;

	for (auto &c : kernel) {

		pi.clear();
		for (auto &e : c.elem) {
			if (e.rule == 0 && e.position == 0)
				e.look_aheads.insert(EOF_PREFIX);

			tmp.reset();
			tmp.insert_and_expand(e.rule, e.position, true);

			for (auto &extended_e : tmp.elem) {
				if (extended_e.position >= rules[extended_e.rule].rhs.size())
					continue;

				for (auto &s : extended_e.look_aheads) {
					if (c.transit.find(rules[extended_e.rule].rhs[extended_e.position]) == c.transit.end()) {
						cerr << "assign_lookaheads(): failed to locate transition" << endl;
						continue;
					}
					action_t action = c.transit[rules[extended_e.rule].rhs[extended_e.position]];

					if (s != "@") {
						if (action.ptr == 9)
							bool propagate_to_closure_9 = true;
						if (action.ptr == 54)
							bool propagate_to_closure_54 = true;
						if (action.ptr == 35)
							bool propagate_to_closure_35 = true;
						if (action.ptr == 15)
							bool propagate_to_closure_15 = true;
						for (auto &succ_e : kernel[action.ptr].elem)
							succ_e.look_aheads.insert(s);
					}
					else {
						if (action.ptr == 9)
							bool propagate_to_closure_9 = true;
						if (action.ptr == 54)
							bool propagate_to_closure_54 = true;
						if (action.ptr == 35)
							bool propagate_to_closure_35 = true;
						if (action.ptr == 15)
							bool propagate_to_closure_15 = true;
						for (auto &succ_e : kernel[action.ptr].elem)
							pi[e].push_back(propagate_info_t(kernel[action.ptr].id, succ_e.rule, succ_e.position));
					}
					/*for (auto &succ_e : kernel[action.ptr].elem) {
						if (succ_e.rule == extended_e.rule && succ_e.position == extended_e.position + 1) {
							if(s != "@")
								succ_e.look_aheads.insert(s);
							else
								pi[e].push_back(propagate_info_t(kernel[action.ptr].id, succ_e.rule, succ_e.position));
							if (kernel[action.ptr].id == 83) {
								int propagate_to_state_83 = 1;
							}
							break;
						}
					}*/
				}
			}
		}
		propagations.push_back(pi);
	}
}

void propagate_lookaheads(closure_set_t &kernel, closure_map_t &kernel_map)
{
	bool done;
	propagate_info_map_t::iterator it;
	do {
		done = true;
		for (auto &c : kernel) {
			for (auto &e : c.elem) {
				if ((it = propagations[c.id].find(e)) != propagations[c.id].end()) {
					for (auto &p : it->second) {
						for (auto &succ_e : kernel[p.closure_index].elem) {
							if (succ_e.rule == p.rule && succ_e.position == p.position) {
								int before_size = succ_e.look_aheads.size();
								succ_e.look_aheads.insert(e.look_aheads.begin(), e.look_aheads.end());
								if (before_size != succ_e.look_aheads.size())
									done = false;
								break;
							}
						}
					}
				}
			}
		}
	} while (!done);
}

void expand_kernel(closure_set_t &kernel)
{
	for (auto &c : kernel) {
		c.expand_all_by_LR1();
	}
}

const string sym_table_to_string()
{
	ostringstream os;
	for (auto &s : sym_table) {
		os << "\t\"" << s << "\",\n";
	}
	return os.str();
}

const string fsm_action_to_string(action_t::fsm_action_t action)
{
	switch (action)
	{
	case action_t::shift:
		return "action_t::shift";
	case action_t::reduce:
		return "action_t::reduce";
	case action_t::error:
		return "action_t::error";
	case action_t::succeed:
		return "action_t::succeed";
	}
	cerr << "fsm_action_to_string(): undefined action" << endl;
	return "";
}

const string actions_to_string(map<string, action_t> &table)
{
	ostringstream os;

	os << "{";
	for (auto const &s : sym_table) {
		os << "{" << fsm_action_to_string(table[s].action) << "," << table[s].ptr << "}, ";
	}
	os << "}";

	return os.str();
}

const string generate_goto_table(closure_set_t const &cset)
{
	ostringstream os;
	map<string, action_t> table;
	map<string, action_t>::iterator it;

	for (auto const &c : cset) {
		table.clear();
		for (auto const &e : c.elem) {
			if (e.position == rules[e.rule].rhs.size()) {
				for (auto const &s : e.look_aheads) {
					if (e.rule == 0 && s[0] == EOF_C) {
						table[s] = action_t(action_t::succeed, e.rule);
						continue;
					}
					if ((it = table.find(s)) != table.end() && (it->second.action != action_t::shift || it->second.ptr != e.rule)) {
						cerr << "generate_goto_table(): reduce conflicts found" << endl;
						return "";
					}
					else {
						table[s] = action_t(action_t::reduce, e.rule);
					}
				}
			}
		}
		for (auto const &t : c.transit) {
			if ((it = table.find(t.first)) != table.end() && (it->second != t.second)) {
				cerr << "generate_goto_table(): shift conflicts found" << endl;
				return "";
			}
			else {
				table[t.first] = t.second;
			}
		}
		for (auto const &s : sym_table) {
			if (table.find(s) == table.end())
				table[s] = action_t(action_t::error, 0);
		}
		os << "\t" << actions_to_string(table) << "," << endl;
	}

	return os.str();
}

int main()
{
	cerr << "LALR Parser started." << endl << endl;

	cerr << "Learned " << rules_count << " rules." << endl;
	cerr << "Building symbol table...";
	make_tables();
	cerr << sym_table.size() << " symbol(s) discovered." << endl;

	freopen("closures.txt", "w", stdout);
	closure_set_t kernel;
	closure_map_t kernel_map;

	cerr << "Finding LR0 kernels..." << endl;
	find_LR0_kernels(kernel, kernel_map);
	cerr << "Assigning initial look aheads..." << endl;
	assign_lookaheads(kernel, kernel_map);
	cerr << "Propagating look aheads..." << endl;
	propagate_lookaheads(kernel, kernel_map);
	cerr << "Expanding kernel..." << endl;
	expand_kernel(kernel);
	print_closures(kernel, true);

	cerr << "Writing rules.h..." << endl;
	freopen("rules.h", "w", stdout);
	cout << EXPORT_FILE(kernel) << endl;

	cerr << "done." << endl;
	fclose(stdout);
	return 0;
}

