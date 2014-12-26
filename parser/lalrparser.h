#ifndef __LALRPARSER_H__
#define __LALRPARSER_H__

#include <iostream>
#include <string>
#include <vector>

namespace Grammar {

struct rule_t {
	std::string lhs;
	std::vector<std::string> rhs;
};

struct action_t {
	enum fsm_action_t {
		shift, reduce, error, succeed
	};

	fsm_action_t	action;
	int				ptr;

	action_t() : action(shift), ptr(0) {}
	action_t(fsm_action_t a, int p) : action(a), ptr(p) {}

	bool operator==(action_t const &a) const {
		return action == a.action && ptr == a.ptr;
	}

	bool operator!=(action_t const &a) const {
		return !(*this == a);
	}
};

#define SYM_PREFIX		"S"
#define TERM_PREFIX		"T"
#define ID_PREFIX		"i"
#define NUM_PREFIX		"n"
#define EOF_PREFIX		"$"

const char SYM_C = SYM_PREFIX[0];
const char TERM_C = TERM_PREFIX[0];
const char ID_C = ID_PREFIX[0];
const char NUM_C = NUM_PREFIX[0];
const char EOF_C = EOF_PREFIX[0];

#define SYM(x)		SYM_PREFIX x,
#define TERM(x)		TERM_PREFIX x,
#define ID			ID_PREFIX,
#define NUM			NUM_PREFIX,

//DEF_RULE(SS, "S'", { SYM("S") }), \
//DEF_RULE(S1, "S", { SYM("L") TERM("=") SYM("R") }), \
//DEF_RULE(S2, "S", { SYM("R") }), \
//DEF_RULE(L1, "L", { TERM("*") SYM("R") }), \
//DEF_RULE(L2, "L", { ID }), \
//DEF_RULE(R, "R", { SYM("L") })
#define RULE_LIST \
	DEF_RULE(PL0,			"pl0",			{ SYM("prog")												}), \
	DEF_RULE(PROG,			"prog",			{ SYM("subprog") TERM(".")									}), \
	DEF_RULE(SUBPROG,		"subprog",		{ SYM("const") SYM("var") SYM("proc") SYM("sentence")		}), \
	DEF_RULE(ECONST,		"const",		{															}), \
	DEF_RULE(CONST,			"const",		{ TERM("const") SYM("constdef") SYM("optconstdef") TERM(";")}), \
	DEF_RULE(CONSTDEF,		"constdef",		{ ID TERM("=") NUM 											}), \
	DEF_RULE(EOPTCONSTDEF,	"optconstdef",	{															}), \
	DEF_RULE(OPTCONSTDEF,	"optconstdef",	{ TERM(",") SYM("constdef") SYM("optconstdef")				}), \
	DEF_RULE(EVAR,			"var",			{															}), \
	DEF_RULE(VAR,			"var",			{ TERM("var") ID SYM("optid") TERM(";")						}), \
	DEF_RULE(EOPTVARID,		"optid",		{															}), \
	DEF_RULE(OPTVARID,		"optid",		{ TERM(",") ID SYM("optid")									}), \
	DEF_RULE(EPROC,			"proc",			{															}), \
	DEF_RULE(PROC,			"proc",			{ SYM("procheader") SYM("subprog") TERM(";") SYM("proc")	}), \
	DEF_RULE(PROCH,			"procheader",	{ TERM("procedure") ID TERM(";")							}), \
	DEF_RULE(ESENTENCE,		"sentence",		{															}), \
	DEF_RULE(ASSIGNST,		"sentence",		{ SYM("assign")												}), \
	DEF_RULE(CONDST,		"sentence",		{ SYM("condition")											}), \
	DEF_RULE(WHILEST,		"sentence",		{ SYM("while")												}), \
	DEF_RULE(CALLST,		"sentence",		{ SYM("call")												}), \
	DEF_RULE(READST,		"sentence",		{ SYM("read")												}), \
	DEF_RULE(WRITEST,		"sentence",		{ SYM("write")												}), \
	DEF_RULE(COMPST,		"sentence",		{ SYM("complex")											}), \
	DEF_RULE(ASSIGN,		"assign",		{ ID TERM(":=") SYM("expr")									}), \
	DEF_RULE(COMPLEX,		"complex",		{ TERM("begin") SYM("multist") TERM("end")					}), \
	DEF_RULE(MULTIST,		"multist",		{ SYM("sentence") SYM("optmultist")							}), \
	DEF_RULE(EOPTMULTIST,	"optmultist",	{															}), \
	DEF_RULE(OPTMULTIST,	"optmultist",	{ TERM(";") SYM("sentence") SYM("optmultist")				}), \
	DEF_RULE(ASSERTION,		"assert",		{ SYM("expr") SYM("relop") SYM("expr")						}), \
	DEF_RULE(RASSERTION,	"assert",		{ TERM("odd") SYM("expr")									}), \
	DEF_RULE(EXPR,			"expr",			{ SYM("optexpritem") SYM("expritem")						}), \
	DEF_RULE(NEXPROP,		"exprop",		{ TERM("-")													}), \
	DEF_RULE(PEXPROP,		"exprop",		{ TERM("+")													}), \
	DEF_RULE(EXPRITEM,		"expritem",		{ SYM("optfactor") SYM("factor")							}), \
	DEF_RULE(IDFACTOR,		"factor",		{ ID														}), \
	DEF_RULE(NUMFACTOR,		"factor",		{ NUM														}), \
	DEF_RULE(EXPRFACTOR,	"factor",		{ TERM("(") SYM("expr") TERM(")")							}), \
	DEF_RULE(EOPTFACTOR,	"optfactor",	{															}), \
	DEF_RULE(OPTFACTOR,		"optfactor",	{ SYM("optfactor") SYM("factor") SYM("mdop")				}), \
	DEF_RULE(MOP,			"mdop",			{ TERM("*")													}), \
	DEF_RULE(DOP,			"mdop",			{ TERM("/")													}), \
	DEF_RULE(EOPTEXPRITEM,	"optexpritem",	{ 															}), \
	DEF_RULE(OPTEXPRITEM,	"optexpritem",	{ SYM("optexpritem") SYM("expritem") SYM("asop")			}), \
	DEF_RULE(FEXPRITEM,		"optexpritem",	{ SYM("exprop") SYM("expritem") SYM("asop")					}), \
	DEF_RULE(AOP,			"asop",			{ TERM("+")													}), \
	DEF_RULE(SOP,			"asop",			{ TERM("-")													}), \
	DEF_RULE(EQOP,			"relop",		{ TERM("=")													}), \
	DEF_RULE(LOP,			"relop",		{ TERM("<")													}), \
	DEF_RULE(LEOP,			"relop",		{ TERM("<=")												}), \
	DEF_RULE(GOP,			"relop",		{ TERM(">")													}), \
	DEF_RULE(GEOP,			"relop",		{ TERM(">=")												}), \
	DEF_RULE(NEOP,			"relop",		{ TERM("#")													}), \
	DEF_RULE(CONDHEADER,	"condheader",	{ TERM("if") SYM("assert") TERM("then") 					}), \
	DEF_RULE(CONDITION,		"condition",	{ SYM("condheader") SYM("sentence")							}), \
	DEF_RULE(CALL,			"call",			{ TERM("call") ID											}), \
	DEF_RULE(WHILEFLAG,		"whileflag",	{ TERM("while")												}), \
	DEF_RULE(WHILEHEADER,	"whileheader",	{ SYM("whileflag") SYM("assert") TERM("do")					}), \
	DEF_RULE(WHILE,			"while",		{ SYM("whileheader") SYM("sentence")						}), \
	DEF_RULE(READ,			"read",			{ TERM("read") TERM("(") SYM("optreadid") ID TERM(")")		}), \
	DEF_RULE(EOPTRDID,		"optreadid",	{															}), \
	DEF_RULE(OPTRDID,		"optreadid",	{ SYM("optreadid") ID TERM(",")								}), \
	DEF_RULE(WRITE,			"write",		{ TERM("write") TERM("(") SYM("optwriteid") SYM("expr") TERM(")")}), \
	DEF_RULE(EOPTWRID,		"optwriteid",	{															}), \
	DEF_RULE(OPTWRID,		"optwriteid",	{ SYM("optwriteid") SYM("expr") TERM(",")							})	\

#define DEF_RULE(id, lhs, rhs) { lhs, rhs }
const rule_t rules[] = { RULE_LIST };
#undef DEF_RULE

#define DEF_RULE(id, lhs, rhs) id
enum ruletype_t { RULE_LIST };
#undef DEF_RULE


static const std::string allupper(const std::string &s)
{
	std::string ret = s;
	for (int i = 0; i < ret.length(); i++)
		ret[i] = toupper(ret[i]);
	return ret;
}

static const std::string get_output_symbol_without_prefix(std::string const &s)
{
	switch (s[0])
	{
	case 'S':
		return allupper(s.substr(1));
	case 'T':
		return s.substr(1);
	case 'i':
		return "id";
	case 'n':
		return "num";
	case 'e':
		return "e";
	case '$':
		return "$";
	default:
		return "[UNKNOWN]";
	}
}

static void print_table(std::vector<std::string> const &table)
{
	std::cout << "Symbol Table:" << std::endl;
	for (auto const &s : table) {
		std::cout << "  " << get_output_symbol_without_prefix(s) << std::endl;
	}
}

static void print_rule(rule_t const &r, int marked_position = -1)
{
	int i; 
	std::cout << allupper(r.lhs) << " -> ";
	for (i = 0; i < r.rhs.size(); i++) {
		if (marked_position == i)
			std::cout << "^ ";
		std::cout << get_output_symbol_without_prefix(r.rhs[i]) << " ";
	}
	if (marked_position == i) std::cout << '^';
}

static void print_all_rules()
{
	std::cout << "Rules: " << std::endl;
	for (auto const &r : rules) {
		std::cout << "  ";
		print_rule(r);
		std::cout << std::endl;
	}
}

}; // namespace Grammar

#endif // __LALRPARSER_H__