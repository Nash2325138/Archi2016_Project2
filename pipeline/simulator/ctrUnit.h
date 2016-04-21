#ifndef CTRUNIT_H
#define CTRUNIT_H

class CtrUnit{
public:

	bool Jump;

	bool RegDst;
	bool ALUop[2];
	bool ALUSrc;

	bool Branch;
	bool MemRead;
	bool MemWrite;
	
	bool MemtoReg;
	bool RegWrite;
	CtrUnit(unsigned int inst);
};

#endif