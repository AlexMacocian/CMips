#include <stdint.h>
#include "stdio.h"
#include "sys/file.h"
#include "string.h"
#include "ctype.h"
#include "limits.h"
#include "stdlib.h"
#include "LinkedList.h"

#define BINARY 0
#define ASSEMBLY 1
#define rs_mask 0b00000011111000000000000000000000 
#define rt_mask 0b00000000000111110000000000000000
#define rd_mask 0b00000000000000001111100000000000
#define op_mask 0b11111100000000000000000000000000
#define im_mask 0b00000000000000001111111111111111
#define ad_mask 0b00000011111111111111111111111111
#define sh_mask 0b00000000000000000000011111000000
#define fc_mask 0b00000000000000000000000000111111
#define AND 0
#define OR 1
#define ADD 2
#define SUB 3
#define SLL 4
#define SRL 5
#define XOR 6
#define SLT 7
#define zero 0
#define at 1
#define v0 2
#define v1 3
#define a0 4
#define a1 5
#define a2 6
#define a3 7
#define t0 8
#define t1 9
#define t2 10
#define t3 11
#define t4 12
#define t5 13
#define t6 14
#define t7 15
#define s0 16
#define s1 17
#define s2 18
#define s3 19
#define s4 20
#define s5 21
#define s6 22
#define s7 23
#define t8 24
#define t9 25
#define k0 26
#define k1 27
#define gp 28
#define sp 29
#define fp 30
#define ra 31

//Variables
uint8_t loadType = 0;
uint8_t verbose = 0;
uint8_t program_counter = 0;
uint8_t brk = 0;
uint8_t stop = 0;
int32_t result = 0;
uint8_t ram_init = 0;
uint32_t ram_size = 1000000;
FILE *im, *dm, *rm;
FILE **files;

//Register types
typedef struct __32BitReg{
    union {
        uint32_t data;
        struct{
            uint32_t bit1 : 1;
            uint32_t bit2 : 1;
            uint32_t bit3 : 1;
            uint32_t bit4 : 1;
            uint32_t bit5 : 1;
            uint32_t bit6 : 1;
            uint32_t bit7 : 1;
            uint32_t bit8 : 1;
            uint32_t bit9 : 1;
            uint32_t bit10 : 1;
            uint32_t bit11 : 1;
            uint32_t bit12 : 1;
            uint32_t bit13 : 1;
            uint32_t bit14 : 1;
            uint32_t bit15 : 1;
            uint32_t bit16 : 1;
            uint32_t bit17 : 1;
            uint32_t bit18 : 1;
            uint32_t bit19 : 1;
            uint32_t bit20 : 1;
            uint32_t bit21 : 1;
            uint32_t bit22 : 1;
            uint32_t bit23 : 1;
            uint32_t bit24 : 1;
            uint32_t bit25 : 1;
            uint32_t bit26 : 1;
            uint32_t bit27 : 1;
            uint32_t bit28 : 1;
            uint32_t bit29 : 1;
            uint32_t bit30 : 1;
            uint32_t bit31 : 1;
            uint32_t bit32 : 1;
        };
    };
} __32BitReg;
typedef struct __3BitReg{
    union {
        uint8_t data;
        struct{
            uint8_t bit1 : 1;
            uint8_t bit2 : 1;
            uint8_t bit3 : 1;
        };
    };
} __3BitReg;
typedef struct __ALUFlagReg{
    union {
        uint8_t data;
        struct{
            uint8_t z : 1;
            uint8_t v : 1;
            uint8_t c : 1;
        };
    };
} __ALUFlagReg;
typedef struct __5BitReg{
    union {
        uint8_t data;
        struct{
            uint8_t bit1 : 1;
            uint8_t bit2 : 1;
            uint8_t bit3 : 1;
            uint8_t bit4 : 1;
            uint8_t bit5 : 1;
        };
    };
} __5BitReg;

//Memory area
__32BitReg Instruction_Memory[32];
__32BitReg Data_Memory[32];
__32BitReg *Memory;
__32BitReg ALU_a, ALU_b, ALU_out, hi, lo;
__ALUFlagReg ALU_Flags;
__3BitReg ALU_op;

void PrintHelp();
void ALU();
int GetLengthOfLine(char * string);
int GetDataRegAddressFromString(char* string);
void LoadFromAssembly();
void LoadInstructionMemory();
void LoadDataMemory();
void LoadRAM();
char** Tokenize(char* line);
void WriteDataMemory();
void WriteRAM();
void Print32BitRegister(__32BitReg *reg);
void ProcessorRun();
void ProcessInstruction(__32BitReg *instruction);
int IsInstruction(char *line);

int main(int argc, char** argv){
    program_counter = 0;
    im = fopen("in.bin", "w");
    dm = fopen("dm.bin", "w");
    rm = fopen("ram.bin", "w");
    if(argc == 1){
        printf("To use this simulator, please pass the instruction memory file in binary format\n");
        printf("Example:\n");
        printf("mips -i <file>");
        printf("Use mips --help or -h for more information");
    }
    if(argc > 1){
        for(int i = 1; i < argc; i++){
            char* arg = argv[i];
            if(strncmp(arg, "-h", 2) == 0 || strncmp(arg, "--help", 6) == 0){
                PrintHelp();
                return 0;
            }
            else if(strncmp(arg, "-i", 2) == 0){
                if(im != NULL) {
                    fclose(im);
                }
                im = fopen(argv[i + 1], "r");
                i++;
                loadType == BINARY;
            }
            else if(strncmp(arg, "-d", 2) == 0){
                if(dm != NULL) {
                    fclose(dm);
                }
                dm = fopen(argv[i + 1], "w+");
                fseek(dm, 0, SEEK_SET);
                i++;
            }
            else if(strncmp(arg, "-ms", 3) == 0){
                ram_size = strtol(argv[i + 1], '\0', 10);
                i++;
            }
            else if(strncmp(arg, "-m", 2) == 0){
                if(rm != NULL) {
                    fclose(rm);
                }
                rm = fopen(argv[i + 1], "w+");
                fseek(rm, 0, SEEK_SET);
                i++;
            }
            else if(strncmp(arg, "-a", 2) == 0){
                if(im != NULL) {
                    fclose(im);
                }
                im = fopen(argv[i + 1], "r");
                i++;
                loadType = ASSEMBLY;
            }
            else if(strncmp(arg, "-v", 2) == 0){
                verbose = 1;
            }
        }
    }
    LoadRAM();
    LoadDataMemory();
    if(loadType == BINARY){
        LoadInstructionMemory();
    }
    else if(loadType == ASSEMBLY){
        LoadFromAssembly();
    }
    if(ram_init == 0){
        return -2;
    }
    if(verbose){
        printf("\nRAM initialized\n");
    }
    Data_Memory[0].data = 0;
    if(verbose > 0){
        printf("\nMachine initialized.\nStarting processing instructions.\n");
    }
    ProcessorRun();
    if(verbose){
        printf("\nMachine finished processing.\n");
        printf("Writing data memory to file\n");
    }
    WriteDataMemory();
    if(verbose){
        printf("Writing RAM to file\n");
    }
    WriteRAM();
    fclose(dm);
    fclose(im);
    if(verbose){
        printf("\nClosing\n");
    }
    return result;
}

