#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>




/* CPU regiseters */
unsigned int R0;
unsigned int R1;
unsigned int R2;
unsigned int R3;
unsigned int R4;
unsigned int R5;
unsigned int R6;
unsigned int R7;
unsigned int R8;
unsigned int R9;
unsigned int R10;
unsigned int R11;
unsigned int R12;
unsigned int R13;
unsigned int R14;
unsigned int R15;



/* Prog -  Counter */
unsigned int pc;

/* Stack - Pointer */
unsigned int sp;

/* Base - Pointer */
unsigned int bp = 600;

unsigned int ax;

unsigned int bx;

unsigned int cx;

/* block of 1024 bytes - Memory */
unsigned char memory[1024];

/* Endian -- Type*/
int endian_type = 1;

/* Flags -- Register */
unsigned int flag;

/* Flag reg bits position */
enum flags_index{
     CF = 0, /* Carry- Flag */
     PF = 2, /* Parity- Flag */
     ZF = 4, /* Zero -Flag */
     SF = 6, /* Sign -Flag */
     OF = 8, /* Overflow -Flag */
     IN   /*  Invalid */
};

char *flagArray[IN+1] = {"CF","","PF","","ZF","","SF","", "OF"};

/* Opcodes */
enum OPCODE{

	STORE_MEMORY 	= 3,LOAD_MEMORY 	= 4,LOAD_MEMORY_I 	,MOV		,ADD		,
SUB	,MUL,DIV,MOD,PUSH,POP,ADDI ,ADDT ,SUBI,
MULI,DIVI,MODI,JMP,JNE,JNC,JC,JNO,JO ,JNZ,JZ,CMPI  ,AND ,
OR ,XOR  ,SHR  ,SHL ,LEA ,CMP ,JA,CMPV,JAE ,DEC
};


#define lbls	10
typedef struct label_sc{
	char 		name[10];
	unsigned int 	address;
}label_sc;
label_sc	labelContents[lbls];
label_sc	labelContentsPen[lbls];
static unsigned int labelContentsCom = 0;

#define BASE_ADDR_STACK 		900
#define TOP_ADDR_STACK 			1024
#define STACK_MEMORY_BASE 	900
#define BASE_ADDR	400
#define BASE_ADDR_INST 	100
#define BASE_BOOT_MEM 	0


enum CPURegisterIndex{

	R0Ind =0,		R1Ind,	R2Ind,	R3Ind,
	R4Ind,		R5Ind,	R6Ind,	R7Ind,
	R8Ind,		R9Ind,	R10Ind,	R11Ind,
	R12Ind,		R13Ind,	R14Ind,	R15Ind,
	Index_BP = 32,		Index_SP,	Index_PC,	Index_AX,
	Index_BX = 36,		Index_CX,       Index_INVALID
};



unsigned int labelInclusion(char *name, unsigned int address)
{

	if(name != NULL)
		strcpy(labelContents[labelContentsCom].name, name);
	else
		return 0;

	labelContents[labelContentsCom].address = address;

	labelContentsCom++;
	return 1;
}

unsigned int addrLabel(char *label)
{
	unsigned int index;
	unsigned int address = 0;


	if (label == NULL)
		return 0;

	for (index = 0; index < lbls && index < labelContentsCom; index++)
	{
		if(!strncasecmp(labelContents[index].name, label, 2))
		{
			address = labelContents[index].address;
			break;
		}
	}

	return address;
}

static unsigned int labelContentsPenCom = 0;

unsigned int addLabelsPen(char *name, unsigned int address)
{

	if(name != NULL)
	{
		strcpy(labelContentsPen[labelContentsPenCom].name, name);
		labelContentsPen[labelContentsPenCom].address = address;
	}
	else
		return 0;

	labelContentsPenCom++;

	return 1;
}

unsigned int getContfromTab(char *name, unsigned int *address)
{
	static unsigned int current_index = 0;
	if(current_index < labelContentsPenCom)
	{
		strcpy(name, labelContentsPen[current_index].name );
		*address = labelContentsPen[current_index].address;
		current_index ++;
	}
	else
		return 0;

	return 1;
}


int flagContentCheck(unsigned int flag_index)
{
	if(flag & (1 << flag_index))
		return 1;
	else
		return 0;
}
//set flag for ALU operation
void setFlag(unsigned int flag_index)
{
	if (flag_index < IN)
		flag |= 1 << flag_index;
	return;
}
//clear flag for alu operation
void clearFlag(unsigned int flag_index)
{
	if (flag_index < IN)
		flag &= ~(1 << flag_index);
	return;
}

