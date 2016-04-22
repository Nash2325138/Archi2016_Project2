#ifndef CTRUNIT_H
#define CTRUNIT_H

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
	CtrUnit(unsigned int inst);
	void change(unsigned int inst);
};

#endif