# CMips

[![Join the chat at https://gitter.im/CMips32/Lobby](https://badges.gitter.im/CMips32/Lobby.svg)](https://gitter.im/CMips32/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

A C implementation of a 32 bits MIPS processor.

## 1.0. Getting started

### 1.1. Linux
To compile the source code, simply run "gcc -o cmips Mipc.c".
Then, run it by using ./cmips.

### 1.2. Windows
Download MinGW and set up the environment variables properly (add the bin folder of MinGW to PATH). After, simply use "gcc -o cmipc Mipc.c" to compile. Lastly, run using cmips.

### 1.3. Windows using WSL
Run it using the same steps as in the Linux explanation.

### 1.3 Other operating systems
The code is written in ANSI-C and it tries to be as dependency free as possible. Thus, it should technically work on any OS that has something similar to MinGW or that implements a very tiny subset of POSIX that handles user input and output.

## 2.0. Options

### 2.1. Help
To get a help menu, run the application with the argument "-h" or "--help"

### 2.2. Instruction memory
To run it, you basically need to load an instruction memory (there's an example provided in the "test.bin" file) using the "-i" argument. (Note, due to how the architecture is developed, the maximum amount of lines this file can have is 32, as there are only 5 bits of address which allow you to select registers)

### 2.3. Data memory
The application allows you to load multiple other files, including a data memory (basically a processor cache). Similarly to the instruction memory, the maximum amount of lines is 32. Each one of those 32 registers can be directly accessed from assembly using a specially named variable.

### 2.4. Work memory (RAM)
Ultimately you can load the application providing a RAM file using the argument "-m" and its size specified using the argument "-ms". This file can theoretically have unlimited size and the architecture does allow really large sizes (the order of millions of lines), so in case you want to work with a lot of data, this is the place to load it.

### 2.5. Debugging
If you are in need of debugging, you can run the application using "-v". This option will print in the console each step the application does in a really verbose and explicit way.

### 2.6. Assembly files
There's one last developmental option. The application can accept files written in MIPS assembly. Most of the instructions work, though the jumps are currently not working properly.

## 3.0. Supported instructions

### 3.1. R-Type instructions
#### 3.1.1. Instruction layout


| Opcode(6) | Rs(5) | Rt(5) | Rd(5) | Shamt(5) | Function(6) |
|---|---|---|---|---|---|

#### 3.1.2. Instruction table

For all R-Type instructions, Opcode is "000000".
The table bellow shows the function bits and their resulting instruction

|Function  |  Bits  | Description                     |
|----------|--------|---------------------------------|
| ADD      | 100000 | Rd = Rs + Rt                    |
| ADDU     | 100001 | Rd = Rs + Rt                    |
| AND      | 001101 | Rd = Rs & Rt                    |
| BREAK    | 001101 |                                 |
| DIV      | 011010 | Hi = Rs / Rt + Lo               | *Hi and Lo are two special registers in Data Memory.*
| DIVU     | 011011 | Hi = Rs / Rt + Lo               |
| JR       | 001000 | PC = Rs                         |
| JALR     | 001001 | Rd = PC + 4; PC = Rs            |
| MFHI     | 010000 | Rd = Hi                         |
| MFLO     | 010010 | Rd = Lo                         |
| MTHI     | 010001 | Hi = Rs                         |
| MTLO     | 010011 | Lo = Rs                         |
| MULT     | 011000 | Lo = Rs * Rt                    |
| MULTU    | 011001 | Lo = Rs * Rt                    |
| OR       | 100101 | Rd = Rs | Rt                    |
| NOR      | 100111 | Rd = !(Rd | Rt)                 |
| SLL      | 000000 | Rd = Rt << Shamt                |
| SLLV     | 000100 | Rd = Rt << Rs                   |
| SLT      | 101010 | if(Rs < Rt) Rd = 1; Else Rd = 0 |
| SRA      | 000011 | Rd = Rt >> Shamt                |
| SRAV     | 000111 | Rd = Rt >> Rs                   |
| SRL      | 000010 | Rd = Rt >> Shamt                |
| SRLV     | 000110 | Rd = Rt >> Rs                   |
| SUB      | 100010 | Rd = Rs - Rt                    |
| SUBU     | 100011 | Rd = Rs - Rt                    |
| SYSCALL  | 001100 | ##############################  | *Will be detailed a bit further*
| XOR      | 100110 | Rd = Rs ^ Rt                    |

#### 3.1.3. SYSCALL Details

After calling Syscall, the system will do an operation based on the value in the v0 register.

The following operations are supported.

|   Operation    | Value | Description |
|----------------|-------|---|
| Print int      |  1    | *Prints the value in register a0* |
| Print float    |  2    | |
| Print string   |  4    | *The system will start reading from RAM starting with the address given in register a0 until it reaches "\n"*|
| Read int       |  5    | *Reads the value from into the register v0* |
| Read float     |  6    | |
| Read string    |  8    | *Reads string and writes it into RAM at the address given in a0 until "\n" or until reading max chars in a1* |
| Allocate heap  |  9    | *Right now it only resets the registers in the RAM* |
| Exit           |  10   |
| Print char     |  11   | *Prints char contained in a0* |
| Read char      |  12   | *Reads char into v0* |
| Open file      |  13   | *Reads file name from RAM at address a0. Stores file descriptor in v0* |
| Read from file |  14   | *a0 contains fd. Reads from fd all chars or until it reaches value in a2 and stores in RAM at address a1* |
| Write to file  |  15   | *a0 contains fd. Writes to fd all chars until "\0" or until it reaches value in a2 from RAM at address a1* |
| Close file     |  16   | *Close file with fd in a0* |
| Exit with code |  17   | *Exits with a return value specified in a0"* |


### 3.2. I-Type Instructions
#### 3.2.1. Instruction layout


| Opcode(6) | Rs(5) | Rt(5) |       Immediate(16)      |
|-----------|------|--------|---------------------|

#### 3.2.2. Instruction table

|  Opcode  |  Bits  | Description                     |
|----------|--------|---------------------------------|
|   ADDI   | 001000 | Rt = Rs + imm                   |
|   ADDIU  | 001001 | Rt = Rs + imm                   |
|   ANDI   | 001100 | Rt = Rs & imm                   |
|   BEQ    | 000100 | if(Rt==Rs) PC+=imm              |
|   BGEZ   | 000001 | if(Rs >= v0) PC+=imm;           ###*NOTE: Bit 17 must be 1 and 21 must be 0*          | 
|   BGEZAL | 000001 | if(Rs >= v0) ra=PC+8; PC += imm ###*NOTE: Bit 17 must be 1 and 21 must be 1* | 
|   BGTZ   | 000111 | if(Rs > 0) PC+=imm              ###*NOTE: Bit 17 must be 0 and 21 must be 0* | 
|   BLEZ   | 000110 | if(Rs <= 0) PC+=imm             ###*NOTE: Bit 17 must be 0 and 21 must be 0*  | 
|   BLTZ   | 000001 | if(Rs < 0) PC+=imm              ###*NOTE: Bit 17 must be 0 and 21 must be 0*  | 
|   BNE    | 000101 | if(Rt!=Rs) PC+=imm              |
|   LB     | 100000 | Rt = MEM[Rs + imm] & 0xff       |
|   LBU    | 100100 | Rt = MEM[Rs + imm] & 0xff       |
|   LH     | 100001 | Rt = MEM[Rs + imm] & 0xffff     |
|   LHU    | 100101 | Rt = MEM[Rs + imm] & 0xffff     |
|   LUI    | 001111 | Rt = imm << 16                  |
|   LW     | 100101 | Rt = MEM[Rs + imm]              |
|   ORI    | 001100 | Rt = Rs | imm                   |
|   SB     | 100000 | MEM[Rs + imm] = Rt & 0xff       |
|   SLTI   | 001010 | if(Rs<imm) Rt = 1; Else Rt = 0  |
|   SLTIU  | 001011 | if(Rs<imm) Rt = 1; Else Rt = 0  |
|   SH     | 101001 | MEM[Rs + imm] = Rt & 0xffff     |
|   SW     | 001110 | MEM[Rs + imm] = Rt              |
|   XORI   | 001110 | Rt = Rs XOR imm                 |

### 3.3. J-Type Instructions
#### 3.3.1. Instruction layout


|Opcode(6)|             Address(26)              |
|---------|--------------------------------------|

#### 3.3.2. Instruction Table

|  Opcode  |  Bits  | Description                     |
|----------|--------|---------------------------------|
|   J      | 000010 | PC = addr                       |
|   JAL    | 000011 | ra = PC + 8; PC = addr          |



## 4. Termination

On reaching the end of instruction registers or by calling "Exit", the system will stop. The Data Memory and RAM file will be overwritten (if provided) with the contents of the registers at the termination time. If "-v", these contents will also be written in the console as the writing in the file happens.

If a return code has been provided by using SYSCALL, the application will return that code, otherwise it will return 0.


## 5. Ending note

Feel free to use it as you want as long as it is not in a commercial product. If you want to use it in a commercial product, please contact me before. I will not stop you from using it but I could probably help you understand it better than you just diving deep into the code.
The code is still under review and changes and not thoroughly debugged. Most instructions work but some of them are not working properly or entirely.

## Feel free to submit tickets or changes as you see fit. I will try to review them as fast as I can.