int printRegMemContents(void)
{
	int i;
	printf("\n The value of general purpose Registers are shown below General Purpose Registers\n\n");
	printf("R0 = %u\t\tR1 = %u\t\tR2 = %u\t\tR3 = %u\n", R0, R1, R2, R3);
	printf("R4 = %u\t\tR5 = %u\t\tR6 = %u\t\tR7 = %u\n", R4, R5, R6, R7);
	printf("R8 = %u\t\tR9 = %u\t\tR10 = %u\t\tR11 = %u\n", R8, R9, R10, R11);
	printf("R12 = %u\t\tR13 = %u\t\tR14 = %u\t\tR15 = %u\n", R12, R13, R14, R15);


	printf("\tPC = %u\n", pc+1);
	printf("\tSP = %u\n", sp);
	printf("\tBP = %u\tAX = %u\tBX = %u\tCX = %u\n", bp,ax,bx,cx);
	printf("\tflag = %u\n", flag);


	for (i = 0; i < IN ; i++)
	{
		if (i != 1 && i != 3  && i != 5 && i != 7)
		{
			if(flagContentCheck(i))
				printf("\t%s = 1", flagArray[i]);
			else
				printf("\t%s = 0", flagArray[i]);
		}
	}

		printf("\n\n\t Contents of Instruction Memory\n");
		printf("Address    \t\tMemory Contents\n");
		for (i = BASE_ADDR_INST; i < BASE_ADDR; i = i+16)
		{
		    printf("%06d   %02X %02X %02X %02X  ", i, memory[i], memory[i+1], memory[i+2], memory[i+3]);
		    printf("%02X %02X %02X %02X  ", memory[i+4], memory[i+5], memory[i+6], memory[i+7]);
		    printf("%02X %02X %02X %02X  ", memory[i+8], memory[i+9], memory[i+10], memory[i+11]);
		    printf("%02X %02X %02X %02X  \n", memory[i+12], memory[i+13], memory[i+14], memory[i+15]);
		}

		printf("\n\n\tContents of Data Memory\n");
		printf("Address    \t\tMemory\n");
		for (i = BASE_ADDR; i < BASE_ADDR_STACK; i = i+16)
		{
		    printf("%06d   %02X %02X %02X %02X  ", i, memory[i], memory[i+1], memory[i+2], memory[i+3]);
		    printf("%02X %02X %02X %02X  ", memory[i+4], memory[i+5], memory[i+6], memory[i+7]);
		    printf("%02X %02X %02X %02X  ", memory[i+8], memory[i+9], memory[i+10], memory[i+11]);
		    printf("%02X %02X %02X %02X  \n", memory[i+12], memory[i+13], memory[i+14], memory[i+15]);
		}
	}


int getRegContent(int reg)
{
	switch (reg)
	{
		case R0Ind: return R0;case R1Ind: return R1;case R2Ind: return R2;
		case R3Ind: return R3;case R4Ind: return R4;case R5Ind: return R5;
		case R6Ind: return R6;case R7Ind: return R7;case R8Ind: return R8;
		case R9Ind: return R9;case R10Ind: return R10;case R11Ind: return R11;
		case R12Ind: return R12;case R13Ind: return R13;case R14Ind: return R14;
		case R15Ind: return R15;case Index_SP: return sp;case Index_BP: return bp;
		case Index_PC: return pc;

		default: printf("\n error reading register");break;
	}


}


void setRegContent(int reg, int value)
{
	switch(reg)
	{
		case R0Ind:	R0 = value;	break;
		case R1Ind:	R1 = value;break;
		case R2Ind:	R2 = value;break;
		case R3Ind:R3 = value;break;
		case R4Ind:R4 = value;break;
		case R5Ind:	R5 = value;break;
		case R6Ind:	R6 = value;	break;
		case R7Ind:	R7 = value;break;
		case R8Ind:	R8 = value;break;
		case R9Ind:	R9 = value;break;
		case R10Ind:R10 = value;break;
		case R11Ind:R11 = value;break;
		case R12Ind:R12 = value;break;
		case R13Ind:R13 = value;break;
		case R14Ind:R14 = value;break;
		case R15Ind:R15 = value;break;
		case Index_BP:bp = value;break;
		case Index_SP:	sp = value;break;
		case Index_PC:	pc = value;break;
		case Index_AX:	ax = value;break;
		case Index_BX:	bx = value;break;
		case Index_CX:	cx = value;	break;

		default: printf("\n error writing to the registers");break;
		break;

	}

}

int binary(int num)
{
    int i;
    for( i=31; i>=0; i--)
    {
        if(i == 31)
        return ((num >> i) & 1);
    }

}

int carry(int x, int y, unsigned int addition)
{
    int c;
    if (addition > 2147483647)
    {
		setFlag(CF);
    }
    else
    {
		clearFlag(CF);
    }
}


int overflow(int x, int y, int z)
{
    if(x == 0 && y == 0)
    {
        if(z == 1)
        {
	    setFlag(OF);
        }
        else
        {
	    clearFlag(OF);
        }
    }

    else if(x == 1 && y == 1)
    {
        if(z == 0)
        {
	    setFlag(OF);
        }
        else
        {
	    clearFlag(OF);
        }
    }
    else
    {
	clearFlag(OF);
    }
}

