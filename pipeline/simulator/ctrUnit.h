#ifndef CTRUNIT_H
#define CTRUNIT_H

class CtrUnit{
public:
	bool RegDst;
	bool Jump;
	bool Branch;
	bool MemRead;
	bool MemtoReg;
	bool ALUop[2];
	bool MemWrite;
	bool ALUSrc;
	bool RegWrite;

	CtrUnit(unsigned char opcode);
};

#endif