void PrintHelp(){
    printf("Usage: \n");
    printf("mips [-options] <arguments>\n\n");
    printf("Options:\n");
    printf("-i <file> \tLoads an instruction memory file in binary format\n");
    printf("-d <file> \tLoads a data memory file in binary format. This file will be overwritten when the execution ends with new data\n");
    printf("-v \t\tVerbose mode. Simulator will output all information available\n");
    printf("-ms \t\tRAM size. Simulator will set the number of RAM memory bytes to the specified value. If value is not set, the simulator will allocate a default size of 1000000\n");
    printf("-m <file> \tLoads into the RAM the values read from the specified file\n");
    printf("-a <file> \tLoads an assembly file to be parsed.\n");
    printf("Use either -i or -a to initialize the instruction memory\n");
}

void ALU(){
    ALU_out.data = 0;
    ALU_Flags.data = 0;
    switch(ALU_op.data){
        case 0://AND
        ALU_out.data = ALU_a.data & ALU_b.data;
        break;
        case 1://OR
        ALU_out.data = ALU_a.data | ALU_b.data;
        break;
        case 2://ADD
        ALU_out.data = ALU_a.data + ALU_b.data;
        if(ALU_a.bit32 == 0 && ALU_b.bit32 == 0 && ALU_out.bit32 == 1){//IF BOTH NUMBERS POSITIVE AND RESULT NEGATIVE
            ALU_Flags.v = 1;
        }
        else if(ALU_a.bit32 == 1 && ALU_b.bit32 == 1 && ALU_out.bit32 == 0){//IF BOTH NUMBERS NEGATIVE AND RESULT POSITIVE
            ALU_Flags.v = 1;
        }
        break;
        case 3://SUB
        ALU_out.data = ALU_a.data - ALU_b.data;
        if(ALU_a.bit32 == 1 && ALU_b.bit32 == 0 && ALU_out.bit32 == 0){//IF A NEGATIVE AND B POSITIVE AND RESULT POSITIVE
            ALU_Flags.v = 1;
        }
        else if(ALU_a.bit32 == 0 && ALU_b.bit32 == 1 && ALU_out.bit32 == 1){//IF A POSITIVE AND B NEGATIVE AND RESULT NEGATIVE
            ALU_Flags.v = 1;
        }
        break;
        case 4://SLL
        ALU_out.data = ALU_a.data << ALU_b.data;
        ALU_Flags.v = 1;
        break;
        case 5://SRL
        ALU_out.data = ALU_a.data >> ALU_b.data;
        ALU_Flags.v = 1;
        break;
        case 6://XOR
        ALU_out.data = ALU_a.data ^ ALU_b.data;
        break;
        case 7://SLT
        if(ALU_a.data < ALU_b.data){
            ALU_out.data = 1;
        }
        else{
            ALU_out.data = 0;
        }
        break;
    }
    if(ALU_out.data == 0){
        ALU_Flags.z = 1;
    }
}