int lsb(int num)
{
     int i;
    for( i=31; i>=0; i--)
    {
        if(i == 0)
        return ((num >> i) & 1);
    }
}

int add_int(int a, int b)
{
	unsigned int c =0;

	while (b != 0)
	{
		c = (a & b) ;

		a = a ^ b;

		b = c << 1;
		if (c != 0 && c != 2147483648) //set auxillary carry
		{
			setFlag(CF);
		}
	}

	if(a == 0)
		setFlag(ZF);

	if(c == 2147483648){ //set overflow
		setFlag(OF);

	}

	return a;
}


int complement(int a)
{
	return (~a);
}

void alu_store(int src, int dest)
{
	int data = 0;
	unsigned char *ptr = memory, *p;
	if (dest < 0) {
		printf("\n\t##Memory reserved for Bootstrap and Instruction Memory");
		return;
	}

	data = getRegContent(src);
	p = (unsigned char *)&data;

		ptr [BASE_ADDR + dest + 0] = p[3];
		ptr [BASE_ADDR + dest + 1] = p[2];
		ptr [BASE_ADDR + dest + 2] = p[1];
		ptr [BASE_ADDR + dest + 3] = p[0];


}

void alu_load(int src, int dest)
{
	int data = 0;
	unsigned char *ptr = memory, *p, *ptr2;
	if (src < 0) {
		printf("\n\t##Memory reserved for Bootstrap and Instruction memory");
		return;
	}

	p = (unsigned char *)&data;

		p[0] = ptr[BASE_ADDR + src + 3];
		p[1] = ptr[BASE_ADDR + src + 2];
		p[2] = ptr[BASE_ADDR + src + 1];
		p[3] = ptr[BASE_ADDR + src + 0];

	setRegContent(dest, data);
}

void alu_load_i(int src, int dest)
{
	setRegContent(dest, src);
}

void Alupush(int s1)
{
	int content = 0;
	unsigned char *push_pointer = memory, *p;

	if (sp == 897) {
		printf("Stack is full");

		return;
	}
	content = getRegContent(s1);
	sp -= 4;
	p = (unsigned char *)&content;

		push_pointer [sp + 0] = p[3];
		push_pointer [sp + 1] = p[2];
		push_pointer [sp + 2] = p[1];
		push_pointer [sp + 3] = p[0];

}

void Alupop(int d1)
{
	int content = 0;
	unsigned char *pop_pointer = memory, *p;
	if (sp == 1021) {
		printf("The stack is empty");
		return;
	}
	p = (unsigned char *)&content;

		p[0] = pop_pointer[sp + 3];
		p[1] = pop_pointer[sp + 2];
		p[2] = pop_pointer[sp + 1];
		p[3] = pop_pointer[sp + 0];

	setRegContent(d1, content);
	sp += 4;
}

void alu_add(int src, int des)
{
	int sum, x, y;
	int a = getRegContent(src); // MSB of a
	int b = getRegContent(des); // MSB of b
	sum = add_int(a,b);
	setRegContent(des,sum);
}

void alu_addt(int src, int des, int target)
{
	int sum;
	int a = getRegContent(src); // MSB  of a
	int b = getRegContent(des); // MSB  of b
	sum = add_int(a, b);
	setRegContent(target, sum);
}

void alu_addi(int src,int des)
{
	int a, b,sum;
	a = src;
	b = getRegContent(des);
	sum = add_int(a,b);
	setRegContent(des,sum);
}

int sub(int a, int b)
{
	int answer,difference;
	printf("%s a = %d b = %d\n", __FUNCTION__,a,b);
	answer = add_int(a, complement(b));

	return answer;
}
void alu_sub(int src, int des)
{

	int answer,a,b, diff;
	a = getRegContent(src);
	b = getRegContent(des);
	diff = sub(a,b);
	setRegContent(des,diff);
}

void alu_subi(int src, int des)
{

	int answer,a,b, diff;
	a = src;
	b = getRegContent(des);
	diff = b - a;
	setRegContent(des,diff);
}


int mul(int x, int y)
{
	int a = 0, b = 0, c = 0, mul = 0;
	a=x;
	b=y;
	while(a >= 1)		/*check for a - even/odd*/
	{
		if (a & 0x1)
		{
			mul = add_int(mul,b);
		}
		a = a>>1;		/*left - shift */
		b<<=1;			/*Right - shift*/
	}
	a = binary(a);
	b = binary(b);
	carry(a, b, mul);
	c = binary(mul);
	overflow(a, b, c);
	return mul;
}

