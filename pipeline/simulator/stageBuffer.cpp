#include "stageBuffer.h"

void IF_ID_Buffer::put(unsigned int inst_in, CtrUnit *control_in, unsigned int PC_p_4)
{
	/*
	unsigned int inst;
	unsigned int PC_puls_4;
	CtrUnit *control;
	
	unsigned char opcode;		//warning: unsigned char has 8 bits
	unsigned char rs;
	unsigned char rt;
	unsigned char shamt;
	unsigned char funct;
	unsigned char rd;

	unsigned short immediate;
	unsigned int address;
	*/
	inst = inst_in;
	this->control = control_in;
	PC_puls_4 = PC_p_4;

	opcode = (unsigned char) (inst >> 26);		//warning: unsigned char has 8 bits
	rs =  (unsigned char) ( (inst >> 21) & 0x1f );
	rt = (unsigned char) ( (inst >> 16) & 0x1f );
	shamt = (unsigned char) ( (inst >> 6) & 0x1f );
	funct = (unsigned char) (inst & 0x3f);
	rd = (unsigned char) ( (inst >> 11) & 0x1f );

	immediate = inst & 0xffff;
	address = inst & 0x3ffffff;
}

void ID_EX_Buffer::put(void)
{
	
}

void ID_EX_Buffer::clearToNop(void)
{
	/*	
	unsigned int inst;
	unsigned int PC_puls_4;
	CtrUnit *control;

	// for a lazy reason, the following 3 argument are forwarded to use project 1 code 
	unsigned char opcode;
	unsigned char funct;
	unsigned char shamt;

	int rs_data;
	int rt_data;
	int extented_immediate;

	unsigned char rs;
	unsigned char rt;
	unsigned char rd;
	*/
	inst = 0;
	PC_puls_4 = 0;
	control->change(0);

	opcode = funct = shamt = 0;

	rs_data = rt_data = extented_immediate = 0;

	rs = rt = rd = 0;
}