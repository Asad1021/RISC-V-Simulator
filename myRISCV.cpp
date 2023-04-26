#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <string.h>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <unordered_map>
// #include <bits/stdc++.h>
#include <list>
#include <utility> // needed for std::pair

// ################# remember to update mem.Oprands

char nullData = '~';

#define MEMORY_SIZE 120000
#define INSTMEM_SIZE 12000
#define BUFFER_SIZE 120

// immb conversion is always a unsigned operation hence we have to convert it to signed one by using typecast
// immB+PC; immB=immb.to_ulong();//unsigned


using namespace std;

typedef struct BranchTargetBuffer
{
    bitset<32> currentPCAdd;
    bitset<32> predictedAdd;
    bool taken;
    bool valid;
} B_T_B;

B_T_B BTB[BUFFER_SIZE];
int BTB_index = 0;
char* Placeholder_Name(char *, int , int , int );
// Cache<int,int> cache(cacheSize,blockSize);

void btb_nuller(B_T_B BTB[], int n)
{
    // this will flush everything out of our Branch target buffer

    for (int i = 0; i < n; i++)
    {
        BTB[i].currentPCAdd = 0;
        BTB[i].predictedAdd = 0;
        BTB[i].taken = false;
    }
}
int btb_traversor(bitset<32> pc, bool taken)
{
    // will check if a particular pc is there or not in the branch target buffer
    int i = 0;
    int flag = 0;

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        if (BTB[i].currentPCAdd == pc)
        { // we have found the same pc in our BTB
            if ((BTB[i].taken) == taken)
            {
                flag = 1; // we have got a instruction which has been taken
            }
            else
            {
                flag = -1; // found but not taken
            }
        }
        i++;
        bitset<32> check_pc = BTB[i].currentPCAdd;
    }
    return flag;
}
void btb_runner(bitset<32> pc, bitset<32> ta, bool taken)
{
    // will add suitable entries to our BTB ensuring only discrete values crept in
    int flag = btb_traversor(pc, taken);//flag=1==found and taken//flag=0==not found//flag=-1==found and not taken
    if (flag == 0 && pc != -1)
    { // pc was not there in BTB, so update the BTB
        BTB[BTB_index].currentPCAdd = pc;
        BTB[BTB_index].predictedAdd = ta;
        BTB[BTB_index].taken = taken;
        BTB_index++;
    }
    else if (flag == -1)
    {
        BTB[BTB_index].taken = taken;
    }
    else
    {
        return; // we are not updating our BTB
    }
}
void btb_printer(B_T_B BTB[], int n)
{
    // will print our Branch Target Buffer
    cout << endl
         << endl;
    cout << "                                         BRANCH TARGET BUFFER" << endl
         << endl;
    cout << "_________________________________________________________________________________________" << endl
         << endl;
    cout << "|              PC                  |          Target Address          | Taken/Not Taken |" << endl;
    for (int i = 0; i < n; i++)
    {
        cout << "| " << BTB[i].currentPCAdd << " | ";
        cout << BTB[i].predictedAdd << " | ";
        cout << "       " << BTB[i].taken << "        |";
        cout << endl;
    }
    cout << "_________________________________________________________________________________________" << endl;
}

