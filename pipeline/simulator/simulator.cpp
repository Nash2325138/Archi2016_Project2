#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>

#include <string.h>

#include "./instruction.h"
#include "./regfile.h"
#include "./memory.h"
#include "./ctrUnit.h"
#include "./stageBuffer.h"

#define RV_NORMAL 1
#define RV_HALT 0
#define RV_CRITICAL_ERROR -1

#define DEBUG_CYCLE 999999

char UpperStringInst[30];

FILE *snapshot;
FILE *error_dump;

Memory* memory;
InstructionMemery* instructions;
Registers* regs;
unsigned int PC;
int cycle;
CtrUnit * (ctrUnit)[30];

IF_ID_Buffer IF_ID_buffer_back;
IF_ID_Buffer IF_ID_buffer_front;

ID_EX_Buffer ID_EX_buffer_back;
ID_EX_Buffer ID_EX_buffer_front;

EX_MEM_Buffer EX_MEM_buffer_back;
EX_MEM_Buffer EX_MEM_buffer_front;

MEM_WB_Buffer MEM_WB_buffer_back;
MEM_WB_Buffer MEM_WB_buffer_front;

std::vector<unsigned int>* readImage(FILE *);
void readInput_initialize(void);
void print_snapshot(void);
int execute(void);
void destroy_all(void);
void sumOverflow(int aluValue1, int aluValue2);
bool needTermination(int *stageReturnValue);
CtrUnit * getEmptyCtrUnit(void)
{
	for(int i=0 ; i<30 ; i++){
		if(ctrUnit[i]->used == false) return ctrUnit[i];
	}
	return NULL;
}

void inst_UpperString(unsigned int inst)
{
	unsigned char opcode = (unsigned char) (inst >> 26);		//warning: unsigned char has 8 bits
	unsigned char funct = (unsigned char) (inst & 0x3f);
	if(opcode == 0x00){
		switch(funct)
		{
			case 0x20:	// add
				strcpy(UpperStringInst, "ADD");
				break;

			case 0x21:	// addu
				strcpy(UpperStringInst, "ADDU");
				break;

			case 0x22:	// sub
				strcpy(UpperStringInst, "SUB");
				break;

			case 0x24:	// and
				strcpy(UpperStringInst, "AND");
				break;

			case 0x25:	// or
				strcpy(UpperStringInst, "OR");
				break;

			case 0x26:	//xor
				strcpy(UpperStringInst, "XOR");
				break;

			case 0x27:	//nor
				strcpy(UpperStringInst, "NOR");
				break;

			case 0x28:	//nand
				strcpy(UpperStringInst, "NAND");
				break;

			case 0x2a:	//slt
				strcpy(UpperStringInst, "SLT");
				break;

			case 0x00:	//sll
				strcpy(UpperStringInst, "SLL");
				break;

			case 0x02:	//srl
				strcpy(UpperStringInst, "SRL");
				break;

			case 0x03:	//sra
				strcpy(UpperStringInst, "SRA");
				break;

			case 0x08:	//jr
				strcpy(UpperStringInst, "JR");
				break;

			default: break;
		}
	} else {
		switch(opcode)
		{
			case 0x08: 	// addi
				strcpy(UpperStringInst, "ADDI");
				break;

			case 0x09:	// addiu
				strcpy(UpperStringInst, "ADDIU");
				break;

			case 0x23:	//lw
				strcpy(UpperStringInst, "LW");
				break;

			case 0x21:	//lh
				strcpy(UpperStringInst, "LH");
				break;

			case 0x25:	//lhu 
				strcpy(UpperStringInst, "LHU");
				break;

			case 0x20:	//lb 
				strcpy(UpperStringInst, "LB");
				break;

			case 0x24:	//lbu
				strcpy(UpperStringInst, "LBU");
				break;

			case 0x2B:	//sw
				strcpy(UpperStringInst, "SW");
				break;

			case 0x29:	//sh
				strcpy(UpperStringInst, "SH");
				break;

			case 0x28:	//sb
				strcpy(UpperStringInst, "SB");
				break;

			case 0x0F:	//lui 
				strcpy(UpperStringInst, "LUI");
				break;

			case 0x0C:	//andi 
				strcpy(UpperStringInst, "ANDI");
				break;

			case 0x0D:	//ori 
				strcpy(UpperStringInst, "ORI");
				break;

			case 0x0E:	//nori 
				strcpy(UpperStringInst, "NORI");
				break;
			
			case 0x0A:	//slti
				strcpy(UpperStringInst, "SLTI");
				break;

			case 0x04:	//beq
				strcpy(UpperStringInst, "BEQ");
				break;

			case 0x05:	//bne 
				strcpy(UpperStringInst, "BNE");
				break;
			
			case 0x07:	//bgtz 
				strcpy(UpperStringInst, "BGTZ");
				break;
			//--------------------------- I type end -----------------------------//


			//--------------------------- J type start -----------------------------//
			case 0x02:	//j
				strcpy(UpperStringInst, "J");
				break;

			case 0x03:	//jal
				strcpy(UpperStringInst, "JAL");
				break;
			//--------------------------- J type end -----------------------------//


			
			case 0x3f:	// halt
				strcpy(UpperStringInst, "HALT");
				break;
			default:
				fputs("no such instruction", stderr);
				break;
		}
	}
}