void LoadFromAssembly(){
    for(int i = 0; i < 32; i++){
        Instruction_Memory[i].data = 0;
    }
    if(verbose){
        printf("Loading from assembly file\n");
    }
    char line[256];
    int lineIndex = 0;
    int instr_count = 0;
    if(im){
        do{//GET LABELS
            fgets(line, 256, im);
            fflush(stdout);
            if(IsInstruction(line)){
                instr_count++;
                if(verbose){
                    printf("Found instruction: %s\n", line);
                }
            }
            else{
                if(strstr(line, ":")){
                    char *label = strtok(line, ":");
                    _node *node = CreateNode(instr_count, label);
                    InsertNode(node, 1);
                    if(verbose){
                        printf("Found label %s. Inserting in list with value %d\n", label, instr_count);
                    }
                }
            }
            lineIndex++;
        }while(!feof(im));
        fseek(im, 0, SEEK_SET);
        lineIndex = 0;
        instr_count = 0;
        do{//PARSE LINES
            fgets(line, 256, im);
            char ** tokens = Tokenize(line);
            char *instr = tokens[0];
            if(strcmp(instr, "add") == 0){
                char *rd = tokens[1];
                char *rs = tokens[2];
                char *rt = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b100000;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "addu") == 0){
                char *rd = tokens[1];
                char *rs = tokens[2];
                char *rt = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b100001;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "and") == 0){
                char *rd = tokens[1];
                char *rs = tokens[2];
                char *rt = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b100100;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "break") == 0){
                Instruction_Memory[instr_count].data = 0b001101;
                instr_count++;
            }
            else if(strcmp(instr, "div") == 0){
                char *rs = tokens[1];
                char *rt = tokens[2];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += 0b011010;
                instr_count++;
                if(verbose){
                    printf("%s %s %s\n", instr, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "divu") == 0){
                char *rs = tokens[1];
                char *rt = tokens[2];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += 0b011011;
                instr_count++;
                if(verbose){
                    printf("%s %s %s\n", instr, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "jalr") == 0){
                char *rd = tokens[1];
                char *rs = tokens[2];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);;
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b001001;
                instr_count++;
                if(verbose){
                    printf("%s %s %s\n", instr, rd, rs);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "jr") == 0){
                char *rs = tokens[1];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += 0b001000;
                instr_count++;
                if(verbose){
                    printf("%s %s %s\n", instr, rs);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "mfhi") == 0){
                char *rd = tokens[1];
                Instruction_Memory[instr_count].data = 0;
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b010000;
                instr_count++;
                if(verbose){
                    printf("%s %s\n", instr, rd);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "mflo") == 0){
                char *rd = tokens[1];
                Instruction_Memory[instr_count].data = 0;
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b010010;
                instr_count++;
                if(verbose){
                    printf("%s %s\n", instr, rd);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "mthi") == 0){
                char *rs = tokens[1];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += 0b010001;
                instr_count++;
                if(verbose){
                    printf("%s %s\n", instr, rs);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "mtlo") == 0){
                char *rs = tokens[1];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += 0b010011;
                instr_count++;
                if(verbose){
                    printf("%s %s\n", instr, rs);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "mult") == 0){
                char *rs = tokens[1];
                char *rt = tokens[2];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += 0b011000;
                instr_count++;
                if(verbose){
                    printf("%s %s %s\n", instr, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "multu") == 0){
                char *rs = tokens[1];
                char *rt = tokens[2];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += 0b011001;
                instr_count++;
                if(verbose){
                    printf("%s %s %s\n", instr, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "nor") == 0){
                char *rd = tokens[1];
                char *rs = tokens[2];
                char *rt = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b100111;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "or") == 0){
                char *rd = tokens[1];
                char *rs = tokens[2];
                char *rt = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b100101;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "sll") == 0){
                char *rd = tokens[1];
                char *rt = tokens[2];
                char *sa = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrt = GetDataRegAddressFromString(rt);
                int vsa = GetDataRegAddressFromString(sa);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += vsa << 6;
                Instruction_Memory[instr_count].data += 0b000000;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rt, sa);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "sllv") == 0){
                char *rd = tokens[1];
                char *rt = tokens[2];
                char *rs = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b000100;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rt, rs);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "slt") == 0){
                char *rd = tokens[1];
                char *rs = tokens[2];
                char *rt = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b101010;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "sltu") == 0){
                char *rd = tokens[1];
                char *rs = tokens[2];
                char *rt = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b101011;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "sra") == 0){
                char *rd = tokens[1];
                char *rt = tokens[2];
                char *sa = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrt = GetDataRegAddressFromString(rt);
                int vsa = GetDataRegAddressFromString(sa);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += vsa << 6;
                Instruction_Memory[instr_count].data += 0b000011;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rt, sa);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "srav") == 0){
                char *rd = tokens[1];
                char *rt = tokens[2];
                char *rs = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b000111;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rt, rs);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "srl") == 0){
                char *rd = tokens[1];
                char *rt = tokens[2];
                char *sa = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrt = GetDataRegAddressFromString(rt);
                int vsa = GetDataRegAddressFromString(sa);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += vsa << 6;
                Instruction_Memory[instr_count].data += 0b000010;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rt, sa);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "srlv") == 0){
                char *rd = tokens[1];
                char *rs = tokens[3];
                char *rt = tokens[2];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b000110;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rt, rs);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "sub") == 0){
                char *rd = tokens[1];
                char *rs = tokens[2];
                char *rt = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b100010;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "subu") == 0){
                char *rd = tokens[1];
                char *rs = tokens[2];
                char *rt = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b100011;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "syscall") == 0){
                Instruction_Memory[instr_count].data = 0b001100;
                instr_count++;
            }
            else if(strcmp(instr, "xor") == 0){
                char *rd = tokens[1];
                char *rs = tokens[2];
                char *rt = tokens[3];
                Instruction_Memory[instr_count].data = 0;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vrd = GetDataRegAddressFromString(rd);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vrd << 11;
                Instruction_Memory[instr_count].data += 0b100110;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rd, rs, rt);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "addi") == 0){
                char *rt = tokens[1];
                char *rs = tokens[2];
                char *immediate = tokens[3];
                Instruction_Memory[instr_count].data = 0b001000 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(tokens[3]);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rt, rs, immediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "addiu") == 0){
                char *rt = tokens[1];
                char *rs = tokens[2];
                char *immediate = tokens[3];
                Instruction_Memory[instr_count].data = 0b001001 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(tokens[3]);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rt, rs, immediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "andi") == 0){
                char *rt = tokens[1];
                char *rs = tokens[2];
                char *immediate = tokens[3];
                Instruction_Memory[instr_count].data = 0b001100 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(tokens[3]);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rt, rs, immediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "beq") == 0){
                char *rs = tokens[1];
                char *rt = tokens[2];
                char *label = tokens[3];
                Instruction_Memory[instr_count].data = 0b000100 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vl;
                vl = GetLineOfLabel(label);
                vl = vl - instr_count;
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vl;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %d\n", instr, rs, rt, vl);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "bgez") == 0){
                char *rs = tokens[1];
                char *label = tokens[2];
                Instruction_Memory[instr_count].data = 0b000001 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = 0b00001;
                int vl;
                vl = GetLineOfLabel(label);
                vl = vl - instr_count;      
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vl;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rs, vl);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "bgtz") == 0){
                char *rs = tokens[1];
                char *label = tokens[2];
                Instruction_Memory[instr_count].data = 0b000111 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = 0b00000;
                int vl;
                vl = GetLineOfLabel(label);
                vl = vl - instr_count;
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vl;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rs, vl);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "blez") == 0){
                char *rs = tokens[1];
                char *label = tokens[2];
                Instruction_Memory[instr_count].data = 0b000110 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = 0b00000;
                int vl;
                vl = GetLineOfLabel(label);
                vl = vl - instr_count;
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vl;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rs, vl);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "bltz") == 0){
                char *rs = tokens[1];
                char *label = tokens[2];
                Instruction_Memory[instr_count].data = 0b000001 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = 0b00000;
                int vl;
                vl = GetLineOfLabel(label);
                vl = vl - instr_count;
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vl;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rs, vl);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "bne") == 0){
                char *rs = tokens[1];
                char *rt = tokens[2];
                char *label = tokens[3];
                Instruction_Memory[instr_count].data = 0b000101 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vl;
                vl = GetLineOfLabel(label);
                vl = vl - instr_count;
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vl;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %d\n", instr, rs, rt, vl);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "lb") == 0){
                char *rt = tokens[1];
                char *immediate = tokens[2];
                Instruction_Memory[instr_count].data = 0b100000 << 26;
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(immediate);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rt, vimmediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "lbu") == 0){
                char *rt = tokens[1];
                char *immediate = tokens[2];
                Instruction_Memory[instr_count].data = 0b100100 << 26;
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(immediate);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rt, vimmediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "lh") == 0){
                char *rt = tokens[1];
                char *immediate = tokens[2];
                Instruction_Memory[instr_count].data = 0b100001 << 26;
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(immediate);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rt, vimmediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "lhu") == 0){
                char *rt = tokens[1];
                char *immediate = tokens[2];
                Instruction_Memory[instr_count].data = 0b100101 << 26;
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(immediate);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rt, vimmediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "lui") == 0){
                char *rt = tokens[1];
                char *immediate = tokens[2];
                Instruction_Memory[instr_count].data = 0b001111 << 26;
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(immediate);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rt, vimmediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "lw") == 0){
                char *rt = tokens[1];
                char *immediate = tokens[2];
                Instruction_Memory[instr_count].data = 0b100011 << 26;
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(immediate);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rt, vimmediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "lwcl") == 0){
                char *rt = tokens[1];
                char *immediate = tokens[2];
                Instruction_Memory[instr_count].data = 0b110001 << 26;
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(immediate);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rt, vimmediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "ori") == 0){
                char *rt = tokens[1];
                char *rs = tokens[2];
                char *immediate = tokens[3];
                Instruction_Memory[instr_count].data = 0b001101 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(tokens[3]);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rt, rs, immediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "sb") == 0){
                char *rt = tokens[1];
                char *immediate = tokens[2];
                Instruction_Memory[instr_count].data = 0b101000 << 26;
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(immediate);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rt, vimmediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "slti") == 0){
                char *rt = tokens[1];
                char *rs = tokens[2];
                char *immediate = tokens[3];
                Instruction_Memory[instr_count].data = 0b001010 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(tokens[3]);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rt, rs, immediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "sltiu") == 0){
                char *rt = tokens[1];
                char *rs = tokens[2];
                char *immediate = tokens[3];
                Instruction_Memory[instr_count].data = 0b001011 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(tokens[3]);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rt, rs, immediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "sh") == 0){
                char *rt = tokens[1];
                char *immediate = tokens[2];
                Instruction_Memory[instr_count].data = 0b101001 << 26;
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(immediate);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rt, vimmediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "sw") == 0){
                char *rt = tokens[1];
                char *immediate = tokens[2];
                Instruction_Memory[instr_count].data = 0b101011 << 26;
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(immediate);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rt, vimmediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "swcl") == 0){
                char *rt = tokens[1];
                char *immediate = tokens[2];
                Instruction_Memory[instr_count].data = 0b111001 << 26;
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(immediate);
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %d\n", instr, rt, vimmediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "xori") == 0){
                char *rt = tokens[1];
                char *rs = tokens[2];
                char *immediate = tokens[3];
                Instruction_Memory[instr_count].data = 0b001110 << 26;
                int vrs = GetDataRegAddressFromString(rs);
                int vrt = GetDataRegAddressFromString(rt);
                int vimmediate = atoi(tokens[3]);
                Instruction_Memory[instr_count].data += vrs << 21;
                Instruction_Memory[instr_count].data += vrt << 16;
                Instruction_Memory[instr_count].data += vimmediate;
                instr_count++;
                if(verbose){
                    printf("%s %s %s %s\n", instr, rt, rs, immediate);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "j") == 0){
                char *label = tokens[1];
                Instruction_Memory[instr_count].data = 0b000010 << 26;
                int vl;
                vl = GetLineOfLabel(label);
                Instruction_Memory[instr_count].data += vl;
                instr_count++;
                if(verbose){
                    printf("%s %d\n", instr, vl);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            else if(strcmp(instr, "jal") == 0){
                char *label = tokens[1];
                Instruction_Memory[instr_count].data = 0b000011 << 26;
                int vl;
                vl = GetLineOfLabel(label);
                Instruction_Memory[instr_count].data += vl;
                instr_count++;
                if(verbose){
                    printf("%s %d\n", instr, vl);
                    Print32BitRegister(&Instruction_Memory[instr_count - 1]);
                }
            }
            lineIndex++;
        }while(!feof(im));
    }
}