bitset<32> predicted_address(bitset<32> pc)
{
    // this will give predicted address when the pc is in the BTB, else will give -1, indicating that given pc was not found in the buffer
    int flag = 0;
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        if (BTB[i].currentPCAdd == pc)
        {
            if ((BTB[i].taken) == 1)
            {
                flag = 1;
                return BTB[i].predictedAdd;
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
        bitset<32> check_pc = BTB[i].currentPCAdd;
    }
    return flag;
}

int Clock = 0; // this will store the No. of Clock cycle used for a program
bool HaltIF = false, HaltDE = false;
bool DataHazard = false,    // true if there is data hazard
    ControlHazard = false,  // true if there is control hazard
    RefreshOprands = false, // true if decode is halted beforea and need to refresh its oprands to procede further
    PushNoOp = false;       // true if we have to push no op in the ex-ma stage

bool ispipeLine = false;
bool printRegFile;
bool printPipe;
bool DataForwarding;
int offset = 0;

int blockSize = 4; /*BYTES*/


int No_Stals = 0, No_Lw_Sw = 0, No_ALU_inst = 0, No_Ctrl_Inst = 0, No_Data_Hazard = 0, No_Control_Hazard = 0, No_stals_due_to_dataHazard = 0, No_stals_due_to_ctrlHazard = 0,No_Branch_miss=0;

int misses = 0;
int hits = 0;
int coldmisses = 0;
int conflictmisses = 0;
int capacitymisses = 0;


#pragma region FILE_RELATED_DATA
void printRF();
//  stores the file name of the dump file
string filename;
ifstream readFile("input.mc");
#pragma endregion FILE_RELATED_DATA

#pragma region INSTRUCTION_RELATED_DATA
bitset<32> currentPCAdd(0);
bitset<32> nextPCAdd(0);

bitset<32> currentPCAdd_Pipe(0);
bitset<32> nextPCAdd_Pipe(0);

bitset<32> currentInstruction;
bitset<32> *instructions;
char InstMem[INSTMEM_SIZE];

struct BlockParameters
{
    int tag; 
    char *data;
    int validBit;
    int recencyInfo;
    int frequency;
    int FIFOindex;
};


// bool HaltIF;
// if this instruction is read then program exits
bitset<32> exitInstruction(0xffffffff);

// stores the pc + 4
#pragma endregion INSTRUCTION_RELATED_DATA
bitset<32> predicted_address(bitset<32>);

#pragma region DECODE_RELATED_DATA
typedef struct if_de_hs_de_ex
{
    int Op1, Op2; // oprands
    int Rd;       // RF write destinstion
    int imm, immU, immS, immJ, immB;
    int branch_target_select; // 0 for immB; 1 for immJ
    int Result_select;        // 0:PC+4 ; 1:ImmU; 2:Load data; 3: ALU result
    int ALU_Operation;        // 0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt 9:beq 10:bne 11:blt 12:bge 13:lui 14:jal 15:jalr 16: auipc
    int mem_OP;               // 0:No operation 1:write 2:read
    int RFWrite;              // 0: no write operation 1:for write operation
    int Store_load_op;        // 0:byte 1:half 2:word//to be used by mem to decide how many bit to store
    int Mem_Op2;              // OP2 input for memmory
} If_DE;
If_DE hs_de_ex;

typedef struct de_ex_pipeline
{
    int Op1, Op2; // oprands
    int Rd;       // RF write destinstion
    int Rs1, Rs2;
    char InstType;
    int imm, immU, immS, immJ, immB;
    int branch_target_select; // 0 for immB; 1 for immJ
    int Result_select;        // 0:PC+4 ; 1:ImmU; 2:Load data; 3: ALU result
    int ALU_Operation;        // 0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt 9:beq 10:bne 11:blt 12:bge 13:lui 14:jal 15:jalr 16: auipc
    int mem_OP;               // 0:No operation 1:write 2:read
    int RFWrite;              // 0: no write operation 1:for write operation
    int Store_load_op;        // 0:byte 1:half 2:word//to be used by mem to decide how many bit to store
    int Mem_Op2;
    bitset<32> nextPCAdd = 0;    // stores the next address to jump
    bitset<32> CurrentPCAdd = 0; // stores the current PC address

} de_ex_pipe;

de_ex_pipe de_ex_mainPipeline, de_ex_No_Op;

int RF[32]; // Register file

typedef struct ex_ma_handshake
{
    // handshake register between execute and memory access
    int ALU_result; // 0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt
    int isBranch;   // will tell whether to branch or not
    // branch target address
} EX_MA;
EX_MA hs_ex_ma;

typedef struct ex_ma_pipeline
{
    int ALU_result; // will store the result of the alu
    int isBranch;   // will tell whether to branch or not

    int Rd;                   // RF write destinstion
    int immU;                 // to be written in the rd
    int branch_target_select; // 0 for immB; 1 for immJ
    int Result_select;        // 0:PC+4 ; 1:ImmU; 2:Load data; 3: ALU result
    int mem_OP;               // 0:No operation 1:write 2:read
    int RFWrite;              // 0: no write operation 1:for write operation
    int Store_load_op;        // 0:byte 1:half 2:word//to be used by mem to decide how many bit to store
    int Mem_Op2;
    bitset<32> nextPCAdd = 0;    // stores the next address to jump
    bitset<32> CurrentPCAdd = 0; // stores the current PC address
} ex_ma_pipe;

ex_ma_pipe ex_ma_mainPipeline, ex_ma_No_Op;
#pragma endregion EXECUTE_RELATED_DATA

#pragma region MEM_ACC_RELATED_DATA
typedef struct ma_wb_handshake
{ // handshake register between memory access and write back
    int loaded_mem;
} MA_WB;
MA_WB hs_ma_wb;

char *memory_arr = (char *)calloc(MEMORY_SIZE, sizeof(char));

typedef struct ma_wb_pipeline
{
    int loaded_mem;

    int Rd;   // RF write destinstion
    int immU; // to be written in the rd
    // int branch_target_select; // 0 for immB; 1 for immJ
    int Result_select;           // 0:PC+4 ; 1:ImmU; 2:Load data; 3: ALU result
    int RFWrite;                 // 0: no write operation 1:for write operation
    int ALU_result;              // will store the result of the alu
    int isBranch;                // will tell whether to branch or not
    bitset<32> nextPCAdd = 0;    // stores the next address to jump
    bitset<32> CurrentPCAdd = 0; // stores the current PC address
} ma_wb_pipe;

ma_wb_pipe ma_wb_mainPipeline, ma_wb_No_Op;
#pragma endregion MEM_ACC_RELATED_DATA

void make_file()
{
    cout << "Enter input filename: ";
    // cin >> filename;
    filename = "Bubble_Asad.txt";

    ifstream infile;
    // the input dump file from venus
    // format of it is
    // 0x12312312
    infile.open(filename);
    ofstream outfile;
    outfile.open("input.mc");
    if (!infile)
    {
        cout << "Error: Could not open input file.\n";
        exit(0);
    }

    string line;
    offset = 0;
    // total no. instructions
    int n = 0;
    // instructions = (bitset<32>*)malloc(sizeof(bitset<32>));
    // int sizeToAllocate = sizeof(bitset<32>);
    while (getline(infile, line))
    {
        // cout << "0x" << hex<<offset << " " << line << endl;
        // skip 0x part
        string hex_str = line;
        hex_str = "0x" + line;
        // stoul converts string of type 0x012312 to its decimal value
        unsigned long hex_to_dec_val = stoul(hex_str, nullptr, 16);
        if (offset > INSTMEM_SIZE - 4)
        {
            cout << "Instruction Memory is full. EXITING";
            exit(0);
        }
        int *InstAdd = (int *)(InstMem + offset /*PC ADD. here*/);

        *InstAdd = hex_to_dec_val;

        // bitset<32> binary_num(hex_to_dec_val);
        // sizeToAllocate += sizeof(bitset<32>);
        // instructions = (bitset<32>*)realloc(instructions, sizeToAllocate);
        // *(instructions + (n++)) = binary_num;
        // cout<<binary_num<<endl;
        outfile << "0x" << hex << offset << " " << line << endl;
        offset += 4; // increase offset by 4 characters
    }
    infile.close();
    outfile.close();
}

void printRF()
{
    cout << "\nRegister File is: \n";
    for (int i = 0; i < 32; i++)
    {
        cout << "x" << i << "=" << RF[i] << endl;
    }
}

class Decode
{
    void Decode_Instruction()
    {
        cout << "\n### Decode ###\n\n";
        bitset<5> opcode;
        char type_of_instruction;

        for (int i = 0; i < 5; i++) // ignoring last two bit as these are always 11
        {
            opcode[i] = currentInstruction[i + 2];
        }

        if ((!opcode[4]) && opcode[3] && opcode[2] && (!opcode[1]) && (!opcode[0])) // 01100
            type_of_instruction = 'R';

        else if ((!opcode[4]) && (!opcode[3]) && (!opcode[1]) && (!opcode[0]) || opcode[4] && opcode[3] && (!opcode[2]) && (!opcode[1]) && opcode[0]) // 00X00 OR 11001
            type_of_instruction = 'I';

        else if ((!opcode[4]) && opcode[3] && (!opcode[2]) && (!opcode[1]) && (!opcode[0])) // 01000
            type_of_instruction = 'S';

        else if (opcode[4] && opcode[3] && (!opcode[2]) && (!opcode[1]) && (!opcode[0])) // 11000
            type_of_instruction = 'B';

        else if (opcode[4] && opcode[3] && (!opcode[2]) && opcode[1] && opcode[0]) // 11011
            type_of_instruction = 'J';

        else if (opcode[4] && opcode[3] && (!opcode[2]) && (!opcode[1]) && opcode[0]) // 11001
            type_of_instruction = 'I';

        else if ((!opcode[4]) && opcode[2] && (!opcode[1]) && opcode[0]) // 0X101
            type_of_instruction = 'U';

        else if (opcode[4] && opcode[3] && opcode[2] && (!opcode[1]) && (!opcode[0])) // 11100
            type_of_instruction = 'I';

        else
            type_of_instruction = 'Z';

        de_ex_mainPipeline.InstType = type_of_instruction;
        switch (type_of_instruction)
        {
        case 'R':
        {
            cout << "\nDECODE: ";
            bitset<3> func3;
            func3[0] = currentInstruction[12];
            func3[1] = currentInstruction[13];
            func3[2] = currentInstruction[14];

            switch (func3.to_ulong()) // alu op:: 0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt
            {
            case 0:
            {
                if (currentInstruction[30])
                {
                    hs_de_ex.ALU_Operation = 1;
                    cout << "Operation is SUB";
                }
                else
                {
                    hs_de_ex.ALU_Operation = 0;
                    cout << "Operation is ADD";
                }
            }
            break;
            case 4:
                hs_de_ex.ALU_Operation = 2;
                cout << "Operation is XOR";

                break;
            case 6:
                hs_de_ex.ALU_Operation = 3;
                cout << "Operation is OR";
                break;
            case 7:
                hs_de_ex.ALU_Operation = 4;
                cout << "Operation is AND";
                break;
            case 1: // 5:sll 6:srl 7:sra 8:slt
                hs_de_ex.ALU_Operation = 5;
                cout << "Operation is SLL";
                break;
            case 5:
            {
                if (currentInstruction[30])
                {
                    hs_de_ex.ALU_Operation = 7;
                    cout << "Operation is SRA";
                }
                else
                {
                    hs_de_ex.ALU_Operation = 6;
                    cout << "Operation is SRL";
                }
            }
            break;
            case 2:
                hs_de_ex.ALU_Operation = 8;
                cout << "Operation is SLT";
                break;

            default:
            {
                cout << "Invalid Instruction\nEXITING...";
                exit(0);
            }
            break;
            }

            // cout<<"operation is"<<hs_de_ex.ALU_Operation;

            hs_de_ex.Result_select = 3;
            hs_de_ex.mem_OP = 0;
            hs_de_ex.RFWrite = 1;

            bitset<5> rs1;
            bitset<5> rs2;
            bitset<5> rd;

            for (int i = 0; i < 5; i++)
            {
                rs1[i] = currentInstruction[i + 15];
                rs2[i] = currentInstruction[i + 20];
                rd[i] = currentInstruction[i + 7];
            }
            cout << ", First Operand X" << rs1.to_ulong() << ", Second Operand X" << rs2.to_ulong() << ", Destination Registor X" << rd.to_ulong() << endl;
            hs_de_ex.Op1 = RF[rs1.to_ulong()]; // value of rs1
            hs_de_ex.Op2 = RF[rs2.to_ulong()]; // value of rs2

            de_ex_mainPipeline.Rs1 = rs1.to_ulong();
            de_ex_mainPipeline.Rs2 = rs2.to_ulong();

            hs_de_ex.Rd = rd.to_ulong(); // address of RD
            cout << "DECODE: "
                 << "Read Register X" << rs1.to_ulong() << " = " << hs_de_ex.Op1 << ", X" << rs2.to_ulong() << " = " << hs_de_ex.Op2 << endl;
        }
        break;

        case 'I':
        {
            cout << "DECODE: I type instruction\n";
            bitset<3> func3;
            func3[0] = currentInstruction[12];
            func3[1] = currentInstruction[13];
            func3[2] = currentInstruction[14];

            if (currentInstruction[4]) // arithmatic operations
            {
                cout << "Arithmetic Opereation\n";
                switch (func3.to_ulong()) // alu op:: 0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt
                {
                case 0:
                    hs_de_ex.ALU_Operation = 0;
                    cout << "Operation is ADD";
                    break;
                case 4:
                    hs_de_ex.ALU_Operation = 2;
                    cout << "Operation is XOR";
                    break;
                case 6:
                    hs_de_ex.ALU_Operation = 3;
                    cout << "Operation is OR";
                    break;
                case 7:
                    hs_de_ex.ALU_Operation = 4;
                    cout << "Operation is AND";
                    break;
                case 1:
                    hs_de_ex.ALU_Operation = 5;
                    cout << "Operation is SLL";
                    break;
                case 5:
                {
                    if (currentInstruction[30])
                    {
                        hs_de_ex.ALU_Operation = 7;
                        cout << "Operation is SRA";
                    }
                    else
                    {
                        hs_de_ex.ALU_Operation = 6;
                        cout << "Operation is SRL";
                    }
                }
                break;
                case 2:
                    hs_de_ex.ALU_Operation = 8;
                    cout << "Operation is SLT";
                    break;

                default:
                {
                    cout << "Invalid Instruction. Exiting...";
                    exit(0);
                }
                break;
                }
                hs_de_ex.Result_select = 3;
                hs_de_ex.mem_OP = 0;
                hs_de_ex.RFWrite = 1;
            }

            else if (!currentInstruction[6]) // Load operation
            {
                cout << "Load Opereation\n";
                hs_de_ex.ALU_Operation = 0; // add
                hs_de_ex.Store_load_op = func3.to_ulong();
                hs_de_ex.Result_select = 2; // load data
                hs_de_ex.mem_OP = 2;        // read
                hs_de_ex.RFWrite = 1;
            }
            else // jalr
            {
                cout << "JALR Opereation\n";
                hs_de_ex.ALU_Operation = 15; // jalr
                hs_de_ex.Result_select = 0;  // PC+4
                hs_de_ex.mem_OP = 0;         // no operation
                hs_de_ex.RFWrite = 1;
            }

            bitset<5> rs1;
            bitset<5> rd;
            bitset<12> temp;

            for (int i = 0; i < 5; i++)
            {
                rs1[i] = currentInstruction[i + 15];
                rd[i] = currentInstruction[i + 7];
            }

            for (int i = 0; i < 12; i++)
            {
                temp[i] = currentInstruction[i + 20];
            }

            bitset<32> imm;

            for (int i = 0; i < 12; i++)
            {
                imm[i] = temp[i];
            }

            for (int i = 12; i < 32; i++)
            {
                imm[i] = temp[11];
            }

            hs_de_ex.Op1 = RF[rs1.to_ulong()];  // value of rs1
            hs_de_ex.Op2 = (int)imm.to_ulong(); // value of rs2
            hs_de_ex.Rd = rd.to_ulong();        // address of RD

            de_ex_mainPipeline.Rs1 = rs1.to_ulong();
            de_ex_mainPipeline.Rs2 = -1;

            cout << ", First Operand X" << rs1.to_ulong() << ", Second Operand imm"
                 << ", Destination Registor X" << rd.to_ulong() << endl;
            cout << "DECODE: "
                 << "Read Register X" << rs1.to_ulong() << " = " << hs_de_ex.Op1 << ", imm = " << hs_de_ex.Op2 << endl;
        }
        break;

        case 'S':
        {
            cout << "S type instruction\n";
            bitset<3> func3;
            func3[0] = currentInstruction[12];
            func3[1] = currentInstruction[13];
            func3[2] = currentInstruction[14];

            hs_de_ex.ALU_Operation = 0; // add
            hs_de_ex.Store_load_op = func3.to_ulong();
            hs_de_ex.mem_OP = 1; // write operation
            hs_de_ex.RFWrite = 0;

            bitset<5> rs1;
            bitset<5> rs2;

            for (int i = 0; i < 5; i++)
            {
                rs1[i] = currentInstruction[i + 15];
                rs2[i] = currentInstruction[i + 20];
            }

            bitset<12> temp;
            for (int i = 0; i < 5; i++)
            {
                temp[i] = currentInstruction[7 + i];
            }

            for (int i = 5; i < 12; i++)
            {
                temp[i] = currentInstruction[i + 20];
            }

            bitset<32> immS;
            for (int i = 0; i < 12; i++)
            {
                immS[i] = temp[i];
            }

            for (int i = 12; i < 32; i++)
            {
                immS[i] = temp[11];
            }

            hs_de_ex.Op1 = RF[rs1.to_ulong()];
            hs_de_ex.Op2 = (int)immS.to_ulong();
            hs_de_ex.Mem_Op2 = RF[rs2.to_ulong()];

            de_ex_mainPipeline.Rs1 = rs1.to_ulong();
            de_ex_mainPipeline.Rs2 = rs2.to_ulong();

            cout << "First Operand X" << rs1.to_ulong() << ", Second Operand immS = " << immS.to_ulong() << endl;
            cout << "DECODE: "
                 << "Read Register X" << rs1.to_ulong() << " = " << hs_de_ex.Op1 << ", X" << rs2.to_ulong() << " = " << hs_de_ex.Mem_Op2 << endl;
        }
        break;
        case 'B':
        {
            cout << "B type instruction\n";
            hs_de_ex.branch_target_select = 0; // immB
            hs_de_ex.RFWrite = 0;
            hs_de_ex.mem_OP = 0;

            bitset<5> rs1;
            bitset<5> rs2;

            for (int i = 0; i < 5; i++)
            {
                rs1[i] = currentInstruction[i + 15];
                rs2[i] = currentInstruction[i + 20];
            }

            hs_de_ex.Op1 = RF[rs1.to_ulong()];
            hs_de_ex.Op2 = RF[rs2.to_ulong()];

            de_ex_mainPipeline.Rs1 = rs1.to_ulong();
            de_ex_mainPipeline.Rs2 = rs2.to_ulong();

            bitset<13> temp;

            temp[0] = 0; // lsb is 0
            temp[11] = currentInstruction[7];
            temp[12] = currentInstruction[31];

            for (int i = 1; i < 5; i++)
            {
                temp[i] = currentInstruction[7 + i];
            }

            for (int i = 5; i < 11; i++)
            {
                temp[i] = currentInstruction[20 + i];
            }

            bitset<32> immB;

            for (int i = 0; i < 13; i++)
            {
                immB[i] = temp[i];
            }

            for (int i = 13; i < 32; i++)
            {
                immB[i] = temp[12];
            }

            hs_de_ex.immB = (int)immB.to_ulong();

            bitset<3> func3;
            func3[0] = currentInstruction[12];
            func3[1] = currentInstruction[13];
            func3[2] = currentInstruction[14];

            switch (func3.to_ulong()) // aluOP== 9:beq 10:bne 11:blt 12:bge
            {
            case 0:
            {
                hs_de_ex.ALU_Operation = 9;
                cout << "BEQ";
            }
            break;
            case 1:
            {
                cout << "BNE";
                hs_de_ex.ALU_Operation = 10;
            }
            break;
            case 4:
            {
                cout << "BLT";
                hs_de_ex.ALU_Operation = 11;
            }
            break;
            case 5:
            {
                cout << "BGE";
                hs_de_ex.ALU_Operation = 12;
            }
            break;

            default:
            {
                cout << "Error: Could not identify instruction.\n";
                exit(0);
            }
            break;
            }
            cout << "\nFirst Operand X" << rs1.to_ulong() << ", Second Operand X" << rs2.to_ulong() << endl;
            cout << "DECODE: "
                 << "Read Register X" << rs1.to_ulong() << " = " << hs_de_ex.Op1 << ", X" << rs2.to_ulong() << " = " << hs_de_ex.Op2 << ", immB = " << immB.to_ulong() << endl;
        }
        break;

        case 'U':
        {
            cout << "U type instruction\n";

            if (currentInstruction[5]) // lui
            {
                hs_de_ex.ALU_Operation = 13;

                bitset<32> immU;
                for (int i = 0; i < 32; i++)
                {
                    if (i >= 12)
                        immU[i] = currentInstruction[i];
                    else
                        immU[i] = 0;
                }

                hs_de_ex.immU = (int)immU.to_ulong();
                hs_de_ex.Result_select = 1;
                hs_de_ex.mem_OP = 0;
                hs_de_ex.RFWrite = 1;

                bitset<5> rd;

                for (int i = 0; i < 5; i++)
                {
                    rd[i] = currentInstruction[i + 7];
                }

                hs_de_ex.Rd = rd.to_ulong();

                cout << "LUI\n";
                cout << "DECODE: "
                     << "Destination Register X" << rd.to_ulong() << endl;
                cout << "immU = " << immU.to_ulong() << endl;
            }
            else // auipc
            {
                hs_de_ex.ALU_Operation = 16; // must be add

                bitset<32> immU;
                for (int i = 0; i < 32; i++)
                {
                    if (i >= 12)
                        immU[i] = currentInstruction[i];
                    else
                        immU[i] = 0;
                }

                bitset<5> rd;

                for (int i = 0; i < 5; i++)
                {
                    rd[i] = currentInstruction[i + 7];
                }
                hs_de_ex.Rd = rd.to_ulong();

                hs_de_ex.immU = immU.to_ulong();

                hs_de_ex.Op1 = immU.to_ulong();         // op1=immu
                hs_de_ex.Op2 = currentPCAdd.to_ulong(); // op1=immu

                hs_de_ex.mem_OP = 0;
                hs_de_ex.RFWrite = 1;
                hs_de_ex.Result_select = 1;

                de_ex_mainPipeline.Rs1 = -1;
                de_ex_mainPipeline.Rs2 = -1;

                cout << "AUIPC\n";
                cout << "DECODE: "
                     << "Destination Register X" << rd.to_ulong() << endl;
                cout << "First Operand immU = " << immU.to_ulong() << ", Second Operand PC" << currentPCAdd.to_ulong() << endl;
            }
        }
        break;

        case 'J':
        {
            cout << "J type instruction\n";
            bitset<21> temp(0);

            for (int i = 12; i < 20; i++)
            {
                temp[i] = currentInstruction[i];
            }

            temp[11] = currentInstruction[20];
            temp[0] = 0;

            for (int i = 1; i < 11; i++)
            {
                temp[i] = currentInstruction[i + 20];
            }

            temp[20] = currentInstruction[31];

            bitset<32> immJ;
            for (int i = 0; i < 21; i++)
            {
                immJ[i] = temp[i];
            }

            for (int i = 21; i < 32; i++)
            {
                immJ[i] = temp[20];
            }

            // cout<<"IMMj="<<ImmJ;

            bitset<5> rd;

            for (int i = 0; i < 5; i++)
            {
                rd[i] = currentInstruction[i + 7];
            }

            hs_de_ex.immJ = (int)immJ.to_ulong();
            hs_de_ex.mem_OP = 0;
            hs_de_ex.branch_target_select = 1; // immj
            hs_de_ex.Rd = rd.to_ulong();
            hs_de_ex.ALU_Operation = 14; // jal
            hs_de_ex.Result_select = 0;
            hs_de_ex.RFWrite = 1;

            de_ex_mainPipeline.Rs1 = -1;
            de_ex_mainPipeline.Rs2 = -1;

            cout << "DECODE: "
                 << "Destination Register X" << rd.to_ulong() << ", immJ = " << immJ.to_ulong() << endl;
        }
        break;

        default:
        {
            cout << "Error: Could not identify instruction.\n";
            exit(0);
        }
        break;
        }

        cout << "\n### End Decode ###\n\n";

        // copying the data to Copy Of the pipe line
        de_ex_mainPipeline.Op1 = hs_de_ex.Op1;
        de_ex_mainPipeline.Op2 = hs_de_ex.Op2;
        de_ex_mainPipeline.Rd = hs_de_ex.Rd;
        de_ex_mainPipeline.imm = hs_de_ex.imm;
        de_ex_mainPipeline.immU = hs_de_ex.immU;
        de_ex_mainPipeline.immS = hs_de_ex.immS;
        de_ex_mainPipeline.immJ = hs_de_ex.immJ;
        de_ex_mainPipeline.immB = hs_de_ex.immB;
        de_ex_mainPipeline.branch_target_select = hs_de_ex.branch_target_select;
        de_ex_mainPipeline.Result_select = hs_de_ex.Result_select;
        de_ex_mainPipeline.ALU_Operation = hs_de_ex.ALU_Operation;
        de_ex_mainPipeline.mem_OP = hs_de_ex.mem_OP;
        de_ex_mainPipeline.RFWrite = hs_de_ex.RFWrite;
        de_ex_mainPipeline.Store_load_op = hs_de_ex.Store_load_op;
        de_ex_mainPipeline.Mem_Op2 = hs_de_ex.Mem_Op2;

        de_ex_mainPipeline.nextPCAdd = nextPCAdd_Pipe.to_ulong();
        de_ex_mainPipeline.CurrentPCAdd = currentPCAdd_Pipe.to_ulong();
    }

public:
    Decode()
    {
        if (!HaltDE)
            Decode_Instruction();
        else
            HaltDE = false;
    }
};

class Execute
{
    int srl(int op1_int, int op2)
    {
        BTB[0].taken = true;
        // this will ensure logical right shift in case of SRL instruction
        bitset<32> op1 = op1_int; // convert input decimal number to a bitset
        bitset<32> op1_shifted;   // local bitset for storing shifted bitset
        for (int i = 0; i < 32; i++)
        {
            op1_shifted[i] = op1[i + op2];
            if (i + op2 == 31)
            {
                break;
            }
        }
        int shifted_dec = op1_shifted.to_ulong(); // converting back the shifted bitset to integer
        return shifted_dec;
    }

    void execute_inst()
    {
        int op1 = de_ex_mainPipeline.Op1;
        int op2 = de_ex_mainPipeline.Op2;
        int ALU_operation = de_ex_mainPipeline.ALU_Operation;
        ex_ma_mainPipeline.immU = de_ex_mainPipeline.immU;

        // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.CurrentPCAdd.to_ulong()+4;
        ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.nextPCAdd.to_ulong();
        ex_ma_mainPipeline.CurrentPCAdd = de_ex_mainPipeline.CurrentPCAdd.to_ulong();

        cout << "\n### Execute ###\n\n";

        switch (ALU_operation)
        {
        case 0: // it will perform addition in ALU also will compute effective address for S and Load instruction

            cout << "ALU performing addition operation" << endl;
            cout << "Execute: ADD " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 + op2;
            ex_ma_mainPipeline.isBranch = 0;
            No_ALU_inst++;
            break;
        case 1: // it will perform subtraction in ALU
            cout << "ALU performing subtraction operation" << endl;
            cout << "Execute: SUB " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 - op2;
            ex_ma_mainPipeline.isBranch = 0;
            No_ALU_inst++;
            break;
        case 2: // it will perform logical XOR in ALU
            cout << "ALU performing XOR operation" << endl;
            cout << "Execute: XOR " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 ^ op2;
            ex_ma_mainPipeline.isBranch = 0;
            No_ALU_inst++;
            break;
        case 3: // it will perform logical OR in ALU
            cout << "ALU performing OR operation" << endl;
            cout << "Execute: OR " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 | op2;
            ex_ma_mainPipeline.isBranch = 0;
            No_ALU_inst++;
            break;
        case 4: // it will perform logical AND in ALU
            cout << "ALU performing AND operation" << endl;
            cout << "Execute: AND " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 & op2;
            ex_ma_mainPipeline.isBranch = 0;
            No_ALU_inst++;
            break;
        case 5: // it will perform shift left logical in ALU
            cout << "ALU performing logical left shift operation" << endl;
            cout << "Execute: SLL " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 << op2;
            ex_ma_mainPipeline.isBranch = 0;
            No_ALU_inst++;
            break;
        case 6: // it will perform shift right logical in ALU
            ex_ma_mainPipeline.ALU_result = srl(op1, op2);
            cout << "ALU performing logical right shift operation" << endl;
            cout << "Execute: SRL " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.isBranch = 0;
            No_ALU_inst++;
            break;
        case 7: // it will perform shift right arithmetic in ALU
            cout << "ALU performing arithmetic right shift operation" << endl;
            cout << "Execute: SRA " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 >> op2;
            ex_ma_mainPipeline.isBranch = 0;
            No_ALU_inst++;
            break;

        case 8: // it will perform set less than in ALU
            cout << "ALU performing set less than operation" << endl;
            cout << "Execute: SLT " << op1 << " and " << op2 << endl;
            if (op1 < op2)
            {
                ex_ma_mainPipeline.ALU_result = 1;
            }
            else
                ex_ma_mainPipeline.ALU_result = 0;

            ex_ma_mainPipeline.isBranch = 0;
            No_ALU_inst++;
            break;
            // for branching, we are assigning 0 for no branch, 1 for branch target adress, and 2 for ALU result, i.e for JALR

        case 9: // will check for beq
            cout << "ALU checking for beq" << endl;
            cout << "Execute: SUB " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 - op2;
            if (ex_ma_mainPipeline.ALU_result == 0)
            {
                ex_ma_mainPipeline.isBranch = 1;
                cout << "Branching done" << endl;
                cout << "immb=" << de_ex_mainPipeline.immB << endl;
                // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.immB + currentPCAdd.to_ulong();
                ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.immB + de_ex_mainPipeline.CurrentPCAdd.to_ulong();

                cout << "Next Pc updated to: " << ex_ma_mainPipeline.nextPCAdd.to_ulong() << endl;
                btb_runner(de_ex_mainPipeline.CurrentPCAdd, ex_ma_mainPipeline.nextPCAdd, 1);
            }
            else
            {
                ex_ma_mainPipeline.isBranch = 0;
                cout << "No branching" << endl;
                btb_runner(de_ex_mainPipeline.CurrentPCAdd, ex_ma_mainPipeline.nextPCAdd, 0);
            }
            No_Ctrl_Inst++;
            break;

        case 10: // will check for bne
            cout << "Execute: SUB " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 - op2;
            if (ex_ma_mainPipeline.ALU_result == 0)
            {
                ex_ma_mainPipeline.isBranch = 0;
                btb_runner(de_ex_mainPipeline.CurrentPCAdd, ex_ma_mainPipeline.nextPCAdd, 0);
                cout << "No branching" << endl;
            }
            else
            {
                ex_ma_mainPipeline.isBranch = 1;
                cout << "Branching done" << endl;
                // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.immB + currentPCAdd.to_ulong();
                ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.immB + de_ex_mainPipeline.CurrentPCAdd.to_ulong();
                cout << "Next Pc updated to: " << ex_ma_mainPipeline.nextPCAdd.to_ulong() << endl;
                btb_runner(de_ex_mainPipeline.CurrentPCAdd, ex_ma_mainPipeline.nextPCAdd, 1);
            }
            No_Ctrl_Inst++;
            break;

        case 11: // will check for blt
            cout << "Execute: SUB " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 - op2;
            if (ex_ma_mainPipeline.ALU_result < 0)
            {
                ex_ma_mainPipeline.isBranch = 1;
                cout << "Branching done" << endl;
                // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.immB + currentPCAdd.to_ulong();
                ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.immB + de_ex_mainPipeline.CurrentPCAdd.to_ulong();
                cout << "Next Pc updated to: " << ex_ma_mainPipeline.nextPCAdd.to_ulong() << endl;
                btb_runner(de_ex_mainPipeline.CurrentPCAdd, ex_ma_mainPipeline.nextPCAdd, 1);
            }
            else
            {
                ex_ma_mainPipeline.isBranch = 0;
                btb_runner(de_ex_mainPipeline.CurrentPCAdd, ex_ma_mainPipeline.nextPCAdd, 0);
                cout << "No branching" << endl;
            }
            No_Ctrl_Inst++;
            break;

        case 12: // will check for bge
            cout << "Execute: SUB " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 - op2;
            cout << "OP1:" << de_ex_mainPipeline.Op1;
            cout << "OP2:" << de_ex_mainPipeline.Op2;
            if (ex_ma_mainPipeline.ALU_result >= 0)
            {
                ex_ma_mainPipeline.isBranch = 1;
                cout << "Branching done" << endl;
                // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.immB + currentPCAdd.to_ulong();
                ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.immB + de_ex_mainPipeline.CurrentPCAdd.to_ulong();
                cout << "Next Pc updated to: " << ex_ma_mainPipeline.nextPCAdd.to_ulong() << endl;
                btb_runner(de_ex_mainPipeline.CurrentPCAdd, ex_ma_mainPipeline.nextPCAdd, 1);
            }
            else
            {
                ex_ma_mainPipeline.isBranch = 0;
                btb_runner(de_ex_mainPipeline.CurrentPCAdd, ex_ma_mainPipeline.nextPCAdd, 0);
                cout << "No branching" << endl;
            }
            No_Ctrl_Inst++;
            break;

        case 13: // lui
            ex_ma_mainPipeline.isBranch = 0;
            cout << "Executin LUI " << endl;
            break;

        case 14: // JAL
            cout << "Executing JAL" << endl;
            ex_ma_mainPipeline.isBranch = 1;
            // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.immJ + currentPCAdd.to_ulong();
            ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.immJ + de_ex_mainPipeline.CurrentPCAdd.to_ulong();

            cout << "Next Pc updated to: " << ex_ma_mainPipeline.nextPCAdd.to_ulong() << endl;
            btb_runner(de_ex_mainPipeline.CurrentPCAdd, ex_ma_mainPipeline.nextPCAdd, 1);
            // cout<<"jal worked :"<<nextPCAdd;
            No_ALU_inst++;
            No_Ctrl_Inst++;
            break;

        case 15: // jalr
            cout << "Executing JALR" << endl;
            cout << "Execute: ADD " << op1 << " and " << op2 << endl;
            ex_ma_mainPipeline.ALU_result = op1 + op2; // op1= rs1 and op2 = imm
            ex_ma_mainPipeline.isBranch = 2;
            ex_ma_mainPipeline.nextPCAdd = op1 + op2;
            cout << "Next Pc updated to: " << ex_ma_mainPipeline.nextPCAdd.to_ulong() << endl;
            btb_runner(de_ex_mainPipeline.CurrentPCAdd, ex_ma_mainPipeline.ALU_result, 1);
            // must give rd in jalr for xi=pc+4
            No_ALU_inst++;
            No_Ctrl_Inst++;
            break;

        case 16: // auipc
            cout << "Executing AUIPC" << endl;
            ex_ma_mainPipeline.isBranch = 0;
            de_ex_mainPipeline.immU = currentPCAdd.to_ulong() + de_ex_mainPipeline.immU;

            break;

        default:
            cout << "Some error has occured in decode!!";
        }

        ex_ma_mainPipeline.branch_target_select = de_ex_mainPipeline.branch_target_select;
        ex_ma_mainPipeline.Rd = de_ex_mainPipeline.Rd;
        ex_ma_mainPipeline.Result_select = de_ex_mainPipeline.Result_select;
        ex_ma_mainPipeline.mem_OP = de_ex_mainPipeline.mem_OP;
        ex_ma_mainPipeline.RFWrite = de_ex_mainPipeline.RFWrite;
        ex_ma_mainPipeline.Store_load_op = de_ex_mainPipeline.Store_load_op;
        ex_ma_mainPipeline.Mem_Op2 = de_ex_mainPipeline.Mem_Op2;

        cout << "\n### End Execute ###\n\n";
    }

public:
    Execute()
    {
        execute_inst(); // making a public constructor function
    }
};

class Memory_Access
{
    void memory_access()
    {
        int memop = ex_ma_mainPipeline.mem_OP;

        int aluresult = ex_ma_mainPipeline.ALU_result;

        int *mem_add = (int *)(memory_arr + aluresult);

        int storeloadop = ex_ma_mainPipeline.Store_load_op;
        int memop2 = ex_ma_mainPipeline.Mem_Op2;
        int loaddata;

        cout << "\n### Memory Access ###\n\n";
        cout << "MEMORY: ";

        switch (memop)
        {
        case 0: // no memory operation
            cout << "Not a memory operation" << endl;
            break;

        case 1:
        { // write
                No_Lw_Sw++;
                int data;
                char *arr = new char[4];
                int bytes_to_read;
                switch (storeloadop)
                {
                        case 0:
                            // *mem_add = memop2 & 255; // sb
                            data = memop2 & 255;
                            cout << "Storing byte " << (memop2 & 255) << endl;
                            bytes_to_read=1;
                            break;

                        case 1:
                            // *mem_add = memop2 & 65535; // sh
                            data = memop2 & 65535;
                            cout << "Storing half-word " << (memop2 & 65535) << endl;
                            bytes_to_read=2;
                            break;

                        case 2:
                            // *mem_add = memop2; // sw
                            data = memop2;
                            cout << "Storing word " << memop2 << endl;
                            bytes_to_read=4;
                            break;
                }
                int *ptr = (int *) arr;
                *ptr = data;

                Placeholder_Name(arr,aluresult,1,bytes_to_read);
        }
        break;

        case 2:
        { // read (load) pending
        int bytesToRead;
            No_Lw_Sw++;
            switch (storeloadop)
            {
            case 0:
                // loaddata = *mem_add; // lb
                // loaddata = loaddata & 255;
                bytesToRead = 1;
                cout << "Loading byte in register" << endl;
                break;

            case 1:
                // loaddata = *mem_add; // lh
                // loaddata = loaddata & 65535;
                bytesToRead = 2;
                cout << "Loading half-word in register" << endl;
                break;

            case 2:
                // loaddata = *mem_add; // lw
                bytesToRead = 4;
                cout << "Loading word in register" << endl;
                break;
            }
            // loaddata = readData(aluresult,storeloadop,0);
            char * output = Placeholder_Name(&nullData, aluresult, 0, bytesToRead);
            int * ptr = (int * ) output;
            hs_ma_wb.loaded_mem = *ptr;
        }
        break;
        }
        
        cout << "\n### End Memmory Access ###\n\n";

        ma_wb_mainPipeline.loaded_mem = loaddata;                            // dont care
        ma_wb_mainPipeline.Rd = ex_ma_mainPipeline.Rd;                       // RF write destinstion
        ma_wb_mainPipeline.immU = ex_ma_mainPipeline.immU;                   // dont care
                                                                             // intmama_wb_mainPipelinech_target_select=0; // 0 for immB; 1 for immJ
        ma_wb_mainPipeline.Result_select = ex_ma_mainPipeline.Result_select; // 3: ALU result
        ma_wb_mainPipeline.RFWrite = ex_ma_mainPipeline.RFWrite;             // 1:for write operation
        // ma_wb_mainPipeline.PC_plus_four = currentPCAdd.to_ulong() + 4; // #################this will not be the case#######################
        ma_wb_mainPipeline.ALU_result = ex_ma_mainPipeline.ALU_result; // will store the result of the alu
        ma_wb_mainPipeline.isBranch = ex_ma_mainPipeline.isBranch;
        ma_wb_mainPipeline.nextPCAdd = ex_ma_mainPipeline.nextPCAdd.to_ulong();
        ma_wb_mainPipeline.CurrentPCAdd = ex_ma_mainPipeline.CurrentPCAdd.to_ulong();
    }

public:
    Memory_Access()
    {
        memory_access();
    }
};

class Write_Back
{

    int resultselect = ma_wb_mainPipeline.Result_select;
    int rfwrite = ma_wb_mainPipeline.RFWrite;
    int isbranch = ma_wb_mainPipeline.isBranch;
    int rd = ma_wb_mainPipeline.Rd;
    int aluresult = ma_wb_mainPipeline.ALU_result;

    // int pc_plus_4 = currentPCAdd.to_ulong() + 4;

    int loadeddata = ma_wb_mainPipeline.loaded_mem;
    int immu = ma_wb_mainPipeline.immU;
    void wb()
    {

        cout << "\n### Write Back ###\n\n";
        cout << "WRITEBACK: ";
        switch (isbranch)
        {
        case 0:
        { // r-type, i-type, failed conditional branching, u-type
            switch (rfwrite)
            {
            case 0: // failed conditional branch

                cout << "No write-back required and current PC updated to PC+4" << endl;
                break;

            case 1: // write data in rf
                switch (resultselect)
                {
                case 0: // pc+4
                    RF[rd] = ma_wb_mainPipeline.CurrentPCAdd.to_ulong() + 4;
                    break;

                case 1: // immu - lui & auipc
                    RF[rd] = immu;
                    break;

                case 2: // ld operation
                    RF[rd] = loadeddata;
                    break;

                case 3: // r-type and i-type operations
                    RF[rd] = aluresult;
                    break;
                }
                cout << "Write " << RF[rd] << " to register " << rd << endl;

                cout << "Current PC updated to PC+4" << endl;
                break;
            }
        }
        break;

        case 1: // jal and conditional branches
            switch (rfwrite)
            {
            case 0: // conditional branch

                cout << "Condtional branch"
                     << ", current PC = " << currentPCAdd << endl;

                break;

            case 1: // jal

                RF[rd] = ma_wb_mainPipeline.CurrentPCAdd.to_ulong() + 4;

                cout << "Write " << RF[rd] << " to register " << rd << endl;
                cout << "Current PC address updated to: " << currentPCAdd;
                break;
            }
            break;

        case 2: // jalr

            RF[rd] = ma_wb_mainPipeline.CurrentPCAdd.to_ulong() + 4;

            cout << "Write " << RF[rd] << " to register " << rd << endl;
            cout << "Current PC address updated to: " << currentPCAdd;
            break;
        }
        RF[0] = 0; // x0 is always 0;

        cout << "\n### End Write Back ###\n\n";
    }

public:
    Write_Back()
    {
        wb();
    }
};

void refreshOprands(bool &RefreshOprands, bool DataHazard, int Rs1DeEx, int Rs2DeEx)
{
    if (RefreshOprands && !DataHazard) // if the decode is previously at halt then we have to refresh its operands so that it contains the new values
    {
        RefreshOprands = false; // resetting the flag

        switch (de_ex_mainPipeline.InstType)
        {
        case 'R':
        {
            de_ex_mainPipeline.Op1 = RF[Rs1DeEx];
            de_ex_mainPipeline.Op2 = RF[Rs2DeEx];
        }
        break;
        case 'I':
        {
            de_ex_mainPipeline.Op1 = RF[Rs1DeEx];
            // if(de_ex_mainPipeline.ALU_Operation == 15)//jalr
            // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.Op1 + de_ex_mainPipeline.imm;
        }
        break;
        case 'S':
        {
            de_ex_mainPipeline.Op1 = RF[Rs1DeEx];
            de_ex_mainPipeline.Mem_Op2 = RF[Rs2DeEx];
        }
        break;
        case 'B':
        {
            de_ex_mainPipeline.Op1 = RF[Rs1DeEx];
            de_ex_mainPipeline.Op2 = RF[Rs2DeEx];
        }
        break;

        default:
        {
            cout << "Some error occured While handeling Hazards, Exiting...";
            exit(0);
        }
        break;
        }
    }
}

void printPipeline()
{
    cout << "currentPCAdd_Pipe " << currentPCAdd_Pipe << endl;
    cout << "nextPCAdd_Pipe " << nextPCAdd_Pipe << endl;
    cout << "current Istruction " << currentInstruction << endl
         << endl;

    cout << "de_ex_mainPipeline.Op1 " << de_ex_mainPipeline.Op1 << endl;
    cout << "de_ex_mainPipeline.Op2 " << de_ex_mainPipeline.Op2 << endl;
    cout << "de_ex_mainPipeline.Rd " << de_ex_mainPipeline.Rd << endl;
    cout << "de_ex_mainPipeline.imm " << de_ex_mainPipeline.imm << endl;
    cout << "de_ex_mainPipeline.immU " << de_ex_mainPipeline.immU << endl;
    cout << "de_ex_mainPipeline.immS " << de_ex_mainPipeline.immS << endl;
    cout << "de_ex_mainPipeline.immJ " << de_ex_mainPipeline.immJ << endl;
    cout << "de_ex_mainPipeline.immB " << de_ex_mainPipeline.immB << endl;
    cout << "de_ex_mainPipeline.branch_target_select " << de_ex_mainPipeline.branch_target_select << endl;
    cout << "de_ex_mainPipeline.Result_select " << de_ex_mainPipeline.Result_select << endl;
    cout << "de_ex_mainPipeline.ALU_Operation " << de_ex_mainPipeline.ALU_Operation << endl;
    cout << "de_ex_mainPipeline.mem_OP " << de_ex_mainPipeline.mem_OP << endl;
    cout << "de_ex_mainPipeline.RFWrite " << de_ex_mainPipeline.RFWrite << endl;
    cout << "de_ex_mainPipeline.Store_load_op " << de_ex_mainPipeline.Store_load_op << endl;
    cout << "de_ex_mainPipeline.Mem_Op2  " << de_ex_mainPipeline.Mem_Op2 << endl;
    cout << "de_ex_mainPipeline.nextPCAdd " << de_ex_mainPipeline.nextPCAdd << endl;
    cout << "de_ex_mainPipeline.CurrentPCAdd " << de_ex_mainPipeline.CurrentPCAdd << endl;
    cout << "de_ex_mainPipeline.InstType " << de_ex_mainPipeline.InstType << endl
         << endl;

    cout << "ex_ma_mainPipeline.isBranch " << ex_ma_mainPipeline.isBranch << endl;
    cout << "ex_ma_mainPipeline.ALU_result " << ex_ma_mainPipeline.ALU_result << endl;
    cout << "ex_ma_mainPipeline.Rd " << ex_ma_mainPipeline.Rd << endl;
    cout << "ex_ma_mainPipeline.immU " << ex_ma_mainPipeline.immU << endl;
    cout << "ex_ma_mainPipeline.branch_target_select " << ex_ma_mainPipeline.branch_target_select << endl;
    cout << "ex_ma_mainPipeline.Result_select " << ex_ma_mainPipeline.Result_select << endl;
    cout << "ex_ma_mainPipeline.mem_OP " << ex_ma_mainPipeline.mem_OP << endl;
    cout << "ex_ma_mainPipeline.RFWrite " << ex_ma_mainPipeline.RFWrite << endl;
    cout << "ex_ma_mainPipeline.Store_load_op " << ex_ma_mainPipeline.Store_load_op << endl;
    cout << "ex_ma_mainPipeline.Mem_Op2 " << ex_ma_mainPipeline.Mem_Op2 << endl;
    cout << "ex_ma_mainPipeline.nextPCAdd " << ex_ma_mainPipeline.nextPCAdd << endl;
    cout << "ex_ma_mainPipeline.CurrentPCAdd " << ex_ma_mainPipeline.CurrentPCAdd << endl
         << endl;

    cout << "ma_wb_mainPipeline.loaded_mem " << ma_wb_mainPipeline.loaded_mem << endl;
    cout << "ma_wb_mainPipeline.Rd " << ma_wb_mainPipeline.Rd << endl;
    cout << "ma_wb_mainPipeline.immU " << ma_wb_mainPipeline.immU << endl;
    cout << "ma_wb_mainPipeline.Result_select " << ma_wb_mainPipeline.Result_select << endl;
    cout << "ma_wb_mainPipeline.RFWrite " << ma_wb_mainPipeline.RFWrite << endl;
    cout << "ma_wb_mainPipeline.ALU_result " << ma_wb_mainPipeline.ALU_result << endl;
    cout << "ma_wb_mainPipeline.isBranch " << ma_wb_mainPipeline.isBranch << endl;
    cout << "ma_wb_mainPipeline.nextPCAdd " << ma_wb_mainPipeline.nextPCAdd << endl;
    cout << "ma_wb_mainPipeline.CurrentPCAdd " << ma_wb_mainPipeline.CurrentPCAdd << endl
         << endl;
}
void resolveHazards()
{
    if (PushNoOp) // pushing the no op in the ex-ma stage
    {
        ex_ma_mainPipeline = ex_ma_No_Op;
        PushNoOp = false;
        No_Stals++;
    }

    int Rs1DeEx = de_ex_mainPipeline.Rs1; // Rs1 of de-ex stage
    int Rs2DeEx = de_ex_mainPipeline.Rs2; // Rs2 of de-ex stage

    
    if ((ex_ma_mainPipeline.isBranch == 1) || (ex_ma_mainPipeline.isBranch == 2))
    {
        if((ex_ma_mainPipeline.nextPCAdd.to_ulong()!=-1)&&(ex_ma_mainPipeline.nextPCAdd.to_ulong()!=de_ex_mainPipeline.CurrentPCAdd.to_ulong()))
        No_Branch_miss++;

        ControlHazard = true;
        No_Control_Hazard++;
        No_stals_due_to_ctrlHazard+=2;
    }

    if (!ControlHazard) // checking only if there is no ctrl hazard
    {
        if (((Rs1DeEx > 0) && ((Rs1DeEx == ex_ma_mainPipeline.Rd) || (Rs1DeEx == ma_wb_mainPipeline.Rd)))) // hazard due to Rs1
        {
            DataHazard = true;
            RefreshOprands = true;
            No_Data_Hazard++;
            No_stals_due_to_dataHazard++;
        }
        if (((Rs2DeEx > 0) && ((Rs2DeEx == ex_ma_mainPipeline.Rd) || (Rs2DeEx == ma_wb_mainPipeline.Rd)))) // hazard due to Rs2
        {
            DataHazard = true;
            RefreshOprands = true;
            No_Data_Hazard++;
            No_stals_due_to_dataHazard++;
        }
    }
    refreshOprands(RefreshOprands, DataHazard, Rs1DeEx, Rs2DeEx); // refreshes the oprands
    // cout<<"Hello";
    cout << endl
         << "This is the value of next pc: " << ex_ma_mainPipeline.nextPCAdd.to_ulong() << endl;
    cout << endl
         << "This is the value of curr pc: " << ex_ma_mainPipeline.CurrentPCAdd.to_ulong() << endl;

    if (DataHazard && (!ControlHazard)) // Only dta hazard
    {
        HaltDE = true;      // halting DE for the next cycle
        HaltIF = true;      // halting IF for the next cycle
        PushNoOp = true;    // pushing no op to ex-ma stage in the next cycle
        DataHazard = false; // resetting the flag
    }
    else if (ControlHazard && (!DataHazard)) // Only Control hazard
    {
        currentInstruction = 51; // noOp
        de_ex_mainPipeline = de_ex_No_Op;
        currentPCAdd = ex_ma_mainPipeline.nextPCAdd; // next pc taken from the ex stage
        ControlHazard = false;                       // resetting the flag
    }
    else if (DataHazard && ControlHazard) // Both at the same time
    {
        // case is not possible
        cout << "Unexpected Error Occured";
    }
    else
    {
        cout << "NO data and Control hazard";
    }
}

void ResolveHazard_Using_dataForwarding()
{
    if (PushNoOp) // pushing the no op in the ex-ma stage
    {
        ex_ma_mainPipeline = ex_ma_No_Op;
        PushNoOp = false;
        No_Stals++;
    }

    int Rs1DeEx = de_ex_mainPipeline.Rs1; // Rs1 of de-ex stage
    int Rs2DeEx = de_ex_mainPipeline.Rs2; // Rs2 of de-ex stage

    if ((ex_ma_mainPipeline.isBranch == 1) || (ex_ma_mainPipeline.isBranch == 2))
    {
        ControlHazard = true;
        No_Control_Hazard++;
        No_stals_due_to_ctrlHazard++;
    }

    if (!ControlHazard) // checking only if there is no ctrl hazard
    {
        if (Rs1DeEx > 0) // if Rs1 is valid
        {
            if (Rs1DeEx == ex_ma_mainPipeline.Rd) // hazard after ex(due to Rs1), we can forward data if the instruction is not lw/lh/lb
            {
                // if (ex_ma_mainPipeline.mem_OP > 0) // a memory operation, there has to be a stall
                if (ex_ma_mainPipeline.Result_select==2) // a memory operation, there has to be a stall
                {
                    DataHazard = true;
                    RefreshOprands = true;
                    No_Data_Hazard++;
                    No_stals_due_to_dataHazard++;
                }
                else // it is a non mem operation and data is present in the pipe line, forwarding is possible
                {
                    int NewValue;
                    if(ex_ma_mainPipeline.Result_select==0)//pc+4
                    NewValue = ex_ma_mainPipeline.CurrentPCAdd.to_ulong()+4;
                    else if(ex_ma_mainPipeline.Result_select==1)//immu
                    NewValue = ex_ma_mainPipeline.immU;
                    else if(ex_ma_mainPipeline.Result_select==3)//aluresult
                    NewValue = ex_ma_mainPipeline.ALU_result;
                    // refreshing the new Vlaue of the RS1 And putting it in the pipeline
                    switch (de_ex_mainPipeline.InstType)
                    {
                    case 'R':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;
                    case 'I':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                        // if(de_ex_mainPipeline.ALU_Operation == 15)//jalr
                        // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.Op1 + de_ex_mainPipeline.imm;
                    }
                    break;
                    case 'S':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;
                    case 'B':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;

                    default:
                    {
                        cout << "Some error occured While handeling Hazards, Exiting...";
                        exit(0);
                    }
                    break;
                    }
                }
            }
            else if (Rs1DeEx == ma_wb_mainPipeline.Rd) // hazard after ma(due to Rs1), we can forward data in any case
            {
                if(ma_wb_mainPipeline.Result_select==2)//loaded mem
                {
                    int NewValue = ma_wb_mainPipeline.loaded_mem;
                    // refreshing the new Vlaue of the RS1 And putting it in the pipeline
                    switch (de_ex_mainPipeline.InstType)
                    {
                    case 'R':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;
                    case 'I':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                        // if(de_ex_mainPipeline.ALU_Operation == 15)//jalr
                        // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.Op1 + de_ex_mainPipeline.imm;
                    }
                    break;
                    case 'S':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;
                    case 'B':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;

                    default:
                    {
                        cout << "Some error occured While handeling Hazards, Exiting...";
                        exit(0);
                    }
                    break;
                    }
                }
                else if(ma_wb_mainPipeline.Result_select==3)//aluresult
                {
                     int NewValue = ma_wb_mainPipeline.ALU_result;
                    // refreshing the new Vlaue of the RS1 And putting it in the pipeline
                    switch (de_ex_mainPipeline.InstType)
                    {
                    case 'R':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;
                    case 'I':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                        // if(de_ex_mainPipeline.ALU_Operation == 15)//jalr
                        // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.Op1 + de_ex_mainPipeline.imm;
                    }
                    break;
                    case 'S':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;
                    case 'B':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;

                    default:
                    {
                        cout << "Some error occured While handeling Hazards, Exiting...";
                        exit(0);
                    }
                    break;
                    }

                }
                else if(ma_wb_mainPipeline.Result_select==1)//immu
                {
                     int NewValue = ma_wb_mainPipeline.immU;
                    // refreshing the new Vlaue of the RS1 And putting it in the pipeline
                    switch (de_ex_mainPipeline.InstType)
                    {
                    case 'R':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;
                    case 'I':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                        // if(de_ex_mainPipeline.ALU_Operation == 15)//jalr
                        // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.Op1 + de_ex_mainPipeline.imm;
                    }
                    break;
                    case 'S':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;
                    case 'B':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;

                    default:
                    {
                        cout << "Some error occured While handeling Hazards, Exiting...";
                        exit(0);
                    }
                    break;
                    }

                }
                else if(ma_wb_mainPipeline.Result_select==0)//pc+4
                {
                    int NewValue = ma_wb_mainPipeline.CurrentPCAdd.to_ulong() + 4;
                    // refreshing the new Vlaue of the RS1 And putting it in the pipeline
                    switch (de_ex_mainPipeline.InstType)
                    {
                    case 'R':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;
                    case 'I':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                        // if(de_ex_mainPipeline.ALU_Operation == 15)//jalr
                        // ex_ma_mainPipeline.nextPCAdd = de_ex_mainPipeline.Op1 + de_ex_mainPipeline.imm;
                    }
                    break;
                    case 'S':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;
                    case 'B':
                    {
                        de_ex_mainPipeline.Op1 = NewValue;
                    }
                    break;

                    default:
                    {
                        cout << "Some error occured While handeling Hazards, Exiting...";
                        exit(0);
                    }
                    break;
                    }

                }
                else{
                    cout<<"error while solving hazards";
                }
            }
        }

        if (Rs2DeEx > 0) // if Rs2 is valid
        {
            if (Rs2DeEx == ex_ma_mainPipeline.Rd) // hazard after ex(Due to Rs2), we can forward data if the instruction is not lw/lh/lb
            {
                if (ex_ma_mainPipeline.mem_OP > 0) // a memory operation, there has to be a stall
                {
                    DataHazard = true;
                    RefreshOprands = true;
                    No_Data_Hazard++;
                    No_stals_due_to_dataHazard++;
                }
                else // it is a non mem operation and data is present in the pipe line, forwarding is possible
                {
                    int NewValue;
                    if(ex_ma_mainPipeline.Result_select==0)//pc+4
                    NewValue = ex_ma_mainPipeline.CurrentPCAdd.to_ulong()+4;
                    else if(ex_ma_mainPipeline.Result_select==1)//immu
                    NewValue = ex_ma_mainPipeline.immU;
                    else if(ex_ma_mainPipeline.Result_select==3)//aluresult
                    NewValue = ex_ma_mainPipeline.ALU_result;
                    else
                    {
                    cout<<"Error handeling hazards EXITING...";
                        exit(0);
                    }
                    // refresh the new data here and  put it in the pipeline
                    switch (de_ex_mainPipeline.InstType)
                    {
                    case 'R':
                    {
                        de_ex_mainPipeline.Op2 = NewValue;
                    }
                    break;
                    case 'S':
                    {
                        de_ex_mainPipeline.Mem_Op2 = NewValue;
                    }
                    break;
                    case 'B':
                    {
                        de_ex_mainPipeline.Op2 = NewValue;
                    }
                    break;

                    default:
                    {
                        cout << "Some error occured While handeling Hazards, Exiting...";
                        exit(0);
                    }
                    break;
                    }
                }
            }
            else if (Rs2DeEx == ma_wb_mainPipeline.Rd) // hazard after ma(due to Rs2), we can forward data in any case
            {
                int NewValue;
                    if(ma_wb_mainPipeline.Result_select==0)//pc+4
                    NewValue = ma_wb_mainPipeline.CurrentPCAdd.to_ulong()+4;
                    else if(ma_wb_mainPipeline.Result_select==1)//immu
                    NewValue = ma_wb_mainPipeline.immU;
                    else if(ma_wb_mainPipeline.Result_select==2)
                    NewValue = ma_wb_mainPipeline.loaded_mem;
                    else if(ma_wb_mainPipeline.Result_select==3)//aluresult
                    NewValue = ma_wb_mainPipeline.ALU_result;
                    else
                    {
                    cout<<"Error handeling hazards EXITING...";
                        exit(0);
                    }
                
                // refresh the new data here and  put it in the pipeline
                switch (de_ex_mainPipeline.InstType)
                {
                case 'R':
                {
                    de_ex_mainPipeline.Op2 = NewValue;
                }
                break;
                case 'S':
                {
                    de_ex_mainPipeline.Mem_Op2 = NewValue;
                }
                break;
                case 'B':
                {
                    de_ex_mainPipeline.Op2 = NewValue;
                }
                break;

                default:
                {
                    cout << "Some error occured While handeling Hazards, Exiting...";
                    exit(0);
                }
                break;
                }
            }
        }
    }
    // refreshOprands(RefreshOprands,DataHazard,Rs1DeEx,Rs2DeEx);//refreshes the oprands
    // cout<<"Hello";
    cout << endl
         << "This is the value of next pc: " << ex_ma_mainPipeline.nextPCAdd.to_ulong() << endl;
    cout << endl
         << "This is the value of curr pc: " << ex_ma_mainPipeline.CurrentPCAdd.to_ulong() << endl;

    if (DataHazard && (!ControlHazard)) // Only dta hazard
    {
        HaltDE = true;      // halting DE for the next cycle
        HaltIF = true;      // halting IF for the next cycle
        PushNoOp = true;    // pushing no op to ex-ma stage in the next cycle
        DataHazard = false; // resetting the flag
    }
    else if (ControlHazard && (!DataHazard)) // Only Control hazard
    {
        currentInstruction = 51; // noOp
        de_ex_mainPipeline = de_ex_No_Op;
        currentPCAdd = ex_ma_mainPipeline.nextPCAdd; // next pc taken from the ex stage
        ControlHazard = false;                       // resetting the flag
    }
    else if (DataHazard && ControlHazard) // Both at the same time
    {
        // case is not possible
        cout << "Unexpected Error Occured";
    }
    else
    {
        cout << "NO data and Control hazard";
    }
}


