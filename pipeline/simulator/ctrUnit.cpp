#include "ctrUnit.h"
/*
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
*/
CtrUnit::CtrUnit(void)
{
	used = Jump = RegDst = ALUop[0] = ALUop[1] = ALUSrc = Branch = MemRead = MemWrite = MemtoReg = RegWrite = false;
}

void CtrUnit::change(unsigned int inst)
{
	unsigned char opcode = (unsigned char) (inst >> 26);		//warning: unsigned char has 8 bits
	unsigned char funct = (unsigned char) (inst & 0x3f);
	/*unsigned char rs =  (unsigned char) ( (inst >> 21) & 0x1f );
	unsigned char rt = (unsigned char) ( (inst >> 16) & 0x1f );
	unsigned char shamt = (unsigned char) ( (inst >> 6) & 0x1f );
	unsigned char rd = (unsigned char) ( (inst >> 11) & 0x1f );*/
	
	if(opcode == 0x00){
		RegDst = true;
		// ALUop[] = {1, 0} means ALU neads funct to know what to perform 
		ALUop[0] = false;
		ALUop[1] = true;
		ALUSrc = false;

		Branch = false;
		MemWrite = MemRead = false;

		RegWrite = true;
		MemtoReg = false;

		switch(funct)
		{

			case 0x20:	// add
				break;

			case 0x21:	// addu
				break;

			case 0x22:	// sub
				break;

			case 0x24:	// and
				break;

			case 0x25:	// or
				break;

			case 0x26:	//xor
				break;

			case 0x27:	//nor
				break;

			case 0x28:	//nand
				break;

			case 0x2a:	//slt
				break;

			case 0x00:	//sll
				break;

			case 0x02:	//srl
				break;

			case 0x03:	//sra
				break;

			case 0x08:	//jr
				RegWrite = false;
				break;

			default: break;
		}
	} else {
		RegDst = false;
		ALUSrc = true;


		Branch = false;
		MemWrite = MemRead = false;

		RegWrite = true; // below to specify which instruction don't write register
		MemtoReg = false;
		switch(opcode)
		{
			//printf("immediate==%d\n", immediate);
			//--------------------------- I type start -----------------------------//
			case 0x08: 	// addi
				break;

			case 0x09:	// addiu
				break;

			case 0x23:	//lw
				// ALUop[] = {0, 0} means ALU needs to perform addition to calculate $rs + immediate
				ALUop[0] = ALUop[1] = false;
				MemWrite = false;
				MemRead = true;
				MemtoReg = true;
				break;

			case 0x21:	//lh
				// ALUop[] = {0, 0} means ALU needs to perform addition to calculate $rs + immediate
				ALUop[0] = ALUop[1] = false;
				MemWrite = false;
				MemRead = true;
				MemtoReg = true;
				break;

			case 0x25:	//lhu 
				// ALUop[] = {0, 0} means ALU needs to perform addition to calculate $rs + immediate
				ALUop[0] = ALUop[1] = false;
				MemWrite = false;
				MemRead = true;
				MemtoReg = true;
				break;

			case 0x20:	//lb 
				// ALUop[] = {0, 0} means ALU needs to perform addition to calculate $rs + immediate
				ALUop[0] = ALUop[1] = false;
				MemWrite = false;
				MemRead = true;
				MemtoReg = true;
				break;

			case 0x24:	//lbu
				// ALUop[] = {0, 0} means ALU needs to perform addition to calculate $rs + immediate
				ALUop[0] = ALUop[1] = false;
				MemWrite = false;
				MemRead = true;
				MemtoReg = true;
				break;

			case 0x2B:	//sw
				MemWrite = true;
				MemRead = false;
				RegWrite = false;
				break;

			case 0x29:	//sh
				MemWrite = true;
				MemRead = false;
				RegWrite = false;
				break;

			case 0x28:	//sb
				MemWrite = true;
				MemRead = false;
				RegWrite = false;
				break;

			case 0x0F:	//lui 
				break;

			case 0x0C:	//andi 
				break;

			case 0x0D:	//ori 
				break;

			case 0x0E:	//nori 
				break;
			
			case 0x0A:	//slti
				break;

			case 0x04:	//beq
				// ALUop[] = {0, 1} means in ID stage, perfrom a detect whether to branch
				ALUop[0] = false;
				ALUop[1] = true;
				ALUSrc = false;

				Branch = true;
				RegWrite = false;
				break;

			case 0x05:	//bne 
				// ALUop[] = {0, 1} means in ID stage, perfrom a detect whether to branch
				ALUop[0] = false;
				ALUop[1] = true;
				ALUSrc = false;

				Branch = true;
				RegWrite = false;
				break;
			
			case 0x07:	//bgtz 
				// ALUop[] = {0, 1} means in ID stage, perfrom a detect whether to branch
				ALUop[0] = false;
				ALUop[1] = true;
				ALUSrc = false;

				Branch = true;
				RegWrite = false;
				break;
			//--------------------------- I type end -----------------------------//


			//--------------------------- J type start -----------------------------//
			case 0x02:	//j
				RegWrite = false;
				break;

			case 0x03:	//jal
				RegWrite = true;
				break;
			//--------------------------- J type end -----------------------------//

			case 0x3f:	// halt
				break;
			default:
				break;
		}
	}
	used = true;
}


void CtrUnit::print_all(void)
{
	printf("Jump==%d", (int) Jump);

	printf(", RegDst==%d", (int )RegDst);
	printf(", ALUop[2]=={%d, %d}", (int)ALUop[0], (int)ALUop[1]);
	printf(", ALUSrc==%d", (int)ALUSrc);

	printf("\n");

	printf(", Branch==%d", (int)Branch);
	printf(", MemRead==%d", (int)MemRead);
	printf(", MemWrite==%d", (int)MemWrite);
	
	printf(", MemtoReg==%d", (int)MemtoReg);
	printf(", RegWrite==%d", (int)RegWrite);

	printf("\n\n");
}


void CtrUnit::flush(void)
{
	Jump = false;

	RegDst = false;
	ALUop[0] = ALUop[1] = false;
	ALUSrc = false;

	Branch = false;
	MemRead = false;
	MemWrite = false;
	
	MemtoReg = false;
	RegWrite = false;
}