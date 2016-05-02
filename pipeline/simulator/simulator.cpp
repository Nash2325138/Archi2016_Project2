#include <cstdio>
#include <cstdlib>
// #include <iostream>
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

#define DEBUG_CYCLE 4

char UpperStringInst[30];
char snapshotWriterBuffer[5][100];

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

void write_32bits_to_image(FILE *image, unsigned int number)
{
	for(int i=0 ; i<4 ; i++){
		unsigned char temp = (unsigned char)(number >> 24);
		fwrite(&temp, sizeof(unsigned char), 1, image);
		number <<= 8;
	}
}
void print_dissembled_inst(unsigned int inst)
{
	FILE *tempBin = fopen("inst_temp.bin", "wb");
	write_32bits_to_image(tempBin, 0);
	write_32bits_to_image(tempBin, 1);
	write_32bits_to_image(tempBin, inst);
	fclose(tempBin);

	system("../little_bird_assembler/assembler -d inst_temp.bin -o inst_temp_dissembled.txt -nolabel");
	
	FILE *tempTxt = fopen("inst_temp_dissembled.txt", "r");
	char str[200];
	fgets(str, sizeof str, tempTxt);
	printf("%s", str);
	fclose(tempTxt);
}

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
	unsigned char rt = (unsigned char) ( (inst >> 16) & 0x1f );
	unsigned char shamt = (unsigned char) ( (inst >> 6) & 0x1f );
	unsigned char rd = (unsigned char) ( (inst >> 11) & 0x1f );
				
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
				if( rt==0 && rd==0 && shamt==0 ) strcpy(UpperStringInst, "NOP"); 
				else strcpy(UpperStringInst, "SLL");
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
	sprintf(snapshotWriterBuffer[0], "IF: 0x%08X", inst);

	CtrUnit *control;
	if( (control = getEmptyCtrUnit())==NULL) fprintf(stderr, "No empty CtrUnit\n");
	PC = PC + 4;

	// in my code, instruction decode to control signal are done in IF stage
	control->change(inst);
	IF_ID_buffer_back.put(inst, control, PC);
}

int stage_decode(void)
{
	ID_EX_buffer_back.inst = IF_ID_buffer_front.inst;
	/*if(cycle==DEBUG_CYCLE){
		printf("ID In cycle %d : ", cycle);
		print_dissembled_inst(IF_ID_buffer_front.inst);
		//printf("IF_ID_buffer_front.rs==%d, IF_ID_buffer_front.rt==%d\n", IF_ID_buffer_front.rs, IF_ID_buffer_front.rt);
	}*/

	inst_UpperString(IF_ID_buffer_front.inst);
	sprintf(snapshotWriterBuffer[1], "ID: %s", UpperStringInst);
	if(IF_ID_buffer_front.inst == 0xffffffff) {
		return RV_HALT;
	}
	IF_ID_buffer_front.control->change(IF_ID_buffer_front.inst);

	ID_EX_buffer_back.PC_puls_4 = IF_ID_buffer_front.PC_puls_4;

	ID_EX_buffer_back.opcode = IF_ID_buffer_front.opcode;
	ID_EX_buffer_back.funct = IF_ID_buffer_front.funct;
	ID_EX_buffer_back.shamt = IF_ID_buffer_front.shamt;

	ID_EX_buffer_back.rs_data = (int) regs->at(IF_ID_buffer_front.rs);
	ID_EX_buffer_back.rt_data = (int) regs->at(IF_ID_buffer_front.rt);
	ID_EX_buffer_back.extented_immediate = (signed short)IF_ID_buffer_front.immediate;

	ID_EX_buffer_back.rt = IF_ID_buffer_front.rt;
	ID_EX_buffer_back.rd = IF_ID_buffer_front.rd;

	if(cycle==DEBUG_CYCLE-1){
		printf("In cycle %d: ", cycle);
		print_dissembled_inst(IF_ID_buffer_front.inst);
		printf("rd==%d ,rs==%d, rt==%d\n", IF_ID_buffer_front.rd, IF_ID_buffer_front.rs, IF_ID_buffer_front.rt);
	}
	return RV_NORMAL;
}