void stage_fetch(void)
{
	// PCSrc == 1 ?
	if(EX_MEM_buffer_front.control->Branch && EX_MEM_buffer_front.ALU_zero){
		PC = EX_MEM_buffer_front.PC_result;
	} else {
		// PC value remains
	}
	unsigned int inst = instructions->at(PC/4);
	fprintf(snapshot, "IF: 0x%08X", inst);

	CtrUnit *control;
	if( (control = getEmptyCtrUnit())==NULL) fprintf(stderr, "No empty CtrUnit\n");
	PC = PC + 4;

	// in my code, instruction decode to control signal are done in IF stage
	control->change(inst);
	IF_ID_buffer_back.put(inst, control, PC);
	
	fprintf(snapshot, "\n");	
}

int stage_decode(void)
{
	ID_EX_buffer_back.inst = IF_ID_buffer_front.inst;
	inst_UpperString(IF_ID_buffer_front.inst);
	fprintf(snapshot, "%s", UpperStringInst);
	if(IF_ID_buffer_front.inst == 0xffffffff) {
		return RV_HALT;
		fprintf(snapshot, "\n");
	}

	ID_EX_buffer_back.PC_puls_4 = IF_ID_buffer_front.PC_puls_4;
	ID_EX_buffer_back.control = IF_ID_buffer_front.control;

	ID_EX_buffer_back.opcode = IF_ID_buffer_front.opcode;
	ID_EX_buffer_back.funct = IF_ID_buffer_front.funct;
	ID_EX_buffer_back.shamt = IF_ID_buffer_front.shamt;

	ID_EX_buffer_back.rs_data = (int) regs->at(IF_ID_buffer_front.rs);
	ID_EX_buffer_back.rt_data = (int) regs->at(IF_ID_buffer_front.rt);
	ID_EX_buffer_back.extented_immediate = (signed)IF_ID_buffer_front.immediate;

	ID_EX_buffer_back.rt = IF_ID_buffer_front.rt;
	ID_EX_buffer_back.rd = IF_ID_buffer_front.rd;
	fprintf(snapshot, "\n");
	return RV_NORMAL;
}

