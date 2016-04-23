#ifndef STAGEBUFFER_H
#define STAGEBUFFER_H

#include "./ctrUnit.h"

class IF_ID_Buffer
{
public:
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

	void put(unsigned int inst_in, CtrUnit *control_in, unsigned int PC_p_4);
};

class ID_EX_Buffer
{
public:
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

	unsigned char rt;
	unsigned char rd;

	void put(void);		// needs implementation
};

class EX_MEM_Buffer
{
public:
	unsigned int inst;
	unsigned int PC_result;
	CtrUnit *control;

	bool ALU_zero;
	int ALU_result;
	int rt_data;

	unsigned char opcode;

	unsigned char write_destination;
};

class MEM_WB_Buffer
{
public:
	int ALU_result;
	int memory_result;
};

#endif