void init_NoOps()
{
    {                                         // here is the NoOp(add x0 x0 x0) instructiion for de-ex stage pipeline
        de_ex_No_Op.Op1 = 0;                  // op1 is always 0
        de_ex_No_Op.Op2 = 0;                  // op2 is always 0
        de_ex_No_Op.Rd = 0;                   // return destonation is x0
        de_ex_No_Op.imm = 0;                  // don't care
        de_ex_No_Op.immU = 0;                 // don't care
        de_ex_No_Op.immS = 0;                 // don't care
        de_ex_No_Op.immJ = 0;                 // don't care
        de_ex_No_Op.immB = 0;                 // don't care
        de_ex_No_Op.branch_target_select = 0; // don't care
        de_ex_No_Op.Result_select = 3;        // Alu result
        de_ex_No_Op.ALU_Operation = 0;        // add
        de_ex_No_Op.mem_OP = 0;               // no operation
        de_ex_No_Op.RFWrite = 1;              // write operation
        de_ex_No_Op.Store_load_op = 0;        // don't care
        de_ex_No_Op.Mem_Op2 = 0;              // don't care
        de_ex_No_Op.nextPCAdd = -1;           // flag value
        de_ex_No_Op.CurrentPCAdd = -1;
        de_ex_No_Op.InstType = 'R'; // R for add
    }

    {                               // here is the NoOp(add x0 x0 x0) instructiion for ex_ma stage pipeline
        ex_ma_No_Op.isBranch = 0;   // will tell whether to branch or not
        ex_ma_No_Op.ALU_result = 0; // will store the result of the alu

        ex_ma_No_Op.Rd = 0;                   // RF write destinstion
        ex_ma_No_Op.immU = 0;                 // don't care
        ex_ma_No_Op.branch_target_select = 0; // don't care
        ex_ma_No_Op.Result_select = 3;        // 3: ALU result
        ex_ma_No_Op.mem_OP = 0;               // 0:No operation
        ex_ma_No_Op.RFWrite = 1;              // 1:for write operation
        ex_ma_No_Op.Store_load_op = 0;        // dont care
        ex_ma_No_Op.Mem_Op2 = 0;              // dont care
        ex_ma_No_Op.nextPCAdd = -1;           // flag value
        ex_ma_No_Op.CurrentPCAdd = -1;
    }

    {                               // here is the NoOp(add x0 x0 x0) instructiion for ma_wb stage pipeline
        ma_wb_No_Op.loaded_mem = 0; // dont care

        ma_wb_No_Op.Rd = 0;   // RF write destinstion
        ma_wb_No_Op.immU = 0; // dont care
        // int branch_target_select=0; // 0 for immB; 1 for immJ
        ma_wb_No_Op.Result_select = 3; // 3: ALU result
        ma_wb_No_Op.RFWrite = 1;       // 1:for write operation
        ma_wb_No_Op.ALU_result = 0;    // will store the result of the alu
        ma_wb_No_Op.isBranch = 0;      // will tell whether to branch or not
        ma_wb_No_Op.nextPCAdd = -1;    // flag value
        ma_wb_No_Op.CurrentPCAdd = -1;
    }

    currentPCAdd_Pipe = -1;
    nextPCAdd_Pipe = -1;

    currentInstruction = 51; // for pipe line of if-de
    currentPCAdd_Pipe = -1;

    de_ex_mainPipeline = de_ex_No_Op;
    ex_ma_mainPipeline = ex_ma_No_Op;
    ma_wb_mainPipeline = ma_wb_No_Op;
}