void alu_mul(int src, int des)
{
	int a,b,prod;
	a = getRegContent(src);   /*read  : from register*/

	b = getRegContent(des);   /*read : from register*/
	prod = mul(a,b);
	setRegContent(des,prod);

}

void alu_muli(int src, int des)
{
int a,b,prod;
	a = src;   /*read from register*/
	b = getRegContent(des);   /*read from register*/
	prod = mul(a,b);
	setRegContent(des,prod);
}

int div_int(int a, int b)
{
int nr, dr, zf, sf;
nr = a;
dr = b;
if (nr == 0 || dr == 0)
	{
		zf = 1;
		setFlag(ZF);

	}


	else
	{
		zf = 0;
	}
	if (dr < 0 || nr < 0)
	{
		sf = 1;
		setFlag(SF);

	}
	else
	{
		sf = 0;

	}

	int q = 1, temp1, temp2;
	temp1 = dr;
	if ((nr != 0 && dr != 0) && (dr < nr))

	{
		while (((temp1 << 1) - nr) < 0)
		{
			temp1 = temp1 << 1;
			q = q << 1;
		}
		while ((temp1 + dr) <= nr)
		{
			temp1 = temp1 + dr;
			q = q + 1;
		}
	}

	if (dr)
	{

		b=q;
	}
	else
	{
		printf("ALERT...EXCEPTIOn\n");
	}



return b;
}

void alu_div(int src, int des)
{
	int nr, dr, ZF, SF, Quotient;
	nr = getRegContent(src);
	 dr = getRegContent(des);
	Quotient = div_int(nr,dr);
	setRegContent(des,Quotient);

}

void alu_divi(int src, int des)
{
int nr, dr, ZF, SF, Quotient;
	nr = src;
	 dr = getRegContent(des);
	Quotient = div_int(nr,dr);
	setRegContent(des,Quotient);

}

int mod_int(int x, int y)
{
	int nr,dr,remainder,n = 0;
	nr = x;
	dr = y;

	if (nr == 0 || dr == 0)
	{
		setFlag(ZF);
	}

	else
	{
		clearFlag(ZF);
	}
	if (dr < 0 || nr < 0)
	{
		setFlag(SF);
	}
	else
	{
		clearFlag(SF);

	}


	if (n>0 && dr == 2^n)
	{
		remainder = (nr & (dr - 1));
	}
	else
	{

		remainder = (nr - (dr * (nr/dr)));
	}

	return remainder;

}

int alu_mov(unsigned int src, unsigned int des) //right shift logical
{
    unsigned int a;
    a = getRegContent(src);
    setRegContent(des, a);
}

void alu_mod(int src, int des)
{
	int nr, dr, remainder;
	nr = getRegContent(src);
	dr = getRegContent(des);
	remainder = mod_int(nr,dr);
	setRegContent(des,remainder);

}

void alu_modi(int src, int des)
{
int nr,dr, remainder;
nr = src;
dr = getRegContent(des);
remainder= mod_int(nr,dr);
setRegContent(des,remainder);
}


void Alujmp(unsigned int destination)
{
	if(100<destination<400)
	{

		pc = destination-1;
		flag = 0;

	}
	else
		printf("Error");

}

void Alujne(unsigned int destination)
{
	if(!flagContentCheck(ZF))
	{
		if(100< destination <400)
		{

				pc = destination-1;

			flag = 0;
		}
		else
			printf("Error");
	}
}

void Alujnc(unsigned int destination)
{
	if(flagContentCheck(CF))
	{
		if(100<destination<400)
		{

				pc = destination-1;
			flag = 0;
		}
	}
}

void Alujc(unsigned int destination)
{
	if(flagContentCheck(CF)==1)
	{
		if(100<destination<400)
		{

				pc = destination-1;
			flag = 0;
		}
	}
}

void Alujno(unsigned int destination)
{
	if(flagContentCheck(OF)==0)
	{
		if(100<destination<400)
		{

				pc = destination-1;
			flag = 0;
		}
	}
}

void Alujo(unsigned int destination)
{
	if(flagContentCheck(OF)==1)
	{
		if(100<destination<400)
		{

				pc = destination-1;
			flag = 0;
		}
	}
}

void Alujnz(unsigned int destination)
{
	if(flagContentCheck(ZF)==0)
	{
		if(100<destination<400)
		{

				pc = destination-1;
			flag = 0;
		}
	}
}

void Alujz(unsigned int destination)
{
	if(flagContentCheck(ZF)==1)
	{
		if(100<destination<400)
		{

				pc = destination-1;
			flag = 0;
		}
	}
}

void Alujae(unsigned int destination)
{
	if(!(flagContentCheck(SF) ^ flagContentCheck(OF)) )
	{
		if(100<destination<400)
		{

				pc = destination-1;
			flag = 0;
		}
	}
}