int stage_execute(void)
{
	EX_MEM_buffer_back.inst = ID_EX_buffer_front.inst;
	inst_UpperString(ID_EX_buffer_front.inst);
	fprintf(snapshot, "%s", UpperStringInst);
	if(ID_EX_buffer_front.inst == 0xffffffff) {
		fprintf(snapshot, "\n");
		return RV_HALT;
	}

	// EX_MEM_buffer_back. = ID_EX_buffer_front.
	EX_MEM_buffer_back.control = ID_EX_buffer_front.control;
	EX_MEM_buffer_back.PC_result = ID_EX_buffer_front.PC_puls_4 + (ID_EX_buffer_front.extented_immediate << 2);

	int aluValue1 = ID_EX_buffer_front.rs_data;
	int aluValue2 = (ID_EX_buffer_front.control->ALUSrc) ? ID_EX_buffer_front.extented_immediate : ID_EX_buffer_front.rt_data;
	int alu_result;

	if(ID_EX_buffer_front.opcode == 0x00){
		switch(ID_EX_buffer_front.funct)
		{
			case 0x20:	// add
				sumOverflow(aluValue1, aluValue2);
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x21:	// addu
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x22:	// sub
				aluValue2 = (~(aluValue2))+1;
				sumOverflow(aluValue1, aluValue2);
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x24:	// and
				alu_result = aluValue1 & aluValue2;
				break;

			case 0x25:	// or
				alu_result = aluValue1 | aluValue2;
				break;

			case 0x26:	//xor
				alu_result = aluValue1 ^ aluValue2;
				break;

			case 0x27:	//nor
				alu_result = ~(aluValue1 | aluValue2);
				break;

			case 0x28:	//nand
				alu_result = ~(aluValue1 & aluValue2);
				break;

			case 0x2a:	//slt
				alu_result = ( aluValue1 < aluValue2 ) ? 1:0;
				break;

			case 0x00:	//sll
				alu_result = aluValue1 << ID_EX_buffer_front.shamt;
				break;

			case 0x02:	//srl
				alu_result = ((unsigned int)aluValue1) >> ID_EX_buffer_front.shamt;
				break;

			case 0x03:	//sra
				alu_result = ((int)aluValue1) >> ID_EX_buffer_front.shamt;
				break;

			case 0x08:	//jr
				break;

			default: break;
		}
	} else {
		//int aluValue1 = (int)regs->at(rs);
		//int aluValue2 = (signed short)immediate;
		//unsigned int location = (regs->at(rs) + ((signed short)immediate) );
		switch(ID_EX_buffer_front.opcode)
		{
			//--------------------------- I type start -----------------------------//
			case 0x08: 	// addi
				sumOverflow(aluValue1, aluValue2);
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x09:	// addiu
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x23:	//lw
				sumOverflow(aluValue1, aluValue2);
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x21:	//lh
				sumOverflow(aluValue1, aluValue2);
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x25:	//lhu 
				sumOverflow(aluValue1, aluValue2);
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x20:	//lb 
				sumOverflow(aluValue1, aluValue2);
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x24:	//lbu
				sumOverflow(aluValue1, aluValue2);
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x2B:	//sw
				sumOverflow(aluValue1, aluValue2);
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x29:	//sh
				sumOverflow(aluValue1, aluValue2);
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x28:	//sb
				sumOverflow(aluValue1, aluValue2);
				alu_result = aluValue1 + aluValue2;
				break;

			case 0x0F:	//lui 
				alu_result = aluValue2 << 16;
				break;

			case 0x0C:	//andi ---> $t = $s & C(unsigned)
				alu_result = aluValue1 & (aluValue2 & 0x0000ffff);
				break;

			case 0x0D:	//ori ---> $t = $s | C(unsigned)
				alu_result = aluValue1 | (aluValue2 & 0x0000ffff);
				break;

			case 0x0E:	//nori ---> $t = ~($s | C(unsigned))
				alu_result = ~( aluValue1 | (aluValue2 & 0x0000ffff) );
				break;
			
			case 0x0A:	//slti
				if( aluValue1 < aluValue2 ) alu_result = 1;
				else alu_result = 0;
				break;

			// ---------- in project 2, branch and jump should be done at ID stage !!!! ---------- //
			case 0x04:	//beq
				EX_MEM_buffer_back.ALU_zero = ( aluValue1==aluValue2 ) ? true : false;
				break;

			case 0x05:	//bne 
				EX_MEM_buffer_back.ALU_zero = ( aluValue1!=aluValue2 ) ? true : false;
				break;
			
			case 0x07:	//bgtz 
				EX_MEM_buffer_back.ALU_zero = ( aluValue1 > 0 ) ? true : false;
				break;
			//--------------------------- I type end -----------------------------//


			//--------------------------- J type start -----------------------------//
			case 0x02:	//j
				//if(toReturn!=0) return toReturn;
				//PC &= 0xf0000000;
				//PC |= ( ((unsigned int)address) << 2 );
				break;

			case 0x03:	//jal
				//if(toReturn!=0) return toReturn;
				//regs->at(31) = PC;
				//PC &= 0xf0000000;
				//PC |= ( ((unsigned int)address) << 2 );
				break;
			//--------------------------- J type end -----------------------------//


			
			case 0x3f:	// halt
				return -1;
				break;
			default:
				fputs("no such instruction", stderr);
				break;
		}
	}
	EX_MEM_buffer_back.ALU_result = alu_result;
	EX_MEM_buffer_back.rt_data = ID_EX_buffer_front.rt_data;	// needs forwarding if EX/MEM.RegWrite && ( ID/EX.rt==EX/MEM.write_destination )
	EX_MEM_buffer_back.opcode = ID_EX_buffer_front.opcode;

	EX_MEM_buffer_back.write_destination = (ID_EX_buffer_front.control->RegDst) ? ID_EX_buffer_front.rd : ID_EX_buffer_front.rt;
	fprintf(snapshot, "\n");
	return RV_NORMAL;
}
int stage_memory(void)
{
	MEM_WB_buffer_back.inst = EX_MEM_buffer_front.inst;
	inst_UpperString(EX_MEM_buffer_front.inst);
	fprintf(snapshot, "%s", UpperStringInst);
	if(EX_MEM_buffer_front.inst == 0xffffffff) {
		fprintf(snapshot, "\n");
		return RV_HALT;
	}
	MEM_WB_buffer_back.control = EX_MEM_buffer_front.control;
	int location = EX_MEM_buffer_front.ALU_result;
	int rt_data = EX_MEM_buffer_front.rt_data;
	int tempValue, toReturn=0;
	if(EX_MEM_buffer_front.control->MemWrite){
		//memory->at(EX_MEM_buffer_front.ALU_result)
		switch(EX_MEM_buffer_front.opcode)
		{
			case 0x2B:	//sw
				if ( location >1020 ) {
					fprintf(error_dump, "In cycle %d: Address Overflow\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if( location % 4 != 0 ){
					fprintf(error_dump, "In cycle %d: Misalignment Error\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if(toReturn!=0) return toReturn;
				memory->at(location/4) = rt_data;
				break;

			case 0x29:	//sh
				if ( location >1022 ) {
					fprintf(error_dump, "In cycle %d: Address Overflow\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if( location % 2 != 0 ){
					fprintf(error_dump, "In cycle %d: Misalignment Error\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if(toReturn!=0) return toReturn;
				
				memory->at(location/4) &= (	(location%4==0) ? 0x0000ffff :
											(location%4==2) ? 0xffff0000 : 0xffff0000 );
				tempValue = rt_data & 0x0000ffff;
				tempValue <<= (	(location%4==0) ? 16 :
								(location%4==2) ? 0  : 0 );
				memory->at(location/4) |= tempValue;
				break;

			case 0x28:	//sb
				if ( location >1023 ) {
					fprintf(error_dump, "In cycle %d: Address Overflow\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if(toReturn!=0) return toReturn;
				
				memory->at(location/4) &= (	(location%4==0) ? 0x00ffffff :
											(location%4==1) ? 0xff00ffff :
											(location%4==2) ? 0xffff00ff :
											(location%4==3) ? 0xffffff00 : 0xffffff00 );
				tempValue = rt_data & 0x000000ff;
				tempValue <<= (	(location%4==0) ? 24 :
								(location%4==1) ? 16 :
								(location%4==2) ? 8  :
								(location%4==3) ? 0  : 0 );
				memory->at(location/4) |= tempValue;
				break;
		}
	}
	if(EX_MEM_buffer_front.control->MemRead){

		signed short halfLoaded;
		signed char byteLoaded;
		switch(EX_MEM_buffer_front.opcode)
		{
			case 0x23:	//lw
				if ( location >1020 ) {
					fprintf(error_dump, "In cycle %d: Address Overflow\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if( location % 4 != 0 ){
					fprintf(error_dump, "In cycle %d: Misalignment Error\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if(toReturn!=0) return toReturn;
				MEM_WB_buffer_back.memory_result = memory->at(location);
				break;

			case 0x21:	//lh
				if( location > 1022) {
					fprintf(error_dump, "In cycle %d: Address Overflow\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if( location % 2 != 0){
					fprintf(error_dump, "In cycle %d: Misalignment Error\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if(toReturn!=0) return toReturn;

				if(location%4==0) halfLoaded = (signed short) ( (memory->at(location/4)) >> 16);
				else if(location%2==0) halfLoaded = (signed short) ( (memory->at(location/4)) & 0x0000ffff );
				MEM_WB_buffer_back.memory_result = (signed short)halfLoaded;		// <-------- this line is very important!!!
				break;

			case 0x25:	//lhu 
				if( location > 1022) {
					fprintf(error_dump, "In cycle %d: Address Overflow\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if( location % 2 != 0){
					fprintf(error_dump, "In cycle %d: Misalignment Error\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if(toReturn!=0) return toReturn;

				if(location%4==0) halfLoaded = (unsigned short) ( ((unsigned int)(memory->at(location/4))) >> 16);
				else if(location%2==0) halfLoaded = (unsigned short) ( (memory->at(location/4)) & 0x0000ffff );
				MEM_WB_buffer_back.memory_result = (unsigned short)halfLoaded;
				break;

			case 0x20:	//lb 
				if( location > 1023) {
					fprintf(error_dump, "In cycle %d: Address Overflow\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if(toReturn!=0) return toReturn;

				byteLoaded = (signed char) ( ((unsigned int)(memory->at(location/4))) >> (
																	(location%4==0) ? 24 :
																	(location%4==1) ? 16 :
																	(location%4==2) ? 8  :
																	(location%4==3) ? 0 : 0) ) & 0x000000ff; 

				MEM_WB_buffer_back.memory_result = (signed char)byteLoaded;
				break;

			case 0x24:	//lbu
				if( location > 1023 ) {
					fprintf(error_dump, "In cycle %d: Address Overflow\n", cycle);
					toReturn = RV_CRITICAL_ERROR;
				}
				if(toReturn!=0) return toReturn;

				byteLoaded = (unsigned char) (((unsigned int)(memory->at(location/4))) >> (
															(location%4==0) ? 24 :
															(location%4==1) ? 16 :
															(location%4==2) ? 8  :
															(location%4==3) ? 0 : 0) ) & 0x000000ff; 

				MEM_WB_buffer_back.memory_result = (unsigned char)byteLoaded;
				break;
		}
	}
	MEM_WB_buffer_back.ALU_result = EX_MEM_buffer_front.ALU_result;
	MEM_WB_buffer_back.write_destination = EX_MEM_buffer_front.write_destination;

	fprintf(snapshot, "\n");
	return RV_NORMAL;
}

int stage_writeBack(void)
{
	inst_UpperString(MEM_WB_buffer_front.inst);
	fprintf(snapshot, "%s", UpperStringInst);
	if(MEM_WB_buffer_front.inst == 0xffffffff){
		fprintf(snapshot, "\n");
		return RV_HALT;
	}
	if(MEM_WB_buffer_front.control->RegWrite)
	{
		if(MEM_WB_buffer_front.write_destination==0){
			if( (MEM_WB_buffer_front.inst & 0xf8000000)==0 ) {}		//NOP
			else {
				fprintf(error_dump, "In cycle %d: Write $0 Error\n", cycle);
				return RV_NORMAL;
			}
		}
		int write_data = (MEM_WB_buffer_front.control->MemtoReg) ?  MEM_WB_buffer_front.memory_result : MEM_WB_buffer_front.ALU_result;
		regs->at(MEM_WB_buffer_front.write_destination) = write_data;
	}
	fprintf(snapshot, "\n");
	return RV_NORMAL;
}

void trigger(void)
{
	MEM_WB_buffer_front.control->used = false;

	MEM_WB_buffer_front = MEM_WB_buffer_back;
	EX_MEM_buffer_front = EX_MEM_buffer_back;
	ID_EX_buffer_front = ID_EX_buffer_back;
	IF_ID_buffer_front = IF_ID_buffer_back;
}

int main(int argc, char const *argv[])
{
	/*printf("This is the main functuion in simulator.cpp\n");
	instruction();
	regfile();
	memory();*/
	readInput_initialize();
	//for(int i=0 ; i<instructions->size() ; i++)printf("%x ", instructions->at(i));
	//for(int i=0 ; i<regs->size() ; i++) printf("%x ", regs->at(i));
	//printf("\n") ;

	cycle = 0;
	snapshot = fopen("snapshot.rpt", "w");
	if(snapshot==NULL){
		fputs("snapshot write error", stderr);
		exit(EXIT_FAILURE);
	}
	error_dump = fopen("error_dump.rpt", "w");
	if(error_dump==NULL){
		fputs("error_dump write error", stderr);
		exit(EXIT_FAILURE);
	}

	int stageReturnValue[4];
	/*do{
		print_snapshot();
		cycle++;
	}while (execute() != -1 && cycle < 510000);*/
	while(true)
	{
		print_snapshot();
		cycle++;
		stageReturnValue[0] = stage_writeBack();
		stageReturnValue[1] = stage_memory();
		stageReturnValue[2] = stage_execute();
		stageReturnValue[3] = stage_decode();
		trigger();
		fprintf(snapshot, "\n\n");
		if( needTermination(stageReturnValue) || cycle > 600000) break;
	}
	

	destroy_all();
	return 0;
}

bool needTermination(int *stageReturnValue)
{
	for(int i=0 ; i<4 ; i++)
		if(stageReturnValue[i]==RV_CRITICAL_ERROR) return true;

	bool flag = true;
	for(int i=0 ; i<4 ; i++){
		if(stageReturnValue[i]==RV_NORMAL) flag = false;
	}
	return flag;
}

void readInput_initialize(void)
{
	FILE *dimage = fopen("dimage.bin", "rb");
	if( dimage==NULL ) {
		fputs("dimage read error", stderr);
		exit(2);
	}
	unsigned int sp=0;
	unsigned char readByte;
	for(int i=0 ; i<4 ; i++){
		fread(&readByte, sizeof(unsigned char), 1, dimage);
		sp <<= 8;
		sp |= readByte;
	}
	memory = new Memory(dimage);
	//for(unsigned int i=0 ; i<memory->size() ; i++)	printf("%x\n", memory->at(i));

	
	FILE *iimage = fopen("iimage.bin", "rb");
	if(iimage==NULL) {
		fputs("iimage read error", stderr);
		exit(2);
	}
	PC = 0;
	for(int i=0 ; i<4; i++){
		fread(&readByte, sizeof(unsigned char), 1, iimage);
		PC <<= 8;
		PC |= readByte;
	}
	instructions = new InstructionMemery(PC, iimage);
	//for(unsigned int i=0 ; i<instructions->size() ; i++)	printf("%x\n", instructions->at(i));

	regs = new Registers(sp);
	//for(unsigned int i=0 ; i<regs->size() ; i++)	printf("%d: %x\n", i, regs->at(i));
	//printf("\n");
	for(int i=0 ; i<30 ; i++){
		ctrUnit[i] = new CtrUnit();
	}
}


void print_snapshot(void)
{
	fprintf(snapshot , "cycle %d\n", cycle);
	for(unsigned int i=0 ; i<regs->size() ; i++){
		fprintf(snapshot, "$%02d: 0x%08X\n", i, (unsigned int)regs->at(i));
	}
	fprintf(snapshot, "PC: 0x%08X\n", PC);
}




void destroy_all(void)
{
	delete memory;
	delete instructions;
	delete regs;
	for(int i=0 ; i<30 ; i++) delete ctrUnit[i];

	fclose(snapshot);
	fclose(error_dump);
}



void sumOverflow(int aluValue1, int aluValue2)
{
	if(aluValue1<=0 && aluValue2<=0 && aluValue1+aluValue2 > 0){
		fprintf(error_dump, "In cycle %d: Number Overflow\n", cycle);
	}
	if(aluValue1>=0 && aluValue2>=0 && aluValue1+aluValue2 < 0){
		fprintf(error_dump, "In cycle %d: Number Overflow\n", cycle);
	}
	if(aluValue1<0 && aluValue2<0 && aluValue1+aluValue2==0){
		fprintf(error_dump, "In cycle %d: Number Overflow\n", cycle);
	}
}
/*
std::vector<unsigned int>* readImage(FILE *image)
{
	std::vector<unsigned int> *input = new std::vector<unsigned int>;
	input->reserve(256);
	unsigned char readByte;
	unsigned int count=0, temp=0;
	while(fread(&readByte, sizeof(unsigned char), 1, image) != 0){
		temp <<= 8;
		temp |= readByte;
		count++;
		if(count >= 4){
			input->push_back(temp);
			temp = count = 0;	
		}
	}
	//printf("input has %lu words\n", input->size()) ;
	return input;
}
*/