#pragma region MainMemory

class MainMemory
{
    private:

    static void Write(int address, char *value,int bytesToWrite)
    {
        for (int i = 0; i < bytesToWrite; i++)
        {
            memory_arr[address + i] = value[i];
        }
        return;
    }

    static void Read(int address,char *Output)
    {
        for (int i = 0; i < blockSize; i++)
        {
            Output[i] = memory_arr[address + i];
        }
        return;
    }

    public:
    static void write(int address, char *values,int bytesToWrite)
    {
        // int EffectiveAddress;//will store the address from where the block starts
        // EffectiveAddress = address - address % blockSize;//removing last log2(Blocksize) bits
        Write(address, values, bytesToWrite);
        return;
    }

    static BlockParameters read(int address,int tag)//this will return a block filled with data
    {
        int EffectiveAddress;//will store the address from where the block starts
        char Output[blockSize];
        struct BlockParameters block;

        EffectiveAddress = address - address % blockSize;//removing last log2(Blocksize) bits
        Read(EffectiveAddress, Output);
        block.data = Output;
        block.tag = tag;
        block.validBit=1;
        block.frequency=0;

        return block;
    }
};

#pragma endregion MainMemory



#pragma region CACHE
enum POLICY {LRU, FIFO, LFU};
enum MAPPING {DIRECT, FULLY_ASSOS, SET_ASSOSC};
int cacheSize = 16; /*BYTES*/  
// int blockSize = 4; /*BYTES*/
//int miss;
int FIFOindex = 0;;
//1 = fifo
//0 = lru
int policy = LRU; 
//0 = direct mapping
//1 = fully assosc
int mapping = DIRECT;
int waysOfSetAssosc = 2;

