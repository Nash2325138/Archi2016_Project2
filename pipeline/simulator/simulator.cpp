#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>

#include "./instruction.h"
#include "./regfile.h"
#include "./memory.h"
#include "./ctrUnit.h"
#include "./stageBuffer.h"

#define RV_NORMAL 1
#define RV_HALT 0
#define RV_CRITICAL_ERROR -1

#define DEBUG_CYCLE 999999

FILE *snapshot;
FILE *error_dump;

Memory* memory;
InstructionMemery* instructions;
Registers* regs;
unsigned int PC;
int cycle;
CtrUnit * (ctrUnit)[10];

IF_ID_Buffer IF_ID_buffer;
ID_EX_Buffer ID_EX_buffer;
EX_MEM_Buffer EX_MEM_buffer;
MEM_WB_Buffer MEM_WB_buffer;

std::vector<unsigned int>* readImage(FILE *);
void readInput_initialize(void);
void print_snapshot(void);
int execute(void);
void destroy_all(void);
void sumOverflow(int aluValue1, int aluValue2);
bool needTermination(int *stageReturnValue);
CtrUnit * getEmptyCtrUnit(void)
{
	for(int i=0 ; i<10 ; i++){
		if(ctrUnit[i]->used == false) return ctrUnit[i];
	}
	return NULL;
}
void stage_fetch(void)
{
	// PCSrc == 1 ?
	if(EX_MEM_buffer.control->Branch && EX_MEM_buffer.ALU_zero){
		PC = EX_MEM_buffer.PC_result;
	} else {
		// PC value remains
	}
	unsigned int inst = instructions->at(PC/4);
	CtrUnit *control;
	if( (control = getEmptyCtrUnit())==NULL) fprintf(stderr, "No empty CtrUnit\n");
	PC = PC + 4;

	// in my code, instruction decode to control signal are done in IF stage
	control->change(inst);
	IF_ID_buffer.put(inst, control, PC);
}

int stage_decode(void)
{
	ID_EX_buffer.inst = IF_ID_buffer.inst;
	if(IF_ID_buffer.inst == 0xffffffff) return RV_HALT;
	ID_EX_buffer.PC_puls_4 = IF_ID_buffer.PC_puls_4;
	ID_EX_buffer.control = IF_ID_buffer.control;

	ID_EX_buffer.opcode = IF_ID_buffer.opcode;
	ID_EX_buffer.funct = IF_ID_buffer.funct;
	ID_EX_buffer.shamt = IF_ID_buffer.shamt;

	ID_EX_buffer.rs_data = (int) regs->at(IF_ID_buffer.rs);
	ID_EX_buffer.rt_data = (int) regs->at(IF_ID_buffer.rt);
	ID_EX_buffer.extented_immediate = (signed)IF_ID_buffer.immediate;

	ID_EX_buffer.rt = IF_ID_buffer.rt;
	ID_EX_buffer.rd = IF_ID_buffer.rd;
	return RV_NORMAL;
}

