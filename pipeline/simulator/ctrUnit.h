#ifndef CTRUNIT_H
#define CTRUNIT_H
#include <cstdio>

class CtrUnit{
public:
	bool used;

	bool Jump;

	bool RegDst;
	bool ALUop[2];
	bool ALUSrc;

	bool Branch;
	bool MemRead;
	bool MemWrite;
	
	bool MemtoReg;
	bool RegWrite;
	CtrUnit(void);
	void change(unsigned int inst);
	void print_all(void);
	void flush(void);
};

#endif