class Cache 
{
    private:
        list<struct BlockParameters> cache;
        int currentSize;
        int blockSize;
        int cacheSize;
        //LRU(0)/FIFO(1)/Random(2)/LFU(3)
        int policy;/*REPLACEMENT POLICY*/
        //number of ways of assosc
        int setAssosciativity;
        /*Direct mapped (0)
        Full Assoc (1)
        Set Assoc (2),*/
        int mapping;
        unordered_map<int , bool> misstable;


        void WriteCache_FA(int blockSize, int blockOffset, int bytesToRW, list<struct BlockParameters>::iterator &it, char *value, int index, int key,int address)
        {
            {
            // if((blockSize - blockOffset) >= bytesToRW)
            // {
            //     // cout<<"Block size "<<blockOffset;
            //     // cout<<"Where";
            //     char* substr = new char[bytesToRW];
            //     memcpy(&it->data[blockOffset], value, bytesToRW);
            // }
            // else if((blockSize - blockOffset) < bytesToRW)
            // {
            //     // char* substr1 = new char[blockSize-blockOffset];
            //     // char* substr2 = new char[bytesToRW - (blockSize-blockOffset)];
            //     char* finalStr = new char[bytesToRW];
            //     cout<<"block size is "<<blockSize - blockOffset<<endl;
            //     // cout<<value[0]<<value[1]<<value[2];
            //     memcpy(&(it->data[blockOffset]), value, blockSize - blockOffset);
            //     if(index + 1 >= (cacheSize/blockSize))
            //     {
            //         //write in next block
            //         char *ptr = FullyAssosciative(key, value + (blockSize-blockOffset), 0, 0, 1,bytesToRW - (blockSize-blockOffset));
            //         // memcpy(&it->data[blockOffset], value + ((blockSize - blockOffset)), bytesToRW - (blockSize - blockOffset));
            //         // strncpy(finalStr, ptr,bytesToRW - (blockSize-blockOffset));
            //     }
            //     else if(index + 1 < (cacheSize/blockSize))
            //     {
            //         char *ptr = FullyAssosciative(key, value + (blockSize-blockOffset), 0, index + 1, 1,bytesToRW - (blockSize-blockOffset));
            //         // strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
            //         // memcpy(&it->data[blockOffset], value + ((blockSize - blockOffset)),bytesToRW - (blockSize - blockOffset));
            //     }
            //     // cout<<"here is"<<*finalStr;
            // }


            // bool isHit=false;
            // for(int i = 0; i < setAssosciativity; i++)
            // {
            //     if(key==it->tag)
            //     {
            //         isHit=true;
            //         break;
            //     }
            // }
}
            MainMemory::write(address,value,bytesToRW);
            // if (isHit)
            // {
                if((blockSize - blockOffset) >= bytesToRW)
                {
                    // cout<<"Block size "<<blockOffset;
                    // cout<<"Where";
                    char* substr = new char[bytesToRW];
                    memcpy(&it->data[blockOffset], value, bytesToRW);
                }
                else if((blockSize - blockOffset) < bytesToRW)
                {
                    // char* substr1 = new char[blockSize-blockOffset];
                    // char* substr2 = new char[bytesToRW - (blockSize-blockOffset)];
                    char* finalStr = new char[bytesToRW];
                    cout<<"block size is "<<blockSize - blockOffset<<endl;
                    // cout<<value[0]<<value[1]<<value[2];
                    memcpy(&(it->data[blockOffset]), value, blockSize - blockOffset);
                    //write remaining bits to the memory

                    // address+=blockSize-blockOffset;
                    it = cache.begin();//writing the remaining bits if the address is present in the cache
                    int newAdd = address + (blockSize-blockOffset);
                    for ( ; it!=cache.end(); it++)
                    {
                        if(it->tag == newAdd)
                        {
                            int numberOfBlocks = (cacheSize/blockSize);
                            int blockOffsetBits = log2(blockSize); //2
                            int indexBits = log2(numberOfBlocks);  //2
                            int mask = (1<<indexBits) - 1;
                            int tagAddress = address>>blockOffsetBits>>indexBits;
                            // cout<<"int address"<<(intAddress>>blockOffsetBits);
                            int index = (mask&(newAdd>>blockOffsetBits));
                            mask = (1<<blockOffsetBits) - 1;
                            int blockOffset = (mask&newAdd);
                            WriteCache_FA(blockSize, blockOffset, bytesToRW - (blockSize-blockOffset), it, value + (blockSize-blockOffset), 0, tagAddress, newAdd);
                            break;
                        }
                    }
                }                
            // }
            // else//wrtite in memory
            // {
            //     MainMemory.write(address, value, bytesToRW);
            // }
            

        }
        char* DirectMap(int key, char *value, int index, int blockOffset, int RW, int bytesToRW, int fullAddress)
        {
            // cout<<"bloo"<<blockOffset;
            switch (RW)
            {
                case 0:
                {
                    auto it = cache.begin();
                    advance(it, index);
                    // while(1)
                        // cout<<"We read";
                    if(it->tag == key)
                    {
                        cout<<"Data found";
                        hits++;
                        // return *it;
                        if((blockSize - blockOffset) >= bytesToRW)
                        {
                            char* substr = new char[bytesToRW];
                            strncpy(substr, &it->data[blockOffset], bytesToRW);
                            return substr;
                        }
                        else if((blockSize - blockOffset) < bytesToRW)
                        {
                            char* finalStr = new char[bytesToRW];
                            strncpy(finalStr, &it->data[blockOffset],(blockSize-blockOffset));

                            if(index + 1 >= (cacheSize/blockSize))
                            {
                                char *ptr = DirectMap(key + 1, &nullData, 0, 0, 0,bytesToRW - (blockSize-blockOffset), fullAddress);
                                strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
                            }
                            else if(index + 1 < (cacheSize/blockSize))
                            {
                                char *ptr = DirectMap(key, &nullData, index + 1, 0, 0,bytesToRW - (blockSize-blockOffset), fullAddress);
                                strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
                            }
                            return finalStr;
                        }
                        
                    } 
                    else
                    {
                        cout<<"Get from main memory";
                        misses++;
                        it->data = MainMemory::read(fullAddress, key).data;

                        // return MainMemory.w
                    }
                    return &nullData;
                }
                break;
                case 1:
                {
                    BlockParameters block;
                    block.tag = key;
                    block.data = value;
                    block.validBit = 1;
                    block.recencyInfo = 0;
                    auto it = cache.begin();
                    advance(it, index);
                    if(it->tag == key)
                    {
                        if((blockSize - blockOffset) >= bytesToRW)
                        {
                            // cout<<"Block size "<<blockOffset;
                            // cout<<"Where";
                            char* substr = new char[bytesToRW];
                            memcpy(&it->data[blockOffset], value, bytesToRW);
                        }
                        else if((blockSize - blockOffset) < bytesToRW)
                        {
                            // char* substr1 = new char[blockSize-blockOffset];
                            // char* substr2 = new char[bytesToRW - (blockSize-blockOffset)];
                            char* finalStr = new char[bytesToRW];
                            cout<<"block siz is "<<blockSize - blockOffset<<endl;
                            // cout<<value[0]<<value[1]<<value[2];
                            memcpy(&(it->data[blockOffset]), value, blockSize - blockOffset);
                            if(index + 1 >= (cacheSize/blockSize))
                            {
                                //write in next block
                                char *ptr = DirectMap(key + 1, value + (blockSize-blockOffset), 0, 0, 1,bytesToRW - (blockSize-blockOffset), fullAddress);
                                // memcpy(&it->data[blockOffset], value + ((blockSize - blockOffset)), bytesToRW - (blockSize - blockOffset));
                                // strncpy(finalStr, ptr,bytesToRW - (blockSize-blockOffset));
                            }
                            else if(index + 1 < (cacheSize/blockSize))
                            {
                                char *ptr = DirectMap(key, value + (blockSize-blockOffset), index + 1, 0, 1,bytesToRW - (blockSize-blockOffset), fullAddress);
                                // strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
                                // memcpy(&it->data[blockOffset], value + ((blockSize - blockOffset)),bytesToRW - (blockSize - blockOffset));
                            }
                            // cout<<"here is"<<*finalStr;
                        }
                        it->tag = key;
                        it->validBit = 1;
                        it->recencyInfo = blockSize -1;

                    }
                    else
                    {
                        MainMemory::write(fullAddress, value, bytesToRW);
                    }
                    return &nullData;
                }
            }

        }
        void LFU_Evict(int fullAddress, int tag, list<BlockParameters>::iterator startOfSet, list<BlockParameters>::iterator endOfSet)
        {
            auto minIter = startOfSet; 
            int min = minIter->frequency;
            for(auto it = startOfSet; it != endOfSet; it++)
            {
                if(it->frequency < min)
                {
                    min = it->frequency;
                    minIter = it;
                }
            }
            
            minIter->data  = MainMemory::read(fullAddress, tag).data;
        }
        void LRU_Evict(int fullAddress, int tag, list<BlockParameters>::iterator startOfSet, list<BlockParameters>::iterator endOfSet)
        {
            auto minIter = startOfSet; 
            cache.splice(endOfSet, cache, minIter);
            minIter->data  = MainMemory::read(fullAddress, tag).data;
        }
        char* FullyAssosciative(int key, char *value, int blockOffset, int index, int RW, int bytesToRW, int fullAddress)
        {
            bool validBitPresent = false;
            switch(RW)
            {
                case 0:
                {
                    for(auto it = cache.begin(); it != cache.end(); ++it)
                    {
                        //check for an empty block
                            // cout<<"we found "<<it->tag; 
                        if(it->tag == key)
                        {
                            //on accessing put the cache block at the end; 
                            //LRU
                            hits++;
                            switch(policy)
                            {
                                case 0:
                                {
                                    //LRU
                                    char *substr;
                                    if((blockSize - blockOffset) >= bytesToRW)
                                    {
                                        substr = new char[bytesToRW+1];
                                        strncpy(substr, &it->data[blockOffset], bytesToRW);
                                        cout<<"Here is "<<it->data[3];
                                        return substr;
                                    }
                                    else if((blockSize - blockOffset) < bytesToRW)
                                    {
                                        //data not in block
                                        char* finalStr = new char[bytesToRW];
                                        strncpy(finalStr, &it->data[blockOffset],(blockSize-blockOffset));

                                        if(index + 1 >= (cacheSize/blockSize))
                                        {
                                            char *ptr = FullyAssosciative(key + 1, &nullData, 0, 0, 0,bytesToRW - (blockSize-blockOffset), fullAddress);
                                            strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
                                        }
                                        else if(index + 1 < (cacheSize/blockSize))
                                        {
                                            char *ptr = FullyAssosciative(key, &nullData, index + 1, 0, 0,bytesToRW - (blockSize-blockOffset), fullAddress);
                                            strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
                                        }
                                    }
                                    // cache.splice(cache.end(), cache, it);
                                    return substr;
                                }
                                break;
                                case 1:
                                {
                                    //FIFO
                                    //NOTHING HAPPENS
                                }
                                break;
                                case 2:
                                {
                                    //LFU
                                    //increment frequency by 1
                                    char *substr;
                                    if((blockSize - blockOffset) >= bytesToRW)
                                    {
                                        it->frequency += 1;
                                        substr = new char[bytesToRW+1];
                                        strncpy(substr, &it->data[blockOffset], bytesToRW);
                                        return substr;
                                    }
                                    else if((blockSize - blockOffset) < bytesToRW)
                                    {
                                        //data not in block
                                        it->frequency += 1;
                                        char* finalStr = new char[bytesToRW];
                                        strncpy(finalStr, &it->data[blockOffset],(blockSize-blockOffset));

                                        if(index + 1 >= (cacheSize/blockSize))
                                        {
                                            char *ptr = FullyAssosciative(key + 1, &nullData, 0, 0, 0,bytesToRW - (blockSize-blockOffset), fullAddress);
                                            strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
                                        }
                                        else if(index + 1 < (cacheSize/blockSize))
                                        {
                                            char *ptr = FullyAssosciative(key, &nullData, index + 1, 0, 0,bytesToRW - (blockSize-blockOffset), fullAddress);
                                            strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
                                        }
                                        return finalStr;
                                    }
                                    return substr;
                                }
                            }
                        }
                        else{
                            //miss
                            //GO TO MAIN MEMORY
                            if(policy == LFU)
                            {   
                                LFU_Evict(fullAddress, key, cache.begin(), cache.end());
                                misstable[fullAddress] = false;
                            }
                            else 
                            {
                                
                                it->data = MainMemory::read(fullAddress, key).data;
                            }
                            // cache.begin = m1.read();
                            return &nullData;
                        }
                    }
                    return &nullData;
                }
                break;
                case 1:
                {
                    for(auto it = cache.begin(); it != cache.end(); ++it)
                    {
                        if(it->tag == key)
                        {
                            WriteCache_FA(blockSize, blockOffset, bytesToRW, it, value, index, key, fullAddress);;
                            return &nullData;
                        }
                    }
                    MainMemory::write(fullAddress, value, bytesToRW);
                            // for(list<struct BranchParameters>::iterator it :cache)
                    // auto it = cache.begin();
                    // {
                    //     //check for an empty block
                    //     // if(it->validBit == 0)
                    //     {
                    //         it->validBit = 1;
                    //         it->tag = key;
                    //         it->recencyInfo = blockSize - 1;
                    //         // it->FIFOindex = (FIFOindex++);
                    //         //on accessing put the cache block at the end; 
                    //         // cout<<"WHerheh h";
                    //         // cache.splice(cache.end(), cache, it);
                    //         // WriteCache(blockSize, blockOffset, bytesToRW, it, value, index, key);
                    //         validBitPresent = true;
                    //         // break;
                    //     }
                    // }
                    // //case where it is full
                    // // if(0)
                    // {
                    //     switch(policy)
                    //         {
                    //             case LRU:
                    //             {
                    //                 //LRU
                    //                 //we evict the first from the cache list which is the LRU
                    //                 auto it = cache.begin();
                    //                 // it->validBit = 1;
                    //                 // it->data = value;
                    //                 // it->tag = key;
                    //                 // it->recencyInfo = blockSize - 1;
                    //                 // cout<<"\n\n\n";
                    //                 cache.splice(cache.end(), cache, it);
                    //                 // WriteCache_FA(blockSize, blockOffset, bytesToRW, it, value, index, key, fullAddress);
                    //                 //on accessing put the cache block at the end; 
                    //             }
                    //             break;
                    //             case FIFO:
                    //             {
                    //                 //FIFO
                    //                 int min = (cache.begin())->FIFOindex;
                    //                 auto minIter = cache.begin(); 
                    //                 //find the first index
                    //                 // for(auto it:cache)
                    //                 for(auto it = cache.begin(); it != cache.end(); it++)
                    //                 {
                    //                     if(it->FIFOindex < min)
                    //                     {
                    //                         min = it->FIFOindex;
                    //                         minIter = it;
                    //                     }
                    //                 }
                    //                 minIter->FIFOindex += 1 ;
                    //                 WriteCache_FA(blockSize, blockOffset, bytesToRW, minIter, value, index, key, fullAddress);
                    //                 // minIter->validBit = 1;
                    //                 // minIter->tag = key;


                    //             }
                                // break;
                            //     case LFU:
                            //     {
                            //         //find with least frequency
                            //         int min = (cache.begin())->frequency;
                            //         auto minIter = cache.begin(); 
                            //         //find the first index
                            //         // for(auto it:cache)
                            //         for(auto it = cache.begin(); it != cache.end(); it++)
                            //         {
                            //             if(it->frequency < min)
                            //             {
                            //                 min = it->frequency;
                            //                 minIter = it;
                            //             }
                            //         }
                            //         // minIter->validBit = 1;
                            //         // WriteCache(blockSize, blockOffset, bytesToRW, minIter, value, index, key);
                            //         // minIter->tag = key;
                            //         // minIter->recencyInfo = blockSize - 1;
                            //         //LFU
                            //         //increment frequency by 1
                            //         minIter->frequency += 1;
                            //         WriteCache_FA(blockSize, blockOffset, bytesToRW, minIter, value, index, key, fullAddress);

                            //     }
                            // }
                        
                        
                

                    }
                }
        }
        char* SetAssosciative(int key, char *value, int index, int blockOffset, int RW, int bytesToRW, int fullAddress)
        {
            //multiplying so that we can go directly to the initial block of that set
            index = index*waysOfSetAssosc;
            auto startOfSet = cache.begin();
            advance(startOfSet, index);
            auto endOfSet = next(startOfSet, setAssosciativity - 1);
            // for(auto it = cache.begin(); it != cache.end(); )

            switch(RW)
            {
                case 0:
                {
                    bool dataPresent = false;
                    // for(int i =0; i <setAssosciativity; i++)
                    for(auto it = startOfSet; it != endOfSet; it++)
                    {
                        if(it->tag == key)
                        {
                            hits++;
                            dataPresent = true;
                            char *substr;
                            switch(policy)
                            {
                                case 0:
                                {
                                    //LRU
                                    char *substr;
                                    if((blockSize - blockOffset) >= bytesToRW)
                                    {
                                        substr = new char[bytesToRW+1];
                                        strncpy(substr, &it->data[blockOffset], bytesToRW);
                                        cout<<"Here is "<<it->data[3];
                                        return substr;
                                    }
                                    else if((blockSize - blockOffset) < bytesToRW)
                                    {
                                        //data not in block
                                        char* finalStr = new char[bytesToRW];
                                        strncpy(finalStr, &it->data[blockOffset],(blockSize-blockOffset));

                                        if(index + 1 >= (cacheSize/blockSize))
                                        {
                                            char *ptr = SetAssosciative(key + 1, &nullData, 0, 0, 0,bytesToRW - (blockSize-blockOffset), fullAddress);
                                            strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
                                        }
                                        else if(index + 1 < (cacheSize/blockSize))
                                        {
                                            char *ptr = SetAssosciative(key, &nullData, index + 1, 0, 0,bytesToRW - (blockSize-blockOffset), fullAddress);
                                            strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
                                        }
                                    }
                                    cache.splice(cache.end(), cache, it);
                                    return substr;
                                }
                                break;
                                case 1:
                                {
                                    //FIFO
                                    //NOTHING HAPPENS
                                }
                                break;
                                case 2:
                                {
                                    //LFU
                                    //increment frequency by 1
                                    char *substr;
                                    if((blockSize - blockOffset) >= bytesToRW)
                                    {
                                        substr = new char[bytesToRW+1];
                                        strncpy(substr, &it->data[blockOffset], bytesToRW);
                                        return substr;
                                    }
                                    else if((blockSize - blockOffset) < bytesToRW)
                                    {
                                        //data not in block
                                        char* finalStr = new char[bytesToRW];
                                        strncpy(finalStr, &it->data[blockOffset],(blockSize-blockOffset));

                                        if(index + 1 >= (cacheSize/blockSize))
                                        {
                                            char *ptr = FullyAssosciative(key + 1, &nullData, 0, 0, 0,bytesToRW - (blockSize-blockOffset), fullAddress);
                                            strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
                                        }
                                        else if(index + 1 < (cacheSize/blockSize))
                                        {
                                            char *ptr = FullyAssosciative(key, &nullData, index + 1, 0, 0,bytesToRW - (blockSize-blockOffset), fullAddress);
                                            strncpy(&finalStr[bytesToRW - (blockSize-blockOffset)], ptr,bytesToRW - (blockSize-blockOffset));
                                        }
                                        return finalStr;
                                    }
                                    it->frequency += 1;
                                    return substr;
                                }
                            }
                            return substr;
                        }
                    }
                    {
                        MainMemory::write(fullAddress, value, bytesToRW);
                        //go to main memory
                        switch (policy)
                        {
                            case LFU:
                                LFU_Evict(fullAddress, key, startOfSet, endOfSet);
                                misstable[fullAddress] = false;
                                break;
                            case LRU:
                                LRU_Evict(fullAddress,key, startOfSet, endOfSet);
                                misstable[fullAddress] = false;
                            default:
                                break;
                        }
                         // LFU(fullAddress, key);
                        // it->data = MainMemory::read(fullAddress, key);
                        misses++;
                    }
                }
                break;
                case 1:
                {
                        for(auto it = startOfSet; it != endOfSet; it++)
                        {
                            if(it->tag == key)
                            {
                                WriteCache_FA(blockSize, blockOffset, bytesToRW, it, value, index, key, fullAddress);
                                return &nullData;
                            }

                                //value written in the empty block
                                // setIsEmpty = i;
                                // it->tag = key;
                                // it->validBit = 1;
                                // it->recencyInfo = 0;
                                //move temp to the front of the set for LRU

                                // switch(policy)
                                // {
                                //     case 0:
                                //     {
                                //         //lru
                                //         startOfSet->validBit = 1;
                                //         // it->data = value;
                                //         startOfSet->tag = key;
                                //         startOfSet->recencyInfo = blockSize - 1;
                                //         //on accessing put the cache block at the end; 
                                //         // WriteCache_FA(blockSize, blockOffset);
                                //         WriteCache_FA(blockSize, blockOffset, bytesToRW, it, value, index, key, fullAddress);
                                //         cache.splice(endOfSet, cache, it);
                                //     }
                        //             break;
                        //             case 1:
                        //             {
                        //                 //fifo
                        //                 int min = (startOfSet)->FIFOindex;
                        //                 auto minIter = startOfSet; 
                        //                 //find the first index
                        //                 // for(auto it:cache)
                        //                 // for(auto iter = startOfSet; iter != endOfSet; iter++)
                        //                 {
                        //                     // if(iter->validBit == 0)
                        //                     {
                        //                         WriteCache_FA(blockSize, blockOffset, bytesToRW, iter, value, index, key, fullAddress);
                        //                         minIter->validBit = 1;
                        //                         minIter->tag = key;
                        //                         minIter->recencyInfo = blockSize - 1;
                        //                         return &nullData;
                        //                     }
                        //                 }
                        //                 //replace the first block in set assosc
                        //                 WriteCache_FA(blockSize, blockOffset, bytesToRW, startOfSet, value, index, key, fullAddress);

                        //                 // cache.splice(endOfSet, cache, it);
                        //             }
                        //             break;
                        //             case 2:
                        //             {
                        //                 //lfu
                        //                 int min = (startOfSet)->frequency;
                        //                 auto minIter = startOfSet; 
                        //                 //find the first index
                        //                 // for(auto it:cache)
                        //                 for(auto iter = startOfSet; iter != endOfSet; iter++)
                        //                 {
                        //                     if(iter->frequency < min)
                        //                     {
                        //                         min = iter->frequency;
                        //                         minIter = iter;
                        //                     }
                        //                 }
                        //                 WriteCache_FA(blockSize, blockOffset, bytesToRW, minIter, value, index, key, fullAddress);
                        //                 minIter->validBit = 1;
                        //                 minIter->frequency += 1;
                        //                 minIter->tag = key;
                        //                 minIter->recencyInfo = blockSize - 1;
                        //             }
                        //             break;
                        //         }
                        //         return &nullData;
                        //     }
                        //     temp = next(temp);
                        // }

                        // //checks if a set is empty
                        // int setIsEmpty = -1;
                        // auto temp = startOfSet;
                        // //first place of the set
                        // auto first = startOfSet;
                        // //find an empty block there
                        // auto endOfSet = next(first, setAssosciativity - 1);
                        //if setIsEmpty != -1 means we have an empty block
                
                        //no empty block found now we evict the first block in the set
                        // if(setIsEmpty == -1)
                        }
                        MainMemory::write(fullAddress, value, bytesToRW);

                }
            }

        }