void Aluja(unsigned int destination)
{

	if((!(flagContentCheck(CF))) & (!(flagContentCheck(OF))) )
	{
		if(100<destination<400)
		{

				pc = destination-1;
		flag = 0;
		}
	}
}

void Alucmpi(unsigned int source, unsigned int dest_reg)
{
	if (source == getRegContent(dest_reg))
	{
        setFlag(ZF);
    }
    else
    {
        clearFlag(ZF);
	}
}

void Alucmp(unsigned int source_reg, unsigned int dest_reg)
{
	int source_a = getRegContent(source_reg);
	int dest_b = getRegContent(dest_reg);
	sub(source_a,dest_b);
	if (source_a == dest_b)
	{
        setFlag(ZF);
    }
    else
    {
        clearFlag(ZF);
	}

	if (source_a < dest_b)
	{
        setFlag(SF);
	}
    else
    {
        clearFlag(SF);
	}

}

void Alucomp_v(unsigned int source, unsigned int dest_reg)
{
	int source_a = memory[getRegContent(source)];
	int dest_b = getRegContent(dest_reg);
	sub(source_a,dest_b);

	if (source_a == dest_b)
	{
        setFlag(ZF);
    }
    else
    {
        clearFlag(ZF);
	}

	if (source_a < dest_b)
	{
        setFlag(SF);
	}
    else
    {
        clearFlag(SF);
	}

}

int Aluand(int s1, int d1)
{
    int t1, t2, t3;
    t1 = getRegContent(s1);
    t2 = getRegContent(d1);
    t3 = t1 & t2;
    setRegContent(d1, t3);
}

int Aluor(int s1, int d1)
{
    int t1, t2, t3;
    t1 = getRegContent(s1);
    t2 = getRegContent(d1);
    t3 = t1 | t2;
    setRegContent(d1, t3);
}

int shiftrightAlu(unsigned int s1, unsigned int d1)
{
    int t1, t2;
    t1 = getRegContent(d1);
    t1 = t1 >> s1;
    if(t1 == 0)
    {
        setFlag(ZF);
    }
    else
    {
        clearFlag(ZF);
    }
    setRegContent(d1, t1);
}

int shiftleftAlu(unsigned int d1, unsigned int s1)
{
    int t1, t2;
    t1 = getRegContent(d1);
    t2 = binary(t1);
    if(t2 == 0)
    {
        clearFlag(CF);
    }
    else
    {
        setFlag(CF);
    }
    t1 = t1 << s1;
    setRegContent(d1, t1);
}

int Aluxor(int s1, int d1)
{
    int t1, t2, t3;
    t1 = getRegContent(s1);
    t2 = getRegContent(d1);
    t3 = t1 ^ t2;
    setRegContent(d1, t3);
}


void Alulea(int source_pointer, int dest_pointer)
{
	int res;
	unsigned char *ptr = memory;
	res= source_pointer + bp;
	if (dest_pointer > 37)
		*(int*)(ptr + BASE_ADDR + dest_pointer) = res;
	else
		setRegContent(dest_pointer, res);
}


void ALU(char opcode, int src, int dest, int target)
{
	switch(opcode)
	{
	case STORE_MEMORY:
		alu_store(src, dest);
	break;
	case LOAD_MEMORY:
		alu_load(src, dest);
	break;
	case LOAD_MEMORY_I:
		alu_load_i(src, dest);
	break;
	case PUSH:
		Alupush(src);
	break;
	case POP:
		Alupop(dest);
	break;
	case MUL:

		alu_mul(src, dest);
	break;
	case ADD:

		alu_add(src, dest);
	break;
	case DIV:

		alu_div(src, dest);
	break;
	case MOD:
		alu_mod(src,dest);
	break;
	case MOV:
		alu_mov(src,dest);
	break;
	case SUB:
		alu_sub(src,dest);
	break;
	case ADDI:
		alu_addi(src, dest);
	break;
	case ADDT:
		alu_addt(src, dest, target);
	break;
	case SUBI:
		alu_subi(src,dest);
	break;
	case MULI:

		alu_muli(src, dest);
	break;
	case DIVI:

		alu_divi(src, dest);
	break;
	case MODI:
		alu_modi(src,dest);
	break;
    case JMP:
		Alujmp(dest);
	break;
    case JNE:
		Alujne(dest);
	break;
	case JNC:
		Alujnc(dest);
	break;
	case JC:
		Alujc(dest);
	break;
	case JNZ:
		Alujnz(dest);
	break;
	case JZ:
		Alujz(dest);
	break;
	case JNO:
		Alujno(dest);
	break;
	case JO:
		Alujo(dest);
	break;
    case JA:
	    Aluja(dest);
	break;
    case JAE:
	    Alujae(dest);
	break;
	case AND:
	    Aluand(src, dest);
	break;
	case OR:
	    Aluor(src, dest);
	break;
	case XOR:
	    Aluxor(src, dest);
	break;
	case SHR:
	    shiftrightAlu(src, dest);
	break;
    case SHL:
	    shiftleftAlu(src, dest);
	break;
    case CMPI:
	    Alucmpi(src, dest);
	break;
    case CMP:
	    Alucmp(src, dest);
	break;
    case CMPV:
	    Alucomp_v(src, dest);
	break;
	case LEA:
		Alulea(src, dest);
	break;
	default:
		break;
	}

}


