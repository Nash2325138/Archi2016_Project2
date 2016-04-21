#include "ctrUnit.h"
/*
	bool RegDst;
	bool Jump;
	bool Branch;
	bool MemRead;
	bool MemtoReg;
	bool ALUop[2];
	bool MemWrite;
	bool ALUSrc;
	bool RegWrite;
*/
CtrUnit::CtrUnit(unsigned char opcode)
{
	if(opcode==0x0){
		RegDst = RegWrite = ALUop[1] = true;
		ALUSrc = MemtoReg = MemRead = MemWrite = Branch = ALUop[0] = false;
	}
	else if(opcode==0x23){
		
	}
}