    public:
        //cache size = cap
        void InitialiseCache(int _capacity, 
        int _blockSize,
        int _policy, 
        int _mapping,
        int _SAassosc)

        {
            cacheSize = _capacity ; blockSize = (_blockSize); policy = (_policy);  mapping = (_mapping); setAssosciativity = (_SAassosc);
            for (int i = 0; i < (_capacity)/(_blockSize); i++) 
            {
                // pair<int, int> block;
                BlockParameters block;
                block.validBit = 0;
                block.recencyInfo = 0;
                block.data = new char[blockSize];
                for(int i = 0; i < blockSize; i++)
                {
                    block.data[i] = '~';
                }
                block.tag = NULL;
                block.FIFOindex = -1;
                // cache.push_back(block);
                currentSize = 0;
                // cout<<"We push"<<_capacity;
                cache.push_back(block);
            }
        };

        char* get(int key, int index, int blockOffset, int fullAddress, int bytesToRW) 
        {
            // if (block.find(key) != block.end()) {
            //     return block[key];
            // }
            char na = '~';
            switch(mapping)
            {
                misstable[fullAddress] = true;
                case 0:
                {
                    // cout<<"BOOOO"<<blockOffset;
                    return DirectMap(key, &na, index, blockOffset, 0, bytesToRW, fullAddress);
                }
                break;
                case 1:
                {
                    return FullyAssosciative(key, &na, blockOffset, index,0, bytesToRW, fullAddress);
                }
                break;
                case 2:
                {
                    return SetAssosciative(key, &na, index, blockOffset, 0, bytesToRW, fullAddress);
                }
            }
            // miss++;
            // return list<int>();
        }
        void put(int key, char *value, int index, int blockOffset, int fullAddress, int bytesToRW)
        {
            // if(0)
            //choose a mapping
            // cout<<"BOO"<<blockOffset;
            switch(mapping)
            {
                misstable[fullAddress] = true;
                case 0:
                {
                    // cout<<"block off is"<<blockOffset;
                    DirectMap(key, value, index, blockOffset, 1, bytesToRW, fullAddress);
                }
                break;
                case 1:
                {
                    FullyAssosciative(key, value, blockOffset, index, 1, bytesToRW, fullAddress);
                }
                break;
                case 2:
                {
                    SetAssosciative(key, value, index, blockOffset, 1, bytesToRW, fullAddress);
                }
            }
        }
        void show_cache()
        {
            cout<<"TARGET ADDRESS                DATA STORED";
                cout<<endl;
            int i = 0;
            for(auto &it :cache)
            // for (auto it = cache.begin(); it != cache.end(); ++it) 
            {
                // cout<<it.v<<" "<<it.second;
                cout<<i++<<") "<<"Data: "<<it.data<<" Tag: "<<it.tag<<" Recency: "<<it.recencyInfo<<" Valid bit: "<<it.validBit;
                // for(auto &obj : it.second)
                // for(auto obj = (it->second).begin(); obj != (it->second).end(); ++obj)
                cout<<endl;
            }
        }
        