int stage_execute(void)
{
	EX_MEM_buffer.inst = ID_EX_buffer.inst;
	if(ID_EX_buffer.inst == 0xffffffff) return RV_HALT;
	// EX_MEM_buffer. = ID_EX_buffer.
	EX_MEM_buffer.control = ID_EX_buffer.control;
	EX_MEM_buffer.PC_result = ID_EX_buffer.PC_puls_4 + (ID_EX_buffer.extented_immediate << 2);

	int aluValue1 = ID_EX_buffer.rs_data;
	int aluValue2 = (ID_EX_buffer.control->ALUSrc) ? ID_EX_buffer.extented_immediate : ID_EX_buffer.rt_data;
	int alu_result;

	if(ID_EX_buffer.opcode == 0x00){
		switch(ID_EX_buffer.funct)
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
				alu_result = aluValue1 << ID_EX_buffer.shamt;
				break;

			case 0x02:	//srl
				alu_result = ((unsigned int)aluValue1) >> ID_EX_buffer.shamt;
				break;

			case 0x03:	//sra
				alu_result = ((int)aluValue1) >> ID_EX_buffer.shamt;
				break;

			case 0x08:	//jr
				break;

			default: break;
		}
	} else {
		//int aluValue1 = (int)regs->at(rs);
		//int aluValue2 = (signed short)immediate;
		//unsigned int location = (regs->at(rs) + ((signed short)immediate) );
		switch(ID_EX_buffer.opcode)
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
				EX_MEM_buffer.ALU_zero = ( aluValue1==aluValue2 ) ? true : false;
				break;

			case 0x05:	//bne 
				EX_MEM_buffer.ALU_zero = ( aluValue1!=aluValue2 ) ? true : false;
				break;
			
			case 0x07:	//bgtz 
				EX_MEM_buffer.ALU_zero = ( aluValue1 > 0 ) ? true : false;
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
	EX_MEM_buffer.ALU_result = alu_result;
	EX_MEM_buffer.rt_data = ID_EX_buffer.rt_data;	// needs forwarding if EX/MEM.RegWrite && ( ID/EX.rt==EX/MEM.write_destination )
	EX_MEM_buffer.opcode = ID_EX_buffer.opcode;

	EX_MEM_buffer.write_destination = (ID_EX_buffer.control->RegDst) ? ID_EX_buffer.rd : ID_EX_buffer.rt;
	return RV_NORMAL;
}
int stage_memory(void)
{
	MEM_WB_buffer.inst = EX_MEM_buffer.inst;
	if(EX_MEM_buffer.inst == 0xffffffff) return RV_HALT;
	MEM_WB_buffer.control = EX_MEM_buffer.control;
	int location = EX_MEM_buffer.ALU_result;
	int rt_data = EX_MEM_buffer.rt_data;
	int tempValue, toReturn=0;
	if(EX_MEM_buffer.control->MemWrite){
		//memory->at(EX_MEM_buffer.ALU_result)
		switch(EX_MEM_buffer.opcode)
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
	if(EX_MEM_buffer.control->MemRead){

		signed short halfLoaded;
		signed char byteLoaded;
		switch(EX_MEM_buffer.opcode)
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
				MEM_WB_buffer.memory_result = memory->at(location);
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
				MEM_WB_buffer.memory_result = (signed short)halfLoaded;		// <-------- this line is very important!!!
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
				MEM_WB_buffer.memory_result = (unsigned short)halfLoaded;
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

				MEM_WB_buffer.memory_result = (signed char)byteLoaded;
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

				MEM_WB_buffer.memory_result = (unsigned char)byteLoaded;
				break;
		}
	}
	MEM_WB_buffer.ALU_result = EX_MEM_buffer.ALU_result;
	MEM_WB_buffer.write_destination = EX_MEM_buffer.write_destination;

	return RV_NORMAL;
}

int stage_writeBack(void)
{
	if(MEM_WB_buffer.inst == 0xffffffff) return RV_HALT;
	if(MEM_WB_buffer.control->RegWrite)
	{
		if(MEM_WB_buffer.write_destination==0){
			if( (MEM_WB_buffer.inst & 0xf8000000)==0 ) {}		//NOP
			else {
				fprintf(error_dump, "In cycle %d: Write $0 Error\n", cycle);
				return RV_NORMAL;
			}
		}
		int write_data = (MEM_WB_buffer.control->MemtoReg) ?  MEM_WB_buffer.memory_result : MEM_WB_buffer.ALU_result;
		regs->at(MEM_WB_buffer.write_destination) = write_data;
	}
	return RV_NORMAL;
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
		exit(666);
	}
	error_dump = fopen("error_dump.rpt", "w");
	if(error_dump==NULL){
		fputs("error_dump write error", stderr);
		exit(666);
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
	for(int i=0 ; i<10 ; i++){
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
	fprintf(snapshot, "\n\n");
}




void destroy_all(void)
{
	delete memory;
	delete instructions;
	delete regs;
	for(int i=0 ; i<10 ; i++) delete ctrUnit[i];

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