int stage_execute(void)
{
	EX_MEM_buffer_back.inst = ID_EX_buffer_front.inst;
	/*if(cycle==DEBUG_CYCLE){
		printf("EX In cycle %d : ", cycle);
		print_dissembled_inst(ID_EX_buffer_front.inst);
		//printf("aluValue1==%d, aluValue2==%d\n", aluValue1, aluValue2);
	}*/

	inst_UpperString(ID_EX_buffer_front.inst);
	sprintf(snapshotWriterBuffer[2], "EX: %s", UpperStringInst);
	if(ID_EX_buffer_front.inst == 0xffffffff) {
		return RV_HALT;
	}
	ID_EX_buffer_front.control->change(ID_EX_buffer_front.inst);

	// EX_MEM_buffer_back. = ID_EX_buffer_front.
	EX_MEM_buffer_back.PC_result = ID_EX_buffer_front.PC_puls_4 + (ID_EX_buffer_front.extented_immediate << 2);

	int aluValue1 = ID_EX_buffer_front.rs_data;
	int aluValue2 = (ID_EX_buffer_front.control->ALUSrc) ? ID_EX_buffer_front.extented_immediate : ID_EX_buffer_front.rt_data;
	int alu_result;

	if(cycle == DEBUG_CYCLE){
		printf("In cycle %d: ",cycle);
		print_dissembled_inst(ID_EX_buffer_front.inst);
		printf("ALUSrc==%d, rs_data==%d, rt_data==%d, C==%d\n", ID_EX_buffer_front.control->ALUSrc, ID_EX_buffer_front.rs_data, ID_EX_buffer_front.rt_data, ID_EX_buffer_front.extented_immediate);
		printf("alu1==%d, alu2==%d\n", aluValue1, aluValue2);
	}

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
	return RV_NORMAL;
}
int stage_memory(void)
{
	MEM_WB_buffer_back.inst = EX_MEM_buffer_front.inst;
	/*if(cycle==DEBUG_CYCLE){
		printf("MEM In cycle %d : ", cycle);
		print_dissembled_inst(EX_MEM_buffer_front.inst);
	}*/

	inst_UpperString(EX_MEM_buffer_front.inst);
	sprintf(snapshotWriterBuffer[3], "DM: %s", UpperStringInst);
	if(EX_MEM_buffer_front.inst == 0xffffffff) {
		return RV_HALT;
	}
	EX_MEM_buffer_front.control->change(EX_MEM_buffer_front.inst);

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

	return RV_NORMAL;
}

int stage_writeBack(void)
{
	inst_UpperString(MEM_WB_buffer_front.inst);
	/*if(cycle==DEBUG_CYCLE){
		printf("WB In cycle %d : ", cycle);
		print_dissembled_inst(MEM_WB_buffer_front.inst);
	}*/

	sprintf(snapshotWriterBuffer[4], "WB: %s", UpperStringInst);
	if(MEM_WB_buffer_front.inst == 0xffffffff){
		return RV_HALT;
	}
	MEM_WB_buffer_front.control->change(MEM_WB_buffer_front.inst);
	
	printf("cycle %d: RegWrite==%d, MemtoReg==%d, write_destination==%d", cycle, MEM_WB_buffer_front.control->RegWrite, MEM_WB_buffer_front.control->MemtoReg, MEM_WB_buffer_front.write_destination);
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
		printf(", write_data==%d", write_data);
	}
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
	printf("!");
	fflush(stdout);

	readInput_initialize();
	
	printf("!");
	fflush(stdout);

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
		//printf("%s\n", snapshotWriterBuffer[4]);

		stageReturnValue[1] = stage_memory();
		//printf("%s\n", snapshotWriterBuffer[3]);

		stageReturnValue[2] = stage_execute();
		//printf("%s\n", snapshotWriterBuffer[2]);

		stageReturnValue[3] = stage_decode();
		//printf("%s\n", snapshotWriterBuffer[1]);

		stage_fetch();
		//printf("%s\n", snapshotWriterBuffer[0]);


		printf("\ncycle %d: \n", cycle);

		printf("ID : ");
		print_dissembled_inst(IF_ID_buffer_front.inst);
		printf("EX : ");
		print_dissembled_inst(ID_EX_buffer_front.inst);
		printf("MEM : ");
		print_dissembled_inst(EX_MEM_buffer_front.inst);
		printf("WB : ");
		print_dissembled_inst(MEM_WB_buffer_front.inst);


		trigger();


		for(int i=0 ; i<5 ; i++){
			fprintf(snapshot, "%s\n", snapshotWriterBuffer[i]);
		}

		fprintf(snapshot, "\n\n");
		if( needTermination(stageReturnValue) || cycle > 600000) break;

		fflush(stdout);
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
	IF_ID_buffer_front.control = getEmptyCtrUnit();
	ID_EX_buffer_front.control = getEmptyCtrUnit();
	EX_MEM_buffer_front.control = getEmptyCtrUnit();
	MEM_WB_buffer_front.control = getEmptyCtrUnit();
	IF_ID_buffer_back.control = getEmptyCtrUnit();
	ID_EX_buffer_back.control = getEmptyCtrUnit();
	EX_MEM_buffer_back.control = getEmptyCtrUnit();
	MEM_WB_buffer_back.control = getEmptyCtrUnit();
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