        int size() 
        {
            return cache.size();
        }

        void typeofmiss(int fulladdress, int key, int index){
            if(misstable.find(fulladdress)==misstable.end()){
                coldmisses++; 
            }else{
                if(misstable[fulladdress]==false){
                    conflictmisses++;
                }
            }
            capacitymisses = misses - coldmisses - conflictmisses;
        }
};
Cache cache;

class Fetch
{
    bitset<32> fetch_instruction(int flag)
    {
        // the part b4 space PC address
        string hex_str;

        if (flag == 0)
        {
            string pc_str = read();
            currentPCAdd = HexStringToBitset(pc_str);
            hex_str = read();
            // addToBitset();//currentPCAdd+=4//it has to be next address
            incrementNextPCAdd();
        }
        else if (flag == 1)
        {
            // ifstream tempRead("input.mc");
            // string tempHexa;
            // tempRead >> tempHexa;
            // bitset<32> tempBitset(HexStringToBitset(tempHexa));

            // while (tempBitset != currentPCAdd)
            // {
            //     tempRead >> tempHexa;
            //     tempRead >> tempHexa;
            //     tempBitset = HexStringToBitset(tempHexa);
            // }

            // tempRead >> hex_str;
        }
        int offsetPC = currentPCAdd.to_ulong();
        int *num = (int *)(InstMem + offsetPC);
        bitset<32> currentInstruction(*num);

        // bitset<32> currentInstruction = HexStringToBitset(hex_str);
        // cout<<endl<<"READING INSTRUCTION "<<hex_str<<endl;
        bitset<32> returnPredictedAddress = predicted_address(currentInstruction);
        bitset<32> minusOne(-1);
        if (returnPredictedAddress != minusOne)
        {
            currentPCAdd = returnPredictedAddress;
            return currentInstruction;
        }

        if (currentInstruction == exitInstruction)
        {
            cout << endl
                 << "EXITING...\n";

            ofstream memFile; // storing the memmory array in a txt file.
            memFile.open("Memory_Dump.txt");
            memFile<<"            DATA SEGMENT"<<endl<<"--------------------------------------"<<endl;
            memFile<<"Address         "<<"+0    +1    +2    +3"<<endl<<endl;

            for (int i = 0; i < MEMORY_SIZE; i++)
            {
                unsigned int temp = (unsigned int)(*(memory_arr + i));
                if (temp > INT32_MAX)
                {
                    temp = temp - 0xffffff00;
                }
                if(i%4==0)
                memFile <<"0x"<< setfill('0') << setw(8) << hex<<(i+INSTMEM_SIZE)<<"      ";
                memFile << setfill('0') << setw(2) << hex << temp << (((i + 1) % 4 == 0) ? "\n" : "    ");
            }
            memFile.close();

            ofstream InstFile; // storing the memmory array in a txt file.
            InstFile.open("Instruction.txt");
            InstFile<<"            TEXT SEGMENT"<<endl<<"--------------------------------------"<<endl;
            InstFile<<"Address         "<<"+0    +1    +2    +3"<<endl<<endl;                  


            for (int i = 0; i < INSTMEM_SIZE; i++)
            {
                unsigned int temp = (unsigned int)(*(InstMem + i));
                if (temp > INT32_MAX)
                {
                    temp = temp - 0xffffff00;
                }
            
                if(i%4==0)
                InstFile <<"0x"<< setfill('0') << setw(8) << (i)<<"      ";
                InstFile << setfill('0') << setw(2) << hex << temp << (((i + 1) % 4 == 0) ? "\n" : "    ");
                // InstFile << i << ": " << hex << temp << (((i + 1) % 4 == 0) ? "\n" : "  |");
            }
            InstFile.close();

            printRF();

            btb_printer(BTB,BUFFER_SIZE);
            cout << endl
                 << endl;
            cout << "Total number of cycles: " << Clock << endl;
            cout << "Total instructions executed: " << (Clock - No_Stals) << endl;
            cout << "CPI: " << (((float)Clock) / (float)(Clock - No_Stals)) << endl;
            cout << "Number of Data-transfer (load and store) instructions executed: " << No_Lw_Sw << endl;
            cout << "Number of ALU instructions executed: " << No_ALU_inst - 1 << endl;
            cout << "Number of Control instructions executed: " << No_Ctrl_Inst << endl;
            cout << "Number of stalls/bubbles in the pipeline: " << No_Stals << endl;
            cout << "Number of data hazards: " << No_Data_Hazard << endl;
            cout << "Number of control hazards: " << No_Control_Hazard << endl;
            cout << "Number of stalls due to data hazards: " << No_stals_due_to_dataHazard << endl;
            cout << "Number of stalls due to control hazards: " << No_stals_due_to_ctrlHazard << endl;
            cout << "Number of branch miss prediction: " << No_Branch_miss << endl;

            cout<<endl <<"CacheParameters are : "<<endl<<endl;

            cout<<"No. of misses:"<<misses<<endl;
            cout<<"No. of coldmisses:"<<coldmisses<<endl;
            cout<<"No. of conflictmisses:"<<conflictmisses<<endl;
            cout<<"No. of capacitymisses:"<<capacitymisses<<endl;


            
            //COUT CACHE
            cout<<"\nCACHE IS\n";
            cache.show_cache();
            exit(0);
        }
        // stoul converts string of type 0x012312 to its decimal value

        return currentInstruction;
    }
    string read()
    {
        string line;
        readFile >> line;
        return line;
    }
    void addToBitset()
    {
        currentPCAdd = currentPCAdd.to_ulong() + 4; // add 4 to the bitset
    }
    void incrementNextPCAdd()
    {
        nextPCAdd = currentPCAdd.to_ulong() + 4; // add 4 to the bitset
    }