void LoadInstructionMemory(){
    int ch;
    uint8_t row = 0;
    uint8_t column = 0;
    if(im != NULL){
        if(verbose > 0){
            printf("Loading instruction memory\n");
        }
        while((ch = fgetc(im)) != EOF){
            if(ch >= 48 && ch <= 49){
                uint8_t v = ch - 48;
                Instruction_Memory[row].data = Instruction_Memory[row].data << 1;
                Instruction_Memory[row].data += v;
                column++;
                if(column >= 32){
                    column = 0;
                    if(verbose > 0){
                        Print32BitRegister(&Instruction_Memory[row]);
                    }
                    row++;
                }
                
            }
        }
    }
}

void LoadDataMemory(){
    int ch;
    uint8_t row = 0;
    uint8_t column = 0;
    if(dm != NULL){
        if(verbose > 0){
            printf("Loading data memory\n");
        }
        while((ch = fgetc(dm)) != EOF){
            if(ch >= 48 && ch <= 49){
                uint8_t v = ch - 48;
                Data_Memory[row].data = Data_Memory[row].data << 1;
                Data_Memory[row].data += v;
                column++;
                if(column >= 32){
                    column = 0;
                    if(verbose > 0){
                        Print32BitRegister(&Data_Memory[row]);
                    }
                    row++;
                }
                
            }
        }
    }
}

void LoadRAM(){
    int ch;
    uint8_t row = 0;
    uint8_t column = 0;
    
    Memory = malloc(ram_size * (sizeof(__32BitReg)));
    ram_init = 1;
    if(Memory == NULL){
        printf("Not enough memory to load RAM");
        ram_init = 0;
    }

    if(rm != NULL){
        if(verbose > 0){
            printf("Loading RAM\n");
        }
        while((ch = fgetc(rm)) != EOF){
            if(ch >= 48 && ch <= 49){
                uint8_t v = ch - 48;
                Memory[row].data = Memory[row].data << 1;
                Memory[row].data += v;
                column++;
                if(column >= 32){
                    column = 0;
                    if(verbose > 0){
                        Print32BitRegister(&Memory[row]);
                    }
                    row++;
                }
                
            }
        }
    }
}

void WriteDataMemory(){
    fseek(dm, 0, SEEK_SET);
    for(int i = 0; i < 32; i++){
       fprintf(dm, "%d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d\n",
           Data_Memory[i].bit32,  
           Data_Memory[i].bit31,
           Data_Memory[i].bit30,
           Data_Memory[i].bit29,
           Data_Memory[i].bit28,
           Data_Memory[i].bit27,
           Data_Memory[i].bit26,
           Data_Memory[i].bit25,
           Data_Memory[i].bit24,
           Data_Memory[i].bit23,
           Data_Memory[i].bit22,
           Data_Memory[i].bit21,
           Data_Memory[i].bit20,
           Data_Memory[i].bit19,
           Data_Memory[i].bit18,
           Data_Memory[i].bit17,
           Data_Memory[i].bit16,
           Data_Memory[i].bit15,
           Data_Memory[i].bit14,
           Data_Memory[i].bit13,
           Data_Memory[i].bit12,
           Data_Memory[i].bit11,
           Data_Memory[i].bit10,
           Data_Memory[i].bit9,
           Data_Memory[i].bit8,
           Data_Memory[i].bit7,
           Data_Memory[i].bit6,
           Data_Memory[i].bit5,
           Data_Memory[i].bit4,
           Data_Memory[i].bit3,
           Data_Memory[i].bit2,
           Data_Memory[i].bit1); 
        if(verbose){
            Print32BitRegister(&Data_Memory[i]);
        }
    }
}

void WriteRAM(){
    fseek(rm, 0, SEEK_SET);
    for(int i = 0; i < ram_size; i++){
       fprintf(rm, "%d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d\n",
           Memory[i].bit32,  
           Memory[i].bit31,
           Memory[i].bit30,
           Memory[i].bit29,
           Memory[i].bit28,
           Memory[i].bit27,
           Memory[i].bit26,
           Memory[i].bit25,
           Memory[i].bit24,
           Memory[i].bit23,
           Memory[i].bit22,
           Memory[i].bit21,
           Memory[i].bit20,
           Memory[i].bit19,
           Memory[i].bit18,
           Memory[i].bit17,
           Memory[i].bit16,
           Memory[i].bit15,
           Memory[i].bit14,
           Memory[i].bit13,
           Memory[i].bit12,
           Memory[i].bit11,
           Memory[i].bit10,
           Memory[i].bit9,
           Memory[i].bit8,
           Memory[i].bit7,
           Memory[i].bit6,
           Memory[i].bit5,
           Memory[i].bit4,
           Memory[i].bit3,
           Memory[i].bit2,
           Memory[i].bit1); 
    }
}

void Print32BitRegister(__32BitReg *reg){
    printf("%d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d\n",
           reg->bit32,  
           reg->bit31,
           reg->bit30,
           reg->bit29,
           reg->bit28,
           reg->bit27,
           reg->bit26,
           reg->bit25,
           reg->bit24,
           reg->bit23,
           reg->bit22,
           reg->bit21,
           reg->bit20,
           reg->bit19,
           reg->bit18,
           reg->bit17,
           reg->bit16,
           reg->bit15,
           reg->bit14,
           reg->bit13,
           reg->bit12,
           reg->bit11,
           reg->bit10,
           reg->bit9,
           reg->bit8,
           reg->bit7,
           reg->bit6,
           reg->bit5,
           reg->bit4,
           reg->bit3,
           reg->bit2,
           reg->bit1);
}