/* Return index of a passed reg string*/
int returnIndex(char *ptr)
{
	char i = 0;
	i = atoi(++ptr);

	return i;
}

int return_offset(char *ptr)
{
	uint16_t i = 0;
	i = atoi(&ptr[7]);
	return i;
}

int executeInstruction()
{
	unsigned char opcode = 0, dst = 0, src = 0, target = 0;

		opcode = memory[pc ];
		src = memory[pc - 1];
		dst = memory[pc - 2];
		target = memory[pc -3];


	if(opcode == 0)
	{
		return 0;
	}
	printf("\nExecuting Instruction number %d : %u %u %u", (pc - 127)/4,  opcode, src, dst);
	ALU(opcode, src, dst, target);
	pc = pc + 4;
	return 1;
}



int loader()
{

	char *instruction;
			FILE *inputFile = 0;
			size_t length = 0;
			char *opcodePointer = NULL, *sourcePointer = NULL, *destPointer = NULL;
			unsigned int address, jmp_address, label_address;
			int i = 0, count = 0, n = -1;
			unsigned char opcode = 0, dst = 0, src = 0, target = 0;
			instruction = (char *) malloc(100);	// instruction pointer - holds address of MEM_HOLDER allocated dynamically .
			inputFile = fopen("./instructions.txt", "r");// Open a text file to fetch instruction

			char *label;



	while( (n = getline(&instruction, &length, inputFile)) != -1)
	{
		/*"Here parse the OPCODE, OPERANDS and perform corresponding operations"*/
		printf("\n\tInstruction = \t %s", instruction);

		opcodePointer = strtok(instruction, " ");
		if (!strcasecmp(opcodePointer, "ld")) {
			sourcePointer = strtok(NULL, ",");	     /* loads from mem loc to reg*/
			src = atoi(sourcePointer) - BASE_ADDR;
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode = LOAD_MEMORY;
		} else if (!strcasecmp(opcodePointer, "ldi")) {
			sourcePointer = strtok(NULL, ",");	     /* loads from mem loc to reg*/
			src = atoi(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode = LOAD_MEMORY_I;
		} else if (!strcasecmp(opcodePointer, "st")) {
			sourcePointer = strtok(NULL, ",");
			src = returnIndex(sourcePointer);
			destPointer = strtok(NULL, " ");		/* loads from Reg to loc*/
			dst = atoi(destPointer) - BASE_ADDR;
			opcode =  STORE_MEMORY;
		} else if (!strcasecmp(opcodePointer, "mov")) {
			sourcePointer = strtok(NULL, ",");	     /* loads from mem loc to reg*/
			src = returnIndex(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);

			opcode = MOV;
		} else if (!strcasecmp(opcodePointer, "mul")) {
			sourcePointer = strtok(NULL, ",");	     /*  loads from mem loc to reg*/
			src = returnIndex(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);

			opcode = MUL;
		}
		else if (!strcasecmp(opcodePointer, "sub")) {
			sourcePointer = strtok(NULL, ",");	     /* loads from mem loc to reg*/
			src = returnIndex(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode = SUB;
		}
		else if (!strcasecmp(opcodePointer, "add")) {
			sourcePointer = strtok(NULL, ",");	     /* loads from mem loc to reg*/
			src = returnIndex(sourcePointer);		/*  load div instruction*/
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode = ADD;
		}
		else if (!strcasecmp(opcodePointer, "addt")) {
			sourcePointer = strtok(NULL, ",");	     /* load from mem loc to register*/
			target = returnIndex(sourcePointer);		/*  loads div instruction*/
			destPointer = strtok(NULL, " ");
			src = returnIndex(destPointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode = ADDT;
		}
		else if (!strcasecmp(opcodePointer, "lea")) {
			sourcePointer = strtok(NULL, ",");	     //Instruction : Load_Effective_Address(LEA)

			if (!strcasecmp(sourcePointer,"ax"))
				dst = Index_AX;
			else if (!strcasecmp(sourcePointer,"bx"))
				dst = Index_BX;
			else if (!strcasecmp(sourcePointer,"cx"))
				dst = Index_CX;
			else
				dst = atoi(sourcePointer) - BASE_ADDR;
			destPointer = strtok(NULL, "]");
			src = return_offset(destPointer);
			opcode = LEA;
		}

		else if (!strcasecmp(opcodePointer, "mod")) {
			sourcePointer = strtok(NULL, ",");	     /*load from mem loc to reg*/
			src = returnIndex(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);

			opcode = MOD;
		}
		else if (!strcasecmp(opcodePointer, "div")) {
			sourcePointer = strtok(NULL, ",");
			src = returnIndex(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);

			opcode = DIV;

		}
		else if (!strcasecmp(opcodePointer, "addi")) {
			sourcePointer = strtok(NULL, ",");
			src = atoi(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode =  ADDI;
		}
		else if (!strcasecmp(opcodePointer, "subi")) {
			sourcePointer = strtok(NULL, ",");
			src = atoi(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode =  SUBI;
		}
		else if (!strcasecmp(opcodePointer, "muli")) {
			sourcePointer = strtok(NULL, ",");
			src = atoi(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode =  MULI;
		}
		else if (!strcasecmp(opcodePointer, "divi")) {
			sourcePointer = strtok(NULL, ",");
			src = atoi(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode =  DIVI;
		}
		else if (!strcasecmp(opcodePointer, "modi")) {
			sourcePointer = strtok(NULL, ",");
			src = atoi(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode =  MODI;
		}
		else if (!strcasecmp(opcodePointer, "push")) {
			sourcePointer = strtok(NULL, " ");
			if(*sourcePointer == 'R')
				src = returnIndex(sourcePointer);
			else if (strstr(sourcePointer,"bp"))
				src = Index_BP;
			else if (strstr(sourcePointer,"sp"))
				src = Index_SP;
			else if (strstr(sourcePointer,"pc"))
				src = Index_PC;

			opcode =  PUSH;

			dst = Index_INVALID;
		}
		else if (!strcasecmp(opcodePointer, "pop")) {
			sourcePointer = strtok(NULL, " ");
			if(*sourcePointer == 'R')
				dst = returnIndex(sourcePointer);
			else if (strstr(sourcePointer,"bp"))
				dst = Index_BP;
			else if (strstr(sourcePointer,"sp"))
				dst = Index_SP;
			else if (strstr(sourcePointer,"pc"))
				dst = Index_PC;

			opcode =  POP;

			src = Index_INVALID;
		}
		else if (!strcasecmp(opcodePointer, "jmp")) {
			sourcePointer = strtok(NULL, " ");

			dst = addrLabel(sourcePointer);

			if(dst == 0)
			{
				addLabelsPen(sourcePointer, BASE_ADDR_INST + i);
			}
			opcode =  JMP;
			src = Index_INVALID;

		}
		else if (!strcasecmp(opcodePointer, "jne")) {
			sourcePointer = strtok(NULL, " ");

			dst = addrLabel(sourcePointer);

			if(dst == 0)
			{
				addLabelsPen(sourcePointer, BASE_ADDR_INST + i);
			}
			opcode =  JNE;
			src = Index_INVALID;

		}
		else if (!strcasecmp(opcodePointer, "jnc")) {
			sourcePointer = strtok(NULL, " ");

			dst = addrLabel(sourcePointer);

			if(dst == 0)
			{
				addLabelsPen(sourcePointer, BASE_ADDR_INST + i);
			}
			opcode =  JNC;
			src = Index_INVALID;

		}
		else if (!strcasecmp(opcodePointer, "jc")) {
			sourcePointer = strtok(NULL, " ");

			dst = addrLabel(sourcePointer);

			if(dst == 0)
			{
				addLabelsPen(sourcePointer, BASE_ADDR_INST + i);
			}
			opcode =  JC;
			src = Index_INVALID;

		}
		else if (!strcasecmp(opcodePointer, "jnz")) {
			sourcePointer = strtok(NULL, " ");

			dst = addrLabel(sourcePointer);

			if(dst == 0)
			{
				addLabelsPen(sourcePointer, BASE_ADDR_INST + i);
			}
			opcode =  JNZ;
			src = Index_INVALID;

		}
		else if (!strcasecmp(opcodePointer, "jz")) {
			sourcePointer = strtok(NULL, " ");

			dst = addrLabel(sourcePointer);

			if(dst == 0)
			{
				addLabelsPen(sourcePointer, BASE_ADDR_INST + i);
			}
			opcode =  JZ;
			src = Index_INVALID;

		}
		else if (!strcasecmp(opcodePointer, "jno")) {
			sourcePointer = strtok(NULL, " ");

			dst = addrLabel(sourcePointer);

			if(dst == 0)
			{
				addLabelsPen(sourcePointer, BASE_ADDR_INST + i);
			}
			opcode =  JNO;
			src = Index_INVALID;

		}
		else if (!strcasecmp(opcodePointer, "jo")) {
			sourcePointer = strtok(NULL, " ");

			dst = addrLabel(sourcePointer);

			if(dst == 0)
			{
				addLabelsPen(sourcePointer, BASE_ADDR_INST + i);
			}
			opcode =  JO;
			src = Index_INVALID;
		}
		else if (!strcasecmp(opcodePointer, "ja")) {
			sourcePointer = strtok(NULL, " ");
			dst = addrLabel(sourcePointer);
			if(dst == 0)
			{
				addLabelsPen(sourcePointer, BASE_ADDR_INST + i);
			}
			opcode =  JA;
			src = Index_INVALID;
		}
		else if (!strcasecmp(opcodePointer, "jae")) {
			sourcePointer = strtok(NULL, " ");
			dst = addrLabel(sourcePointer);
			if(dst == 0)
			{
				addLabelsPen(sourcePointer, BASE_ADDR_INST + i);
			}
			opcode =  JAE;
			src = Index_INVALID;
		}
		else if (!strcasecmp(opcodePointer, "and")) {
		    sourcePointer = strtok(NULL, ",");
			src = returnIndex(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);

			opcode = AND;
		}
		else if(!strcasecmp(opcodePointer, "or")) {
		    sourcePointer = strtok(NULL, ",");
			src = returnIndex(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);

			opcode = OR;
		}
		else if(!strcasecmp(opcodePointer, "xor")) {
		    sourcePointer = strtok(NULL, ",");
			src = returnIndex(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);

			opcode = XOR;
		}
		else if(!strcasecmp(opcodePointer, "shr")) {
		    sourcePointer = strtok(NULL, ",");
		    src = atoi(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode = SHR;
		}
		else if(!strcasecmp(opcodePointer, "shl")) {
		    src = atoi(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);

			opcode = SHL;
		}
		else if (!strcasecmp(opcodePointer, "cmpi")) {
			sourcePointer = strtok(NULL, ",");
			src = atoi(sourcePointer);
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode = CMPI;
		}
		else if (!strcasecmp(opcodePointer, "cmp")) {
			sourcePointer = strtok(NULL, ",");
			src = returnIndex(sourcePointer);;
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);
			opcode = CMP;
		}
		else if (!strcasecmp(opcodePointer, "cmpv")) {
			sourcePointer = strtok(NULL, "]");

			src = returnIndex(sourcePointer + 1);
			sourcePointer = strtok(NULL, " ");
			destPointer = strtok(NULL, " ");
			dst = returnIndex(destPointer);

			opcode = CMPV;
		}
		else if (!strcasecmp(opcodePointer, "dec")) {
			sourcePointer = strtok(NULL, " ");
			src = returnIndex(sourcePointer);
			opcode = DEC;
		}
		else if (*opcodePointer, "L") {
			sourcePointer = strtok(opcodePointer, ":");
			labelInclusion(sourcePointer, BASE_ADDR_INST + i);
			opcodePointer = strtok(NULL, " ");
			continue;


		}
		else {
			usage();
		}


			memory[BASE_ADDR_INST + i] = target;
			i++;
			memory[BASE_ADDR_INST + i] = dst;
			i++;
			memory[BASE_ADDR_INST + i] = src;
			i++;
			memory[BASE_ADDR_INST + i] = opcode;
			i++;

	       	count++;
		opcodePointer = strtok(NULL, " ");
	}

	label = (char *)malloc(10);
	while(getContfromTab(label, &jmp_address))
	{
		label_address = addrLabel(label);

			memory[jmp_address + 1] = label_address;

	}

	return count;
}

int main(int argc, char **argv)
{


	int size = 0,i;
	    FILE *myFile;
		char ch,
		flag = 0;unsigned int valueMax = -1;
		char *pointer = NULL;valueMax = valueMax / 2;
		int noOfInstrcutions = 0;
		pointer = (char *) &valueMax;
		//little enddian to load memory
		memory [384 + 0] = pointer[3];
		memory [384 + 1] = pointer[2];
		memory [384 + 2] = pointer[1];
		memory [384 + 3] = pointer[0];


		pc = BASE_ADDR_INST + 3;
		sp = TOP_ADDR_STACK;
		flag = 0;
		// memory for labels allocation
		memset(labelContents, 0, sizeof(labelContents));
		memset(labelContentsPen, 0, sizeof(labelContentsPen));
		noOfInstrcutions = loader();
//inputnos.txt file contains numbers for binary search
    myFile = fopen("inputnos.txt", "r");
    fscanf(myFile, "%d", &R4);
	while(fscanf(myFile, "%d", &memory[i]) > 0) {
        i++;
    }
    fclose(myFile);

    R3 = i;
    R2 = 0;


    for (i = 0; i < R3; i++)
    {
        printf("%d ", memory[ i]);
    }
    //Exec each instruction... 
printf("\n Contents of Registers and Memory before the execution of instructions");
	printRegMemContents();

	while (executeInstruction()) {
		printRegMemContents();
}
	printf("\n Contents of Registers and Memory after the execution of instructions");

	printRegMemContents();

}