    bitset<32> HexStringToBitset(string hex_instr)
    {
        // cout<<"use of stoul for str: "<<hex_instr<<endl;
        unsigned long hex_to_dec_val = stoul(hex_instr, nullptr, 16);
        bitset<32> binary_num(hex_to_dec_val);
        return binary_num;
    }

public:
    Fetch(int flag = 0)
    {
        if (!HaltIF)
        {
            cout << "\n### Fetch ###\n\n";
            currentInstruction = fetch_instruction(flag);
            currentPCAdd_Pipe = currentPCAdd;
            // nextPCAdd_Pipe = nextPCAdd;
            nextPCAdd_Pipe = currentPCAdd.to_ulong() + 4;

            cout << "FETCH:Fetch instruction " << currentInstruction << " From address " << currentPCAdd << endl;
            if (ispipeLine)
            {
                bitset<32> preAdd = predicted_address(currentPCAdd);
                if(((int)(preAdd.to_ulong()))==-1)
                currentPCAdd = currentPCAdd.to_ulong() + 4;
                else 
                currentPCAdd = preAdd;
            }
            cout << "\n### End Fetch ###\n\n";
        }
        else
            HaltIF = false; // resetting the flag so that next time this will execute asusual
    }

};
// makes .mc file

char* Placeholder_Name(char *data, int intAddress, int readWrite, int bytesToRW)
{
    bitset<32> address(intAddress);
    int numberOfBlocks = (cacheSize/blockSize);
    int blockOffsetBits = log2(blockSize); //2
    int indexBits = log2(numberOfBlocks);  //2
    int mask = (1<<indexBits) - 1;
    bitset<32> tagAddress = address>>blockOffsetBits>>indexBits;
    // cout<<"int address"<<(intAddress>>blockOffsetBits);
    int index = (mask&(intAddress>>blockOffsetBits));
    mask = (1<<blockOffsetBits) - 1;
    int blockOffset = (mask&intAddress);
    // cout<<"BO"<<blockOffset;

    // cout<<"Tag address "<<tagAddress<<endl;
    // cout<<"Block offset bits "<<blockOffsetBits<<endl;
    // printf("number of blocks: %d \n", numberOfBlocks);
    // printf("block off-set bits: %d \n", blockOffset);
    // printf("index: %d \n", index);
    
    switch (mapping)
    {
        //direct mapping
        case 0:
        {
            index = index % numberOfBlocks;
        }
            break;
        //fully ass
        case 1:
        {
            index = index;
        }
        break;
        //set ass
        case 2:
        {
            int totalSets = (numberOfBlocks/waysOfSetAssosc);
            index = index % totalSets;
        }
        break;
        
        default:
            break;
    }

    int targetAddressInt = tagAddress.to_ulong();
    switch (readWrite)
    {
        //read
        case 0:
            return cache.get(targetAddressInt, index, blockOffset, intAddress, bytesToRW);
            break;
        //write
        case 1:
            cache.put(targetAddressInt, data, index, blockOffset, intAddress, bytesToRW);
            return &nullData;
            break;
        default:
            break;
    }

        
}

#pragma endregion CACHE


void RISCv_Processor()
{
    // ispipeLine = true;

    RF[2] = MEMORY_SIZE - 0xc;

    if (ispipeLine)
    {
        if (!DataForwarding)
        {
            while (1)
            {
                Write_Back e;
                Memory_Access d;
                Execute c;
                Decode b;
                Fetch a(1);
                Clock++;

                cout << "contrh:" << ControlHazard << endl;
                cout << "dataH:" << DataHazard << endl;
                cout << "refOP:" << RefreshOprands << endl;

                resolveHazards();

                if (printRegFile)
                    printRF();

                if (printPipe)
                {
                    printPipeline();
                    cout << endl
                         << endl
                         << endl
                         << "End of cycle:" << Clock << endl
                         << endl
                         << endl;
                }
            }
        }
        else
        {
            while (1)
            {
                Write_Back e;
                Memory_Access d;
                Execute c;
                Decode b;
                Fetch a(1);
                Clock++;

                cout << "contrh:" << ControlHazard << endl;
                cout << "dataH:" << DataHazard << endl;
                cout << "refOP:" << RefreshOprands << endl;

                ResolveHazard_Using_dataForwarding();

                if (printRegFile)
                    printRF();

                if (printPipe)
                {
                    printPipeline();
                    cout << endl
                         << endl
                         << endl
                         << "End of cycle:" << Clock << endl
                         << endl
                         << endl;
                }
            }
        }
    }
    else
    {
        while (1)
        {
            Fetch a(1);
            Decode b;
            Execute c;
            Memory_Access d;
            Write_Back e;
            currentPCAdd = ex_ma_mainPipeline.nextPCAdd.to_ulong(); // update PC
            Clock+=5;

            if (printRegFile)
                printRF();
        }
    }
}

int main()
{
    btb_nuller(BTB, BUFFER_SIZE);
    // cout << "enter 1 for enable pipeline else 0:";
    // cin >> ispipeLine;

    // cout << "enter 1 for enable data forwarding else 0:";
    // cin >> DataForwarding;

    // cout << "enter 1 for enable printing RF at the end of each cycle else 0:";
    // cin >> printRegFile;

    // cout << "enter 1 for enable printing PipeLine at the end of each cycle else 0:";
    // cin >> printPipe;
    ispipeLine=true;
    DataForwarding = false;
    printRegFile = false;
    printPipe = false;
    //miss = 0;
    // cacheSize = 16;
    // blockSize = 4;
    // // char chr = 'a'; 
    // char chr2[5] = {'1','2','3','4','5'};
    // char chr3[5] = {'!','@','#','$','%'};
    // char chr4[5] = {'a','b','c','d','e'};


    // // char *data = &chr;
    // char *data2 = &(chr2[0]);
    // char *data3 = &(chr3[0]);
    // char *data4 = &(chr4[0]);
    // cout<<"Input Cache Size\n";
    // cin>>cacheSize;
    // cout<<"Input Block Size\n";
    // cin>>blockSize;
    // cout<<"Input Policy (0 - LRU, 1 - FIFO, 2 - LFU)\n";
    // cin>>policy;
    // cout<<"Input Mapping (0 - Direct mapping, 1 - Fully Assosc, 2 - Set Assosc)\n";
    // cin>>mapping;

    policy = 0;
    mapping = 0;

    if(mapping == SET_ASSOSC)
    {
        cout<<"Input Set Assosciative mapping\n";
        cin>>waysOfSetAssosc;
    }
    cache.InitialiseCache(cacheSize, blockSize, policy, mapping, waysOfSetAssosc );

    // //intialise cache capacity, policy
    // // Placeholder_Name(data2, 56, 1, 3);/*OUTPUT 2*/
    // // cache.show_cache();
    // char *charray = Placeholder_Name(data2, 56, 0, 3);/*OUTPUT 2*/
    // cache.show_cache();
    // char *charray = Placeholder_Name(data2, 56, 0, 3);/*OUTPUT 2*/
    // cout<<"Input";
    
    // Placeholder_Name(data3, 29, 1, 3);
    // Placeholder_Name(data4, 19, 1, 5);
    // Placeholder_Name(data2, 101, 1, 3);
    // Placeholder_Name(data2, 18, 1, 5);
    // Placeholder_Name(data2, 10, 1, 5);

    // Placeholder_Name(data2, 28, 0, 0, 0);
    // Placeholder_Name(data, 16, 0, 0);
    // charray++;
    // charray++;
    // charray++;

    // cout<<*(charray);
    init_NoOps();
    make_file();
    RISCv_Processor();
   
    return 0;
}