void ProcessorRun(){
    while(program_counter < 32 && brk == 0 && stop == 0){
        if(verbose > 0){
            printf("\nProcessing instruction: \n");
            Print32BitRegister(&Instruction_Memory[program_counter]);
        }
        ProcessInstruction(&Instruction_Memory[program_counter]);
    }
}

void ProcessInstruction(__32BitReg *instruction){
    uint8_t opcode = (instruction->data & op_mask) >> 26;
    if(verbose > 0){
    	printf("%d. ", program_counter);
    }
    if(opcode == 0b000000){ //R-Type Instruction
        uint8_t function = (instruction->data & fc_mask);
        uint8_t rd = (instruction->data & rd_mask) >> 11;
        uint8_t rt = (instruction->data & rt_mask) >> 16;
        uint8_t rs = (instruction->data & rs_mask) >> 21;
        uint8_t sa = (instruction->data & sh_mask) >> 6;
        if(function == 0b100000){//ADD
            ALU_a.data = Data_Memory[rs].data;
            ALU_b.data = Data_Memory[rt].data;
            ALU_op.data = ADD;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) + [%d](%d)\n", rd, rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);
            }
        }
        else if(function == 0b100001){//ADDU
            ALU_a.data = Data_Memory[rs].data;
            ALU_b.data = Data_Memory[rt].data;
            ALU_op.data = ADD;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) + [%d](%d)\n", rd, rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);
            }
        }
        else if(function == 0b100100){//AND
            ALU_a.data = Data_Memory[rs].data;
            ALU_b.data = Data_Memory[rt].data;
            ALU_op.data = ADD;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) & [%d](%d)\n", rd, rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);
            }
        }
        else if(function == 0b001101){//BREAK
            brk = 1;
            program_counter++;
            if(verbose){
                printf("BREAK\n");
            }
        }
        else if(function == 0b011010){//DIV
            ALU_a.data = Data_Memory[rs].data;
            ALU_b.data = Data_Memory[rt].data;
            ALU_op.data = SUB;
            uint32_t div = 0;
            while(ALU_a.data > ALU_b.data){
                ALU();
                ALU_a.data = ALU_out.data;
                div++;
            }
            hi.data = div;
            lo.data = ALU_a.data;
            program_counter++;
            if(verbose){
                printf("[hi] = [%d](%d) / [%d](%d) + [lo]\n", rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);
            }
        }
        else if(function == 0b011011){//DIVU
            ALU_a.data = Data_Memory[rs].data;
            ALU_b.data = Data_Memory[rt].data;
            ALU_op.data = SUB;
            uint32_t div = 0;
            while(ALU_a.data > ALU_b.data){
                ALU();
                ALU_a.data = ALU_out.data;
                div++;
            }
            hi.data = div;
            lo.data = ALU_a.data;
            program_counter++;
            if(verbose){
                printf("[hi] = [%d](%d) / [%d](%d) + [lo]\n", rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);          
            }
        }
        else if(function == 0b001000){//JR
            program_counter = Data_Memory[rs].data / 4;
            if(verbose){
                printf("PC = [%d](%d)\n", rs, Data_Memory[rs].data);
            }
        }
        else if(function == 0b001001){//JALR
            Data_Memory[rd].data = (program_counter + 1) * 4;
            program_counter = Data_Memory[rs].data / 4;
            if(verbose){
                printf("[%d] = PC(%d) + 1; PC = [%d](%d)\n", rd, program_counter, rs, Data_Memory[rs].data);
            }
        }
        else if(function == 0b010000){//MFHI
            Data_Memory[rd].data = hi.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [hi](%d)\n", rd, hi.data);
            }
        }
        else if(function == 0b010010){//MFLO
            Data_Memory[rd].data = lo.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [lo](%d)\n", rd, lo.data);
            }
        }
        else if(function == 0b010001){//MTHI
            hi.data = Data_Memory[rs].data;
            program_counter++;
            if(verbose){
                printf("[hi] = [%d](%d)\n", rs, Data_Memory[rs].data);
            }
        }
        else if(function == 0b010011){//MTLO
            lo.data = Data_Memory[rs].data;
            program_counter++;
            if(verbose){
                printf("[lo] = [%d](%d)\n", rs, Data_Memory[rs].data);
            }
        }
        else if(function == 0b011000){//MULT
            uint32_t multicand;
            if(Data_Memory[rt].bit32 == 1){
                multicand = -Data_Memory[rt].data;
                if(Data_Memory[rs].bit32 == 1){
                    ALU_a.data = -Data_Memory[rs].data;
                }
                else{
                    ALU_a.data = Data_Memory[rs].data;
                }          
                ALU_b.data = ALU_a.data;
            }
            else if(Data_Memory[rs].bit32 == 1){
                multicand = -Data_Memory[rs].data;
                if(Data_Memory[rt].bit32 == 1){
                    ALU_a.data = -Data_Memory[rt].data;
                }
                else{
                    ALU_a.data = Data_Memory[rt].data;
                }          
                ALU_b.data = ALU_a.data;
            }
            else{
                multicand = Data_Memory[rt].data;
                ALU_a.data = Data_Memory[rs].data;
                ALU_b.data = ALU_a.data;
            }
            ALU_op.data = ADD;
            for(uint32_t i = 0; i < multicand; i++){
                ALU();
                ALU_b.data = ALU_out.data;
            }
            lo.data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[lo] = [%d](%d) * [%d](%d)\n", rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);
            }
        }
        else if(function == 0b011001){//MULTU
            uint32_t multicand;
            if(Data_Memory[rt].bit32 == 1){
                multicand = -Data_Memory[rt].data;
                if(Data_Memory[rs].bit32 == 1){
                    ALU_a.data = -Data_Memory[rs].data;
                }
                else{
                    ALU_a.data = Data_Memory[rs].data;
                }          
                ALU_b.data = ALU_a.data;
            }
            else if(Data_Memory[rs].bit32 == 1){
                multicand = -Data_Memory[rs].data;
                if(Data_Memory[rt].bit32 == 1){
                    ALU_a.data = -Data_Memory[rt].data;
                }
                else{
                    ALU_a.data = Data_Memory[rt].data;
                }          
                ALU_b.data = ALU_a.data;
            }
            else{
                multicand = Data_Memory[rt].data;
                ALU_a.data = Data_Memory[rs].data;
                ALU_b.data = ALU_a.data;
            }
            ALU_op.data = ADD;
            for(uint32_t i = 0; i < multicand; i++){
                ALU();
                ALU_b.data = ALU_out.data;
            }
            lo.data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[lo] = [%d](%d) * [%d](%d)\n", rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);
            }
        }
        else if(function == 0b100101){//OR
            ALU_a.data = Data_Memory[rs].data;
            ALU_b.data = Data_Memory[rt].data;
            ALU_op.data = OR;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) | [%d](%d)\n", rd, rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);
            }
        }
        else if(function == 0b100111){//NOR
            ALU_a.data = Data_Memory[rs].data;
            ALU_b.data = Data_Memory[rt].data;
            ALU_op.data = OR;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            ALU_a.data = Data_Memory[0].data;
            ALU_b.data = Data_Memory[rd].data;
            ALU_op.data = SUB;
            ALU();
            Data_Memory[rd].data = ALU_op.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) !| [%d](%d)\n", rd, rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);
            }
        }
        else if(function == 0b000000){//SLL
            ALU_a.data = Data_Memory[rt].data;
            ALU_b.data = sa;
            ALU_op.data = SLL;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) << %d\n", rd, rt, Data_Memory[rt].data, sa);
            }
        }
        else if(function == 0b000100){//SLLV
            ALU_a.data = Data_Memory[rt].data;
            ALU_b.data = Data_Memory[rs].data;
            ALU_op.data = SLL;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) << [%d](%d)\n", rd, rt, Data_Memory[rt].data, rs, Data_Memory[rs].data);
            }
        }
        else if(function == 0b101010){//SLT
            ALU_a.data = Data_Memory[rs].data;
            ALU_b.data = Data_Memory[rt].data;
            ALU_op.data = SLT;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("if([%d](%d) < [%d](%d)) [%d] = 1; else [%d] = 0\n", rs, Data_Memory[rs].data, rt, Data_Memory[rt].data, rd, rd);
            }
        }
        else if(function == 0b000011){//SRA
            ALU_a.data = Data_Memory[rt].data;
            ALU_b.data = sa;
            ALU_op.data = SRL;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) >> %d\n", rd, rt, Data_Memory[rt].data, sa);
            }
        }
        else if(function == 0b000111){//SRAV
            ALU_a.data = Data_Memory[rt].data;
            ALU_b.data = Data_Memory[rs].data;;
            ALU_op.data = SRL;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) >> [%d](%d)\n", rd, rt, Data_Memory[rt].data, rs, Data_Memory[rs].data);
            }
        }
        else if(function == 0b000010){//SRL
            ALU_a.data = Data_Memory[rt].data;
            ALU_b.data = sa;
            ALU_op.data = SRL;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) >> %d\n", rd, rt, Data_Memory[rt].data, sa);
            }
        }
        else if(function == 0b000110){//SRLV
            ALU_a.data = Data_Memory[rt].data;
            ALU_b.data = Data_Memory[rs].data;;
            ALU_op.data = SRL;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) >> [%d](%d)\n", rd, rt, Data_Memory[rt].data, rs, Data_Memory[rs].data);
            }
        }
        else if(function == 0b100010){//SUB
            ALU_a.data = Data_Memory[rs].data;
            ALU_b.data = Data_Memory[rt].data;
            ALU_op.data = SUB;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) - [%d](%d)\n", rd, rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);
            }
        }
        else if(function == 0b100011){//SUBU
            ALU_a.data = Data_Memory[rs].data;
            ALU_b.data = Data_Memory[rt].data;
            ALU_op.data = SUB;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) - [%d](%d)\n", rd, rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);
            }
        }
        else if(function == 0b001100){//SYSCALL
            switch(Data_Memory[v0].data){
                case 1://PRINT INT
                    printf("%d", Data_Memory[a0].data);
                break;
                case 2://PRINT FLOAT
                    printf("%f", Data_Memory[a0].data);
                break;
                case 4://PRINT STRING
                {
                    int offset = 0;
                    uint8_t c = Memory[Data_Memory[a0].data + offset].data & 0xff;
                    while(c != '\n'){
                        printf("%c", c);
                        offset++;
                        c = Memory[Data_Memory[a0].data + offset].data & 0xff;
                }
                break;
                }
                case 5://READ INT
                    scanf("%d", &Data_Memory[v0].data);
                break;
                case 6://READ FLOAT
                    scanf("%f", &Data_Memory[v0].data);
                break;
                case 8://READ STRING
                {
                    char str[256];
                    gets(str);
                    int index = 0;
                    char c = str[index];
                    while(c != '\0' && index < Data_Memory[a1].data){
                        Memory[Data_Memory[a0].data + index].data = c;
                        index++;
                        c = str[index];
                    }
                    Memory[Data_Memory[a0].data + index].data = c;
                }
                break;
                case 9://ALLOCATE HEAP
                    for(uint8_t i = 0; i < Data_Memory[a0].data; i++){
                        Memory[Data_Memory[v0].data + i].data = 0;
                    }
                break;
                case 10://EXIT
                    stop = 1;
                break;
                case 11://PRINT CHARACTER
                    printf("%c", Data_Memory[a0].data);
                break;
                case 12://READ CHARACTER
                    scanf("%lc", &Data_Memory[v0].data);
                break;
                case 13://OPEN FILE
                {
                    char filename[256];
                    int offset = 0;
                    char c = Memory[Data_Memory[a0].data + offset].data;
                    while(c != '\n'){
                        filename[offset] = c;
                        offset++;
                        c = Memory[Data_Memory[a0].data + offset].data;
                    }
                    uint8_t flags = Data_Memory[a1].data;
                    FILE *fd;
                    if(flags == 0){//READ ONLY
                        fd = fopen(filename, "r");
                    }
                    else if(flags == 1){//WRITE ONLY
                        fd = fopen(filename, "r");
                    }
                    else if(flags == 9){//WRITE ONLY WITH CREATE AND APPEND
                        fd = fopen(filename, "w+");
                    }
                    if(fd){
                        Data_Memory[v0].data = fileno(fd);
                        files[Data_Memory[v0].data] = fd;
                    }
                    else{
                        Data_Memory[v0].data = -1;
                    }
                }
                break;
                case 14://READ FROM FILE
                {
                    FILE *f = files[Data_Memory[a0].data];
                    char c = fgetc(f);
                    int offset = 0;
                    while(c != EOF && offset <= Data_Memory[a2].data){
                        Memory[Data_Memory[a1].data + offset].data = c;
                        offset++;
                        c = fgetc(f);
                    }
                }
                break;
                case 15://WRITE TO FILE
                {
                    FILE *f = files[Data_Memory[a0].data];
                    char c = Memory[Data_Memory[a1].data].data;
                    int offset = 0;
                    while(c != '\0' && offset <= Data_Memory[a2].data){
                        fputc(c, f);
                        offset++;
                        c = Memory[Data_Memory[a1].data + offset].data;
                    }
                }
                break;
                case 16://CLOSE FILE
                {
                    FILE *f = files[Data_Memory[a0].data];
                    fclose(f);
                }
                break;
                case 17://TERMINATE WITH RESULT
                {
                    result = Data_Memory[a0].data;
                    stop = 1;
                }
                break;
            }
            program_counter++;
            if(verbose){
                printf("SYSCALL\n");
            }
        }
        else if(function == 0b100110){//XOR
            ALU_a.data = Data_Memory[rs].data;
            ALU_b.data = Data_Memory[rt].data;
            ALU_op.data = XOR;
            ALU();
            Data_Memory[rd].data = ALU_out.data;
            program_counter++;
            if(verbose){
                printf("[%d] = [%d](%d) !^ [%d](%d)\n", rd, rs, Data_Memory[rs].data, rt, Data_Memory[rt].data);
            }
        }
    }
    else if(opcode == 0b001000){ //ADDI
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = immediate;
        ALU_op.data = ADD;
        ALU();
        Data_Memory[rt].data = ALU_out.data;
        program_counter++;
        if(verbose){
            printf("[%d] = [%d](%d) + %d\n", rt, rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b001001){ //ADDIU
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = immediate;
        ALU_op.data = ADD;
        ALU();
        Data_Memory[rt].data = ALU_out.data;
        program_counter++;
        if(verbose){
            printf("[%d] = [%d](%d) + %d\n", rt, rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b001100){ //ANDI
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = immediate;
        ALU_op.data = AND;
        ALU();
        Data_Memory[rt].data = ALU_out.data;
        program_counter++;
        if(verbose){
            printf("[%d] = [%d](%d) & %d\n", rt, rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b000100){ //BEQ
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = Data_Memory[rt].data;
        ALU_op.data = SUB;
        ALU();
        if(ALU_Flags.z){
            program_counter += immediate;
        }
        else
        {
            program_counter++;
        }
        if(verbose){
            printf("if([%d](%d) == [%d](%d)) PC += %d; else PC++\n", rt, Data_Memory[rt].data, rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b000001 && instruction->bit17 == 1 && instruction->bit21 == 0){ //BGEZ
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = Data_Memory[0].data;
        ALU_op.data = SLT;
        ALU();
        if(ALU_out.bit1 == 0){
            program_counter += immediate;
        }
        else
        {
            program_counter++;
        }
        if(verbose){
            printf("if([%d](%d) >= [0]) PC += %d\n", rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b000001 && instruction->bit17 == 1 && instruction->bit21 == 1){ //BGEZAL
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = Data_Memory[0].data;
        ALU_op.data = SLT;
        ALU();
        if(ALU_out.bit1 == 0){
            Data_Memory[31].data = (program_counter + 2) * 4;
            program_counter += immediate;
        }
        else
        {
            program_counter++;
        }
        if(verbose){
            printf("if([%d](%d) >= [0]) [31] = PC + 8 PC += %d\n", rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b000111 && instruction->bit17 == 0 && instruction->bit21 == 0){ //BGTZ
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[0].data;
        ALU_b.data = Data_Memory[rs].data;
        ALU_op.data = SLT;
        ALU();
        if(ALU_out.bit1 == 1){
            program_counter += immediate;
        }
        else
        {
            program_counter++;
        }
        if(verbose){
            printf("if([%d](%d) > [0]) PC += %d\n", rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b000110 && instruction->bit17 == 0 && instruction->bit21 == 0){ //BLEZ
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[0].data;
        ALU_b.data = Data_Memory[rs].data;
        ALU_op.data = SLT;
        ALU();
        if(ALU_out.bit1 == 1){
            program_counter += immediate;
        }
        else
        {
            program_counter++;
        }
        if(verbose){
            printf("if([%d](%d) <= [0]) PC += %d\n", rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b000001 && instruction->bit17 == 0 && instruction->bit21 == 0){ //BLTZ
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = Data_Memory[0].data;
        ALU_op.data = SLT;
        ALU();
        if(ALU_out.bit1 == 1){
            program_counter += immediate;
        }
        else
        {
            program_counter++;
        }
        if(verbose){
            printf("if([%d](%d) < [0]) PC += %d\n", rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b000101){ //BNE
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = Data_Memory[rt].data;
        ALU_op.data = SUB;
        ALU();
        if(ALU_Flags.z == 0){
            program_counter += immediate;
        }
        else
        {
            program_counter++;
        }
        if(verbose){
            printf("if([%d](%d) != [%d](%d)) PC += %d\n", rt, Data_Memory[rt].data, rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b100000){ //LB
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        Data_Memory[rt].data = Memory[Data_Memory[rs].data + immediate].data & 0xff;
        program_counter++;
        if(verbose){
            printf("[%d] = MEM[[%d] + %d] & 0xff\n", rt, rs, immediate);
        }
    }
    else if(opcode == 0b100100){ //LBU
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        Data_Memory[rt].data = Memory[Data_Memory[rs].data + immediate].data & 0xff;
        program_counter++;
        if(verbose){
            printf("[%d] = MEM[[%d] + %d] & 0xff\n", rt, rs, immediate);
        }
    }
    else if(opcode == 0b100001){ //LH
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        Data_Memory[rt].data = Memory[Data_Memory[rs].data + immediate].data & 0xffff;
        program_counter++;
        if(verbose){
            printf("[%d] = MEM[[%d] + %d] & 0xffff\n", rt, rs, immediate);
        }
    }
    else if(opcode == 0b100101){ //LHU
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        Data_Memory[rt].data = Memory[Data_Memory[rs].data + immediate].data & 0xffff;
        program_counter++;
        if(verbose){
            printf("[%d] = MEM[[%d] + %d] & 0xffff\n", rt, rs, immediate);
        }
    }
    else if(opcode == 0b001111){ //LUI
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int16_t immediate = (instruction->data & im_mask);
        Data_Memory[rt].data = immediate << 16;
        program_counter++;
        if(verbose){
            printf("[%d] = %d << 16\n", rt, immediate);
        }
    }
    else if(opcode == 0b100101){ //LW
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        Data_Memory[rt].data = Memory[Data_Memory[rs].data + immediate].data;
        program_counter++;
        if(verbose){
            printf("[%d] = MEM[[%d] + %d]\n", rt, rs, immediate);
        }
    }
    else if(opcode == 0b001100){ //ORI
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = immediate;
        ALU_op.data = OR;
        ALU();
        Data_Memory[rt].data = ALU_out.data;
        program_counter++;
        if(verbose){
            printf("[%d] = [%d](%d) | %d\n", rt, rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b100000){ //SB
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        Memory[Data_Memory[rs].data + immediate].data = 0xff & Data_Memory[rt].data;
        program_counter++;
        if(verbose){
            printf("MEM[[%d] + %d] = [%d] & 0xff\n", rs, immediate, rt);
        }
    }
    else if(opcode == 0b001010){ //SLTI
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = immediate;
        ALU_op.data = SLT;
        ALU();
        Data_Memory[rt].data = ALU_out.data;
        program_counter++;
        if(verbose){
            printf("if([%d](%d) < %d) [%d] = 1; else [%d] = 0\n", rs, Data_Memory[rs].data, immediate, rt, rt);
        }
    }
    else if(opcode == 0b001011){ //SLTIU
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = immediate;
        ALU_op.data = SLT;
        ALU();
        Data_Memory[rt].data = ALU_out.data;
        program_counter++;
        if(verbose){
            printf("if([%d](%d) < %d) [%d] = 1; else [%d] = 0\n", rs, Data_Memory[rs].data, immediate, rt, rt);
        }
    }
    else if(opcode == 0b101001){ //SH
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        Memory[Data_Memory[rs].data + immediate].data = 0xffff & Data_Memory[rt].data;
        program_counter++;
        if(verbose){
            printf("MEM[[%d] + %d] = [%d] & 0xffff\n", rs, immediate, rt);
        }
    }
    else if(opcode == 0b101011){ //SW
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        Memory[Data_Memory[rs].data + immediate].data = Data_Memory[rt].data;
        program_counter++;
        if(verbose){
            printf("MEM[[%d] + %d] = [%d]\n", rs, immediate, rt);
        }
    }
    else if(opcode == 0b001110){ //XORI
        int8_t rt = (instruction->data & rt_mask) >> 16;
        int8_t rs = (instruction->data & rs_mask) >> 21;
        int16_t immediate = (instruction->data & im_mask);
        ALU_a.data = Data_Memory[rs].data;
        ALU_b.data = immediate;
        ALU_op.data = XOR;
        ALU();
        Data_Memory[rt].data = ALU_out.data;
        program_counter++;
        if(verbose){
            printf("[%d] = [%d](%d) XOR %d\n", rt, rs, Data_Memory[rs].data, immediate);
        }
    }
    else if(opcode == 0b000010){ //J
        int32_t address = (instruction->data & ad_mask);
        program_counter = address;
        if(verbose){
            printf("PC = %d\n", address);
        }
    }
    else if(opcode == 0b000011){ //JAL
        int32_t address = (instruction->data & ad_mask);
        Data_Memory[31].data = program_counter + 2;
        program_counter = address;
        if(verbose){
            printf("[31] = PC(%d) + 8; PC = %d\n", address, program_counter);
        }
    }
}

int GetDataRegAddressFromString(char* string){
    if(strncmp(string, "$zero", 5) == 0){
        return zero;
    }
    else if(strncmp(string, "$at", 3) == 0){
        return at;
    }
    else if(strncmp(string, "$v0", 3) == 0){
        return v0;
    }
    else if(strncmp(string, "$v1", 3) == 0){
        return v1;
    }
    else if(strncmp(string, "$a0", 3) == 0){
        return a0;
    }
    else if(strncmp(string, "$a1", 3) == 0){
        return a1;
    }
    else if(strncmp(string, "$a2", 3) == 0){
        return a2;
    }
    else if(strncmp(string, "$a3", 3) == 0){
        return a3;
    }
    else if(strncmp(string, "$t0", 3) == 0){
        return t0;
    }
    else if(strncmp(string, "$t1", 3) == 0){
        return t1;
    }
    else if(strncmp(string, "$t2", 3) == 0){
        return t2;
    }
    else if(strncmp(string, "$t3", 3) == 0){
        return t3;
    }
    else if(strncmp(string, "$t4", 3) == 0){
        return t4;
    }
    else if(strncmp(string, "$t5", 3) == 0){
        return t5;
    }
    else if(strncmp(string, "$t6", 3) == 0){
        return t6;
    }
    else if(strncmp(string, "$t7", 3) == 0){
        return t7;
    }
    else if(strncmp(string, "$s0", 3) == 0){
        return s0;
    }
    else if(strncmp(string, "$s1", 3) == 0){
        return s1;
    }
    else if(strncmp(string, "$s2", 3) == 0){
        return s2;
    }
    else if(strncmp(string, "$s3", 3) == 0){
        return s3;
    }
    else if(strncmp(string, "$s4", 3) == 0){
        return s4;
    }
    else if(strncmp(string, "$s5", 3) == 0){
        return s5;
    }
    else if(strncmp(string, "$s6", 3) == 0){
        return s6;
    }
    else if(strncmp(string, "$s7", 3) == 0){
        return s7;
    }
    else if(strncmp(string, "$t8", 3) == 0){
        return t8;
    }
    else if(strncmp(string, "$t9", 3) == 0){
        return t9;
    }
    else if(strncmp(string, "$k0", 3) == 0){
        return k0;
    }
    else if(strncmp(string, "$k1", 3) == 0){
        return k1;
    }
    else if(strncmp(string, "$gp", 3) == 0){
        return gp;
    }
    else if(strncmp(string, "$sp", 3) == 0){
        return sp;
    }
    else if(strncmp(string, "$fp", 3) == 0){
        return fp;
    }
    else if(strncmp(string, "$ra", 3) == 0){
        return ra;
    }
}

char** Tokenize(char* line){
    char **tokens = malloc(100 * sizeof(char *));
    int token_index = 0;
    int char_index = 0;
    int lineIndex = 0;
    int new = 0;
    tokens[0] = (char *)malloc(256);
    char c = line[lineIndex];
    while(c != '\n' && c != '\0' && c != '\r' && lineIndex < 256){
        if(c == 36 || c >= 48 && c <= 57 || c >= 65 && c <= 90 || c >= 97 && c <= 122){
            tokens[token_index][char_index] = c;
            char_index++;
            new = 0;
        }
        else{
            if(new == 0){
                token_index++;
                tokens[token_index] = (char *)malloc(256);
                char_index = 0;
                new = 1;
            }
        }
        lineIndex++;
        c = line[lineIndex];
    }
    return tokens;
}

int GetLengthOfLine(char * line){
    int count = 0;
    char c = line[count];
    while(c != '\n' && c != '\0'){
        count++;
        c = line[count];
    }
    return count;
}

int IsInstruction(char* line){
            char ** tokens = Tokenize(line);
            char *instr = tokens[0];
            if(strncmp(instr, "add", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "addu", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "and", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "break", 5) == 0){
                return 1;
            }
            else if(strncmp(instr, "div", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "divu", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "jalr", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "jr", 2) == 0){
                return 1;
            }
            else if(strncmp(instr, "mfhi", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "mflo", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "mthi", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "mtlo", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "mult", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "multu", 5) == 0){
                return 1;
            }
            else if(strncmp(instr, "nor", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "or", 2) == 0){
                return 1;
            }
            else if(strncmp(instr, "sll", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "sllv", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "slt", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "sltu", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "sra", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "srav", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "srl", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "srlv", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "sub", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "subu", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "syscall", 7) == 0){
                return 1;
            }
            else if(strncmp(instr, "xor", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "addi", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "addiu", 5) == 0){
                return 1;
            }
            else if(strncmp(instr, "andi", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "beq", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "bgez", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "bgtz", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "blez", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "bltz", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "bne", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "lb", 2) == 0){
                return 1;
            }
            else if(strncmp(instr, "lbu", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "lh", 2) == 0){
                return 1;
            }
            else if(strncmp(instr, "lhu", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "lui", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "lw", 2) == 0){
                return 1;
            }
            else if(strncmp(instr, "lwcl", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "ori", 3) == 0){
                return 1;
            }
            else if(strncmp(instr, "sb", 2) == 0){
                return 1;
            }
            else if(strncmp(instr, "slti", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "sltiu", 5) == 0){
                return 1;
            }
            else if(strncmp(instr, "sh", 2) == 0){
                return 1;
            }
            else if(strncmp(instr, "sw", 2) == 0){
                return 1;
            }
            else if(strncmp(instr, "swcl", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "xori", 4) == 0){
                return 1;
            }
            else if(strncmp(instr, "j", 1) == 0){
                return 1;
            }
            else if(strncmp(instr, "jal", 3) == 0){
                return 1;
            }
            return 0;
}

