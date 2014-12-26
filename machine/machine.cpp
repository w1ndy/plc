#include <iostream>
#include <cstring>
#include <cstdio>
#include <vector>
using namespace std;

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

struct inst_t {
	enum inst_type_t { LIT, LOD, STO, CAL, INT, JMP, JPC, OPR } type;
	int l, a;

	inst_t() {};
	inst_t(inst_type_t _t, int _l, int _a) : type(_t), l(_l), a(_a) {};
};

vector<inst_t> 	inst;
int 			stack[1024];

int rPC, rBP, rSP;

bool read_asm(const char *f)
{
	char inst_name[32];
	inst_t i;
	FILE *fp = fopen(f, "r");

	if(fp == NULL)
		return false;
	while(!feof(fp)) {
		fscanf(fp, "%s %d %d", inst_name, &(i.l), &(i.a));
		if(0 == strcmp(inst_name, "LIT"))
			i.type = inst_t::LIT;
		else if(0 == strcmp(inst_name, "LOD"))
			i.type = inst_t::LOD;
		else if(0 == strcmp(inst_name, "STO"))
			i.type = inst_t::STO;
		else if(0 == strcmp(inst_name, "CAL"))
			i.type = inst_t::CAL;
		else if(0 == strcmp(inst_name, "INT"))
			i.type = inst_t::INT;
		else if(0 == strcmp(inst_name, "JMP"))
			i.type = inst_t::JMP;
		else if(0 == strcmp(inst_name, "JPC"))
			i.type = inst_t::JPC;
		else if(0 == strcmp(inst_name, "OPR"))
			i.type = inst_t::OPR;
		else {
			cout << "unknown " << inst_name << endl;
			return false;
		}
		inst.push_back(i);
	}
	return true;
}

int main(int argc, char *argv[])
{
	if(argc != 2) {
		cout << "Usage: " << argv[0] << " assembly_file" << endl;
		return 1;
	}

	if(!read_asm(argv[1])) {
		cout << "read file error" << endl;
		return -1;
	}

	rPC = 0;
	rBP = 0;
	rSP = -1;

	stack[++rSP] = -1;
	stack[++rSP] = -1;
	stack[++rSP] = -1;


	while(1) {
		//printf("dbg: rPC = %d, rBP = %d, rSP = %d\n", rPC, rBP, rSP);
		int ptr, i;
		inst_t rInst = inst[rPC++];

		switch(rInst.type) {
			case inst_t::INT:
				for(i = 0; i < rInst.a - 3; i++)
					stack[++rSP] = 0;
				break;
			case inst_t::LIT:
				stack[++rSP] = rInst.a;
				break;
			case inst_t::LOD:
				ptr = rBP;
				for(i = 0; i < rInst.l; i++)
					ptr = stack[ptr];
				stack[++rSP] = stack[ptr + rInst.a];
				break;
			case inst_t::STO:
				ptr = rBP;
				for(i = 0; i < rInst.l; i++)
					ptr = stack[ptr];
				stack[ptr + rInst.a] = stack[rSP];
				break;
			case inst_t::CAL:
				ptr = rBP;
				for(i = 0; i < rInst.l; i++)
					ptr = stack[ptr];
				stack[++rSP] = ptr;
				stack[++rSP] = rBP;
				stack[++rSP] = rPC;
				rBP = rSP - 2;
				rPC = rInst.a;
				break;
			case inst_t::JMP:
				rPC = rInst.a;
				break;
			case inst_t::JPC:
				if(stack[rSP] == 0)
					rPC = rInst.a;
				break;
			case inst_t::OPR:
				switch(rInst.a) {
					case OPR_RET:
						if(rBP == 0) {
							cout << "simulation completed." << endl;
							return 0;
						}
						rSP = rBP - 1;
						rPC = stack[rBP + 2];
						rBP = stack[rBP + 1];
						break;
					case OPR_NOT:
						stack[rSP] = !stack[rSP];
						break;
					case OPR_ADD:
						stack[--rSP] += stack[rSP + 1];
						break;
					case OPR_SUB:
						stack[--rSP] -= stack[rSP + 1];
						break;
					case OPR_MUL:
						stack[--rSP] *= stack[rSP + 1];
						break;
					case OPR_DIV:
						stack[--rSP] /= stack[rSP + 1];
						break;
					case OPR_ODD:
						stack[++rSP] = stack[rSP - 1] % 2;
						break;
					case OPR_NOP:
						break;
					case OPR_CMPE:
						stack[++rSP] = (stack[rSP - 1] == stack[rSP - 2]);
						break;
					case OPR_CMPNE:
						stack[++rSP] = (stack[rSP - 1] != stack[rSP - 2]);
						break;
					case OPR_CMPL:
						stack[++rSP] = (stack[rSP - 2] < stack[rSP - 1]);
						break;
					case OPR_CMPGE:
						stack[++rSP] = (stack[rSP - 2] >= stack[rSP - 1]);
						break;
					case OPR_CMPG:
						stack[++rSP] = (stack[rSP - 2] > stack[rSP - 1]);
						break;
					case OPR_CMPLE:
						stack[++rSP] = (stack[rSP - 2] <= stack[rSP - 1]);
						break;
					case OPR_WR:
						printf("%d", stack[rSP]);
						break;
					case OPR_NL:
						printf("\n");
						break;
					case OPR_RD:
						scanf("%d", &ptr);
						stack[++rSP] = ptr;
						break;
				}
				break;
		}
	}

	return 0;
}