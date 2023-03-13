#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <string>

#define MEMORY_SIZE 120000
//immb conversion is always a unsigned operation hence we have to convert it to signed one by using typecast
//immB+PC; immB=immb.to_ulong();//unsigned

using namespace std;

#pragma region FILE_RELATED_DATA
    // stores the file name of the dump file
    string filename;
    ifstream readFile("input.mc");
#pragma endregion FILE_RELATED_DATA 


#pragma region INSTRUCTION_RELATED_DATA 
    bitset<32> currentPCAdd(0);
    bitset<32> nextPCAdd(0);
    bitset<32> currentInstruction;
    //if this instruction is read then program exits
    bitset<32> exitInstruction(0xffffffff);
    //stores the pc + 4
    bitset<32> pc_plus_four;
#pragma endregion INSTRUCTION_RELATED_DATA 


#pragma region DECODE_RELATED_DATA
typedef struct if_de_hs_de_ex{
    int Op1,Op2;//oprands
    int Rd;//RF write destinstion
    int imm,immU,immS,immJ,immB;
    int branch_target_select;//0 for immB; 1 for immJ
    int Result_select;//0:PC+4 ; 1:ImmU; 2:Load data; 3: ALU result
    int ALU_Operation;//0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt 9:beq 10:bne 11:blt 12:bge 13:lui 14:jal 15:jalr 16: auipc
    int mem_OP;//0:No operation 1:write 2:read
    int RFWrite;//0: no write operation 1:for write operation
    int Store_load_op;//0:byte 1:half 2:word//to be used by mem to decide how many bit to store
    int Mem_Op2;//OP2 input for memmory
} If_DE;
If_DE hs_de_ex;
int RF[32];//Register file
#pragma endregion DECODE_RELATED_DATA


#pragma region EXECUTE_RELATED_DATA
typedef struct ex_ma_handshake{
   //handshake register between execute and memory access 
    int ALU_result;//0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt
    int isBranch;//will tell whether to branch or not
    int PC_plus_four;//PC + 4
    //branch target address
} EX_MA;
EX_MA hs_ex_ma;
#pragma endregion EXECUTE_RELATED_DATA

#pragma region MEM_ACC_RELATED_DATA
typedef struct ma_wb_handshake{
    //handshake register between memory access and write back
    int loaded_mem;
} MA_WB ;
MA_WB hs_ma_wb;
int memory_arr[MEMORY_SIZE];
#pragma endregion MEM_ACC_RELATED_DATA

class Fetch
{
    bitset<32> fetch_instruction(int flag)
    {
        //the part b4 space PC address
        string hex_str;
        
        if(flag == 0)
        {
            string pc_str = read();
            currentPCAdd = HexStringToBitset(pc_str);
            hex_str = read();
            // addToBitset();//currentPCAdd+=4//it has to be next address
            incrementNextPCAdd();

        }
        else if(flag == 1)
        {
            ifstream tempRead("input.mc");
            string tempHexa;
            tempRead>>tempHexa;
            bitset<32> tempBitset(HexStringToBitset(tempHexa));

            while(tempBitset != currentPCAdd)
            {
                tempRead>>tempHexa;
                tempRead>>tempHexa;
                tempBitset = HexStringToBitset(tempHexa); 
            }

            tempRead>>hex_str;
        }

        bitset<32> currentInstruction = HexStringToBitset(hex_str);
        // cout<<endl<<"READING INSTRUCTION "<<hex_str<<endl;

        if(currentInstruction == exitInstruction) 
        {
            cout<<endl<<"EXITING...\n";

            ofstream memFile;//storing the memmory array in a txt file
            memFile.open("Memmory_Dump.txt");

            for (int i = 0; i<MEMORY_SIZE; i++)
            {
                memFile<<i<<": "<<memory_arr[i]<<(((i+1)%4==0)? "\n": "  |");
            }

            cout<<"\nRegister File is: \n";
            for (int i = 0; i < 32; i++)
            {
                cout<<"x"<<i<<"="<<RF[i]<<endl;
            }

            exit(0);
        }
        //stoul converts string of type 0x012312 to its decimal value

        return currentInstruction;

    }
    string read()
    {
        string line;
        readFile>>line;
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
        unsigned long hex_to_dec_val = stoul(hex_instr, nullptr, 16);
        bitset<32> binary_num(hex_to_dec_val);
        return binary_num;
    }

    public:
    Fetch(int flag = 0)
    {
        cout<<"\n### Fetch ###\n\n";
        currentInstruction = fetch_instruction(flag);
        cout<<"FETCH:Fetch instruction "<<currentInstruction<<" From address "<<currentPCAdd<<endl;
        cout<<"\n### End Fetch ###\n\n";

    }
};
//makes .mc file
void make_file()
{
        cout << "Enter input filename: ";
        cin >> filename;
        // filename = "input.txt";


        ifstream infile;
        //the input dump file from venus
        //format of it is
        //0x12312312
        infile.open(filename);
        ofstream outfile;
        outfile.open("input.mc");
        if (!infile)
        {
            cerr << "Error: Could not open input file.\n";
            exit(0);
        }

        string line;
        int offset = 0;
        
        while (getline(infile, line))
        {
            // cout << "0x" << hex<<offset << " " << line << endl; 
            //skip 0x part
            string hex_str = line;
            hex_str = "0x" + line;
            //stoul converts string of type 0x012312 to its decimal value
            unsigned long hex_to_dec_val = stoul(hex_str, nullptr, 16);
            bitset<32> binary_num(hex_to_dec_val);
            // cout<<binary_num<<endl;
            outfile << "0x" <<hex<<offset << " " << line << endl;
            offset += 4; // increase offset by 4 characters
        }

        infile.close();
        outfile.close(); 
}

class Decode
{
    void Decode_Instruction()
    {
        cout<<"\n### Decode ###\n\n";
        bitset<5> opcode;
        char type_of_instruction;

        for (int i = 0; i < 5; i++)//ignoring last two bit as these are always 11
        {
            opcode[i]=currentInstruction[i+2];
        }
        // cout<<opcode;

        if((!opcode[4])&&opcode[3]&&opcode[2]&&(!opcode[1])&&(!opcode[0]))//01100
        type_of_instruction = 'R';

        else if((!opcode[4])&&(!opcode[3])&&(!opcode[1])&&(!opcode[0])||opcode[4]&&opcode[3]&&(!opcode[2])&&(!opcode[1])&&opcode[0])//00X00 OR 11001
        type_of_instruction = 'I';

        else if((!opcode[4])&&opcode[3]&&(!opcode[2])&&(!opcode[1])&&(!opcode[0]))//01000
        type_of_instruction = 'S';

        else if(opcode[4]&&opcode[3]&&(!opcode[2])&&(!opcode[1])&&(!opcode[0]))//11000
        type_of_instruction = 'B';

        else if(opcode[4]&&opcode[3]&&(!opcode[2])&&opcode[1]&&opcode[0])//11011
        type_of_instruction = 'J';

        else if(opcode[4]&&opcode[3]&&(!opcode[2])&&(!opcode[1])&&opcode[0])//11001
        type_of_instruction = 'I';

        else if((!opcode[4])&&opcode[2]&&(!opcode[1])&&opcode[0])//0X101
        type_of_instruction = 'U';

        else if(opcode[4]&&opcode[3]&&opcode[2]&&(!opcode[1])&&(!opcode[0]))//11100
        type_of_instruction = 'I';

        else type_of_instruction = 'Z';

        // cout<<type_of_instruction;

        switch (type_of_instruction)
        {
            case 'R':
               {
                    cout<<"\nDECODE: ";
                    bitset<3> func3;
                    func3[0]=currentInstruction[12];
                    func3[1]=currentInstruction[13];
                    func3[2]=currentInstruction[14];

                    switch (func3.to_ulong())//alu op:: 0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt
                    {
                    case 0:
                    {
                        if (currentInstruction[30])
                        {
                            hs_de_ex.ALU_Operation=1;
                            cout<<"Operation is SUB";
                        }
                        else{
                            hs_de_ex.ALU_Operation=0;
                            cout<<"Operation is ADD";
                        }
                    }
                        break;
                    case 4:
                        hs_de_ex.ALU_Operation=2;
                        cout<<"Operation is XOR";

                    
                        break;
                    case 6:
                        hs_de_ex.ALU_Operation=3;
                        cout<<"Operation is OR";
                        break;
                    case 7:
                        hs_de_ex.ALU_Operation=4;
                        cout<<"Operation is AND";
                        break;
                    case 1://5:sll 6:srl 7:sra 8:slt
                        hs_de_ex.ALU_Operation=5;
                        cout<<"Operation is SLL";
                        break;
                    case 5:
                        {
                            if (currentInstruction[30])
                            {
                                hs_de_ex.ALU_Operation=7;
                                cout<<"Operation is SRA";
                            }
                            else{
                                hs_de_ex.ALU_Operation=6;
                                cout<<"Operation is SRL";
                            }
                        }
                        break;
                    case 2:
                        hs_de_ex.ALU_Operation=8;
                        cout<<"Operation is SLT";
                        break;
                                          
                    default:
                    {
                        cerr<<"Invalid Instruction\nEXITING...";
                        exit(0);
                    }
                        break;
                    }

                    // cout<<"operation is"<<hs_de_ex.ALU_Operation;

                    hs_de_ex.Result_select=3;
                    hs_de_ex.mem_OP=0;
                    hs_de_ex.RFWrite=1;

                    bitset <5> rs1;
                    bitset <5> rs2;
                    bitset <5> rd;

                    for (int i = 0; i < 5; i++)
                    {
                        rs1[i]=currentInstruction[i+15];
                        rs2[i]=currentInstruction[i+20];
                        rd[i]=currentInstruction[i+7]; 
                    }
                    cout<<", First Operand X"<<rs1.to_ulong()<<", Second Operand X"<<rs2.to_ulong()<<", Destination Registor X"<<rd.to_ulong()<<endl;
                    hs_de_ex.Op1 = RF[rs1.to_ulong()];//value of rs1
                    hs_de_ex.Op2 = RF[rs2.to_ulong()];//value of rs2
                    hs_de_ex.Rd = rd.to_ulong();//address of RD
                    cout<<"DECODE: "<<"Read Register X"<<rs1.to_ulong()<<" = "<<hs_de_ex.Op1<<", X"<<rs2.to_ulong()<<" = "<<hs_de_ex.Op2<<endl;
                
               }
                break;

            case 'I':
                {
                    cout<<"DECODE: I type instruction\n";
                    bitset<3> func3;
                    func3[0]=currentInstruction[12];
                    func3[1]=currentInstruction[13];
                    func3[2]=currentInstruction[14];

                    if(currentInstruction[4])//arithmatic operations
                    {
                        cout<<"Arithmetic Opereation\n";
                        switch (func3.to_ulong())//alu op:: 0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt
                        {
                        case 0:
                            hs_de_ex.ALU_Operation=0;
                            cout<<"Operation is ADD";
                            break;
                        case 4:
                            hs_de_ex.ALU_Operation=2;
                            cout<<"Operation is XOR";
                            break;
                        case 6:
                            hs_de_ex.ALU_Operation=3;
                            cout<<"Operation is OR";
                            break;
                        case 7:
                            hs_de_ex.ALU_Operation=4;
                            cout<<"Operation is AND";
                            break;
                        case 1:
                            hs_de_ex.ALU_Operation=5;
                            cout<<"Operation is SLL";
                            break;
                        case 5:
                            {
                                if (currentInstruction[30])
                                {
                                    hs_de_ex.ALU_Operation=7;
                                    cout<<"Operation is SRA";
                                }
                                else{
                                    hs_de_ex.ALU_Operation=6;
                                    cout<<"Operation is SRL";
                                }
                            }
                            break;
                        case 2:
                            hs_de_ex.ALU_Operation=8;
                            cout<<"Operation is SLT";
                            break;
                                            
                        default:
                        {
                            cout<<"Invalid Instruction. Exiting...";
                            exit(0);
                        }
                            break;
                        }
                        hs_de_ex.Result_select=3;
                        hs_de_ex.mem_OP=0;
                        hs_de_ex.RFWrite=1;
                    }

                    else if(!currentInstruction[6])//Load operation
                    {
                        cout<<"Load Opereation\n";
                        hs_de_ex.ALU_Operation = 0;//add
                        hs_de_ex.Store_load_op=func3.to_ulong();
                        hs_de_ex.Result_select=2;//load data
                        hs_de_ex.mem_OP=2;//read
                        hs_de_ex.RFWrite=1;
                    }
                    else//jalr
                    {
                        cout<<"JALR Opereation\n";
                        hs_de_ex.ALU_Operation = 15;//jalr
                        hs_de_ex.Result_select=0;//PC+4
                        hs_de_ex.mem_OP=0;//no operation
                        hs_de_ex.RFWrite=1;
                    }


                    bitset <5> rs1;
                    bitset <5> rd;
                    bitset <12> temp;

                    for (int i = 0; i < 5; i++)
                    {
                        rs1[i]=currentInstruction[i+15];
                        rd[i]=currentInstruction[i+7];     
                    }

                    for (int i = 0; i < 12; i++)
                    {
                        temp[i] = currentInstruction[i+20];
                    }

                    bitset <32> imm;

                    for (int i = 0; i < 12; i++)
                    {
                        imm[i]=temp[i];                        
                    }

                    for (int i = 12; i < 32; i++)
                    {
                        imm[i]=temp[11];
                    }

                    hs_de_ex.Op1 = RF[rs1.to_ulong()];//value of rs1
                    hs_de_ex.Op2 = (int)imm.to_ulong();//value of rs2
                    hs_de_ex.Rd = rd.to_ulong();//address of RD
                    cout<<", First Operand X"<<rs1.to_ulong()<<", Second Operand imm"<<", Destination Registor X"<<rd.to_ulong()<<endl;
                    cout<<"DECODE: "<<"Read Register X"<<rs1.to_ulong()<<" = "<<hs_de_ex.Op1<<", imm = "<<hs_de_ex.Op2<<endl;
                    
                    
                }
                break;

            case 'S':
                {
                    cout<<"S type instruction\n";
                    bitset<3> func3;
                    func3[0]=currentInstruction[12];
                    func3[1]=currentInstruction[13];
                    func3[2]=currentInstruction[14];
                    
                    hs_de_ex.ALU_Operation=0;//add
                    hs_de_ex.Store_load_op = func3.to_ulong();
                    hs_de_ex.mem_OP = 1;//write operation
                    hs_de_ex.RFWrite=0;

                    bitset <5> rs1;
                    bitset <5> rs2;

                    for (int i = 0; i < 5; i++)
                    {
                        rs1[i]=currentInstruction[i+15];
                        rs2[i]=currentInstruction[i+20];
                    }

                    
                    bitset<12> temp;
                     for (int i = 0; i < 5; i++)
                     {
                        temp[i] = currentInstruction[7+i];
                     }

                     for (int i = 5; i < 12; i++)
                     {
                        temp[i] = currentInstruction[i+20];
                     }

                     bitset <32> immS;
                    for (int i = 0; i < 12; i++)
                    {
                        immS[i]=temp[i];                        
                    }

                    for (int i = 12; i < 32; i++)
                    {
                        immS[i]=temp[11];
                    }

                     hs_de_ex.Op1 = RF[rs1.to_ulong()];
                     hs_de_ex.Op2 = (int) immS.to_ulong();
                     hs_de_ex.Mem_Op2 = RF[rs2.to_ulong()];
                    cout<<"First Operand X"<<rs1.to_ulong()<<", Second Operand immS = "<<immS.to_ulong()<<endl;
                    cout<<"DECODE: "<<"Read Register X"<<rs1.to_ulong()<<" = "<<hs_de_ex.Op1<<", X"<<rs2.to_ulong()<<" = "<<hs_de_ex.Mem_Op2<<endl;
                    

                }
                break;
            case 'B':
               {
                    cout<<"B type instruction\n";
                    hs_de_ex.branch_target_select=0;//immB
                    hs_de_ex.RFWrite= 0;
                    hs_de_ex.mem_OP=0;                    

                    bitset <5> rs1;
                    bitset <5> rs2;

                    for (int i = 0; i < 5; i++)
                    {
                        rs1[i]=currentInstruction[i+15];
                        rs2[i]=currentInstruction[i+20];
                    }

                    hs_de_ex.Op1 = RF[rs1.to_ulong()];
                    hs_de_ex.Op2 = RF[rs2.to_ulong()];

                    bitset<13> temp;

                    temp[0]=0;//lsb is 0
                    temp[11]=currentInstruction[7];
                    temp[12]=currentInstruction[31];

                    for (int i = 1; i < 5; i++)
                    {
                        temp[i] = currentInstruction[7+i];
                    }

                    for (int i = 5; i < 11; i++)
                    {
                        temp[i] = currentInstruction[20+i];
                    }

                    bitset <32> immB;

                    for (int i = 0; i < 13; i++)
                    {
                        immB[i]=temp[i];                        
                    }

                    for (int i = 13; i < 32; i++)
                    {
                        immB[i]=temp[12];
                    }

                    hs_de_ex.immB = (int)immB.to_ulong();

                    bitset<3> func3;
                    func3[0]=currentInstruction[12];
                    func3[1]=currentInstruction[13];
                    func3[2]=currentInstruction[14];

                    switch (func3.to_ulong())//aluOP== 9:beq 10:bne 11:blt 12:bge
                    {
                    case 0:
                        {
                            hs_de_ex.ALU_Operation=9;
                            cout<<"BEQ";
                        }
                        break;
                    case 1:
                        {
                            cout<<"BNE";
                            hs_de_ex.ALU_Operation=10;
                        }
                        break;
                    case 4:
                        {
                            cout<<"BLT";
                            hs_de_ex.ALU_Operation=11;
                            
                        }
                        break;
                    case 5:
                        {
                            cout<<"BGE";
                            hs_de_ex.ALU_Operation=12;
                        }
                        break;
                    
                    default:
                    {
                        cerr << "Error: Could not identify instruction.\n";
                        exit(0);
                    }
                        break;
                    }
                cout<<"\nFirst Operand X"<<rs1.to_ulong()<<", Second Operand X"<<rs2.to_ulong()<<endl;
                cout<<"DECODE: "<<"Read Register X"<<rs1.to_ulong()<<" = "<<hs_de_ex.Op1<<", X"<<rs2.to_ulong()<<" = "<<hs_de_ex.Op2<<", immB = "<<immB.to_ulong()<<endl;
                    
               }
                break;
            
            case 'U':
               {
                cout<<"U type instruction\n";

                if(currentInstruction[5])//lui
                {
                    hs_de_ex.ALU_Operation= 13;

                    bitset <32> immU;
                    for (int i = 0; i < 32; i++)
                    {
                        if(i>=12)
                        immU[i]=currentInstruction[i];
                        else
                        immU[i]=0;
                    }

                    hs_de_ex.immU = (int)immU.to_ulong();
                    hs_de_ex.Result_select= 1;
                    hs_de_ex.mem_OP=0;
                    hs_de_ex.RFWrite=1;

                    bitset<5> rd;

                    for (int i = 0; i < 5; i++)
                    {
                        rd[i]=currentInstruction[i+7];
                    }

                    hs_de_ex.Rd = rd.to_ulong();

                    cout<<"LUI\n";
                    cout<<"DECODE: "<<"Destination Register X"<<rd.to_ulong()<<endl;
                    cout<<"immU = "<<immU.to_ulong()<<endl;
                 

                }
                else//auipc
                {
                    hs_de_ex.ALU_Operation= 16;//must be add

                    bitset <32> immU;
                    for (int i = 0; i < 32; i++)
                    {
                        if(i>=12)
                        immU[i]=currentInstruction[i];
                        else
                        immU[i]=0;
                    }

                    bitset<5> rd;

                    for (int i = 0; i < 5; i++)
                    {
                        rd[i]=currentInstruction[i+7];
                    }
                    hs_de_ex.Rd = rd.to_ulong();

                    hs_de_ex.immU = immU.to_ulong();

                    hs_de_ex.Op1 = immU.to_ulong();//op1=immu
                    hs_de_ex.Op2 = currentPCAdd.to_ulong();//op1=immu

                    hs_de_ex.mem_OP=0;
                    hs_de_ex.RFWrite= 1;
                    hs_de_ex.Result_select=1;

                    cout<<"AUIPC\n";
                    cout<<"DECODE: "<<"Destination Register X"<<rd.to_ulong()<<endl;                 
                    cout<<"First Operand immU = "<<immU.to_ulong()<<", Second Operand PC"<<currentPCAdd.to_ulong()<<endl;
                    
                }

                

                
               }
                break;
            
            case 'J':
                {
                    cout<<"J type instruction\n";
                    bitset <21> temp(0);

                    for (int i = 12; i <20 ; i++)
                    {
                        temp[i]=currentInstruction[i];
                    }

                    temp[11]=currentInstruction[20];
                    temp[0]=0;

                    for (int i = 1; i < 11; i++)
                    {
                        temp[i]=currentInstruction[i+20];
                    }

                    temp[20]=currentInstruction[31];

                    bitset <32> immJ;
                    for (int i = 0; i < 21; i++)
                    {
                        immJ[i]=temp[i];                        
                    }

                    for (int i = 21; i < 32; i++)
                    {
                        immJ[i]=temp[20];
                    }

                    // cout<<"IMMj="<<ImmJ;

                    bitset <5> rd;

                    for (int i = 0; i < 5; i++)
                    {
                        rd[i]=currentInstruction[i+7];
                    }
                    

                    hs_de_ex.immJ = (int)immJ.to_ulong();
                    hs_de_ex.mem_OP=0;
                    hs_de_ex.branch_target_select=1;//immj
                    hs_de_ex.Rd = rd.to_ulong();
                    hs_de_ex.ALU_Operation=14;//jal
                    hs_de_ex.Result_select=0;
                    hs_de_ex.RFWrite=1;

                    cout<<"DECODE: "<<"Destination Register X"<<rd.to_ulong()<<", immJ = "<<immJ.to_ulong()<<endl;
                 

                    
                    
                }
                break;

            
            default: 
                {
                    cerr << "Error: Could not identify instruction.\n";
                    exit(0);
                }
                break;
        }
        
        cout<<"\n### End Decode ###\n\n";
    }


    public:

    Decode()
    {
        Decode_Instruction();
    }

};

void reset_pointer()
{

}

class Execute{

   int srl(int op1_int, int op2){
    //this will ensure logical right shift in case of SRL instruction
        bitset<32>op1=op1_int;//convert input decimal number to a bitset
        bitset<32>op1_shifted;//local bitset for storing shifted bitset
        for(int i=0;i<32;i++){
            op1_shifted[i]=op1[i+op2];
            if(i+op2==31){
                break;
            }
        }
        int shifted_dec=op1_shifted.to_ulong();//converting back the shifted bitset to integer
        return shifted_dec;
    }

    void execute_inst(){
        int op1 =hs_de_ex.Op1;
        int op2 =hs_de_ex.Op2;
        int ALU_operation=hs_de_ex.ALU_Operation;
        cout<<"\n### Execute ###\n\n";

        switch (ALU_operation){
        case 0:     //it will perform addition in ALU also will compute effective address for S and Load instruction

        cout<<"ALU performing addition operation"<<endl;
        cout<<"Execute: ADD "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1+op2;
        hs_ex_ma.isBranch=0;
        break;
        case 1:     //it will perform subtraction in ALU
        cout<<"ALU performing subtraction operation"<<endl;
        cout<<"Execute: SUB "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1-op2;
        hs_ex_ma.isBranch=0;
        break;
        case 2:     //it will perform logical XOR in ALU
        cout<<"ALU performing XOR operation"<<endl;
        cout<<"Execute: XOR "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1^op2;
        hs_ex_ma.isBranch=0;
        break;
        case 3:     //it will perform logical OR in ALU
        cout<<"ALU performing OR operation"<<endl;
        cout<<"Execute: OR "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1|op2;
        hs_ex_ma.isBranch=0;
        break;
        case 4:     //it will perform logical AND in ALU
        cout<<"ALU performing AND operation"<<endl;
        cout<<"Execute: AND "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1&op2;
        hs_ex_ma.isBranch=0;
        break;
        case 5:     //it will perform shift left logical in ALU
        cout<<"ALU performing logical left shift operation"<<endl;
        cout<<"Execute: SLL "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1<<op2;
        hs_ex_ma.isBranch=0;
        break;
        case 6:     //it will perform shift right logical in ALU
        hs_ex_ma.ALU_result=srl(op1,op2);
        cout<<"ALU performing logical right shift operation"<<endl;
        cout<<"Execute: SRL "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.isBranch=0;
        break;
        case 7:     //it will perform shift right arithmetic in ALU
        cout<<"ALU performing arithmetic right shift operation"<<endl;
        cout<<"Execute: SRA "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1>>op2;
        hs_ex_ma.isBranch=0;
        break;
        
        case 8:     //it will perform set less than in ALU
        cout<<"ALU performing set less than operation"<<endl;
        cout<<"Execute: SLT "<<op1<<" and "<<op2<<endl;
        if(op1<op2){
            hs_ex_ma.ALU_result=1;
        }
        else hs_ex_ma.ALU_result=0;

        hs_ex_ma.isBranch=0;
        break;
        //for branching, we are assigning 0 for no branch, 1 for branch target adress, and 2 for ALU result, i.e for JALR

        case 9:     //will check for beq
        cout<<"ALU checking for beq"<<endl;
        cout<<"Execute: SUB "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1-op2;
        if (hs_ex_ma.ALU_result==0){
            hs_ex_ma.isBranch=1;
            cout<<"Branching done"<<endl;
            cout<<"immb="<<hs_de_ex.immB<<endl;
            nextPCAdd=hs_de_ex.immB+currentPCAdd.to_ulong(); //making pc=pc+immb
        
        }
        else
            {hs_ex_ma.isBranch=0;
            cout<<"No branching"<<endl;}
        break;

        case 10:     //will check for bne
        cout<<"Execute: SUB "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1-op2;
        if (hs_ex_ma.ALU_result==0){
            hs_ex_ma.isBranch=0;
            cout<<"No branching"<<endl;
        }
        else{
            hs_ex_ma.isBranch=1;
            cout<<"Branching done"<<endl;
            nextPCAdd=hs_de_ex.immB+currentPCAdd.to_ulong(); //making pc=pc+immb
        }
        break;


        case 11:     //will check for blt
        cout<<"Execute: SUB "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1-op2;
        if (hs_ex_ma.ALU_result<0){
            hs_ex_ma.isBranch=1;
            cout<<"Branching done"<<endl;
            nextPCAdd=hs_de_ex.immB+currentPCAdd.to_ulong(); //making pc=pc+imm
        }
        else
           { hs_ex_ma.isBranch=0;
            cout<<"No branching"<<endl;}
        break;
        

        case 12:     //will check for bge
        cout<<"Execute: SUB "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1-op2;
        cout<<"OP1:"<<hs_de_ex.Op1;
        cout<<"OP2:"<<hs_de_ex.Op2;
        if (hs_ex_ma.ALU_result>=0){
            hs_ex_ma.isBranch=1;
            cout<<"Branching done"<<endl;
            nextPCAdd=hs_de_ex.immB+currentPCAdd.to_ulong(); //making pc=pc+immb
        }
        else
            {hs_ex_ma.isBranch=0;
            cout<<"No branching"<<endl;}
        break;

        case 13: //lui
            hs_ex_ma.isBranch=0;
            cout<<"Executin LUI "<<endl;
            break;
        
        
        case 14: //JAL 
        cout<<"Executing JAL"<<endl;
        hs_ex_ma.isBranch=1;
        nextPCAdd=currentPCAdd.to_ulong()+4;
        nextPCAdd=hs_de_ex.immJ+currentPCAdd.to_ulong(); //making pc=pc+immj
        // cout<<"jal worked :"<<nextPCAdd;
        break;


        case 15:  //jalr
        cout<<"Executing JALR"<<endl;
        cout<<"Execute: ADD "<<op1<<" and "<<op2<<endl;
        hs_ex_ma.ALU_result=op1+op2;
        hs_ex_ma.isBranch=2;
        nextPCAdd=op1+op2; //making pc=pc+immj
        //must give rd in jalr for xi=pc+4
        break;
                
                
        case 16://auipc 
        cout<<"Executing AUIPC"<<endl;
        hs_ex_ma.isBranch=0;
        hs_de_ex.immU=currentPCAdd.to_ulong()+hs_de_ex.immU;
  
        break;
        
        default:
        cout<<"Some error has occured in decode!!";
        }

        cout<<"\n### End Execute ###\n\n";

}



    public:
    Execute(){
        execute_inst();//making a public constructor function
    }
};

class Memory_Access{
    void memory_access(){
        int memop = hs_de_ex.mem_OP;
        int aluresult = hs_ex_ma.ALU_result;
        int storeloadop = hs_de_ex.Store_load_op;
        int memop2 = hs_de_ex.Mem_Op2;
        int loaddata;
        cout<<"\n### Memory Access ###\n\n";
        cout<<"MEMORY: ";

        switch(memop){
            case 0://no memory operation
                cout<<"Not a memory operation"<<endl;
            break;

            case 1:{//write
                switch(storeloadop){
                    case 0: 
                    memory_arr[aluresult] = memop2&255;//sb
                    cout<<"Storing byte "<< (memop2&255) <<endl;
                    break;

                    case 1:
                    memory_arr[aluresult] = memop2&65535;//sh
                    cout<<"Storing half-word "<<(memop2&65535)<<endl;
                    break;

                    case 2:
                    memory_arr[aluresult] = memop2; //sw
                    cout<<"Storing word "<<memop2<<endl;
                    break; 
                }
            }
            break;

            case 2:{//read (load) pending
                switch (storeloadop)
                {
                case 0:
                loaddata = memory_arr[aluresult];//lb
                loaddata = loaddata & 255;
                cout<<"Loading byte in register"<<endl;
                break;
                
                case 1:
                loaddata = memory_arr[aluresult];//lh
                loaddata = loaddata & 65535;
                cout<<"Loading half-word in register"<<endl;
                break;

                case 2:
                loaddata = memory_arr[aluresult];//lw
                cout<<"Loading word in register"<<endl;
                break;
                }
                hs_ma_wb.loaded_mem = loaddata;
            }
            break;

        }

        cout<<"\n### End Memmory Access ###\n\n";

    }
    public:
    Memory_Access()
    {
        memory_access();
    }
};

class Write_Back{
    int resultselect = hs_de_ex.Result_select;
    int rfwrite = hs_de_ex.RFWrite;
    int isbranch = hs_ex_ma.isBranch;
    int rd = hs_de_ex.Rd;
    int aluresult = hs_ex_ma.ALU_result;
    int pc_plus_4 = currentPCAdd.to_ulong()+4;
    int loadeddata = hs_ma_wb.loaded_mem;
    int immu = hs_de_ex.immU;
    void wb(){

        cout<<"\n### Write Back ###\n\n";
        cout<<"WRITEBACK: ";
        switch(isbranch){
            case 0:{//r-type, i-type, failed conditional branching, u-type
                switch(rfwrite){
                    case 0://failed conditional branch
                    currentPCAdd = currentPCAdd.to_ulong()+4;
                    cout<<"No write-back required and current PC updated to PC+4"<<endl;
                    break;

                    case 1://write data in rf
                    switch(resultselect){
                        case 0: //pc+4
                        RF[rd] = currentPCAdd.to_ulong()+4;
                        break;

                        case 1: //immu - lui & auipc
                        RF[rd] = immu; 
                        break;

                        case 2: //ld operation
                        RF[rd] = loadeddata;
                        break;

                        case 3: //r-type and i-type operations
                        RF[rd] = aluresult;
                        break;
                    }
                    cout<<"Write "<<RF[rd]<<" to register "<<rd<<endl;   
                    currentPCAdd = currentPCAdd.to_ulong()+4;
                    cout<<"Current PC updated to PC+4"<<endl;
                    break;
                }
            }
            break;

            case 1://jal and conditional branches
            switch(rfwrite){
                case 0: //conditional branch
                currentPCAdd = nextPCAdd.to_ulong();
                cout<<"Condtional branch"<<", current PC = "<<currentPCAdd<<endl;
                
                break;

                case 1://jal
                RF[rd] = currentPCAdd.to_ulong()+4;
                currentPCAdd = nextPCAdd.to_ulong();
                cout<<"Write "<<RF[rd]<<" to register "<<rd<<endl;
                cout<<"Current PC address updated to: "<<currentPCAdd;
                break;
            }
            break;

            case 2://jalr
            RF[rd] = currentPCAdd.to_ulong()+4;
            currentPCAdd = aluresult;
            cout<<"Write "<<RF[rd]<<" to register "<<rd<<endl;
            cout<<"Current PC address updated to: "<<currentPCAdd;
            break;
        }
        RF[0]=0;  //x0 is always 0;

        cout<<"\n### End Write Back ###\n\n";

    }
    public:
    Write_Back()
    {
        wb();
    }
};

void RISCv_Processor()
{
    RF[2]=MEMORY_SIZE-1;
    while(1)
    {
        Fetch a(1);
        Decode b;
        Execute c;
        Memory_Access d;
        Write_Back e;
    }
}

int main()
{
    make_file();
    RISCv_Processor();
    //whenever you create an object pc increments by 4 and new instruction is loaded in currentInstruction global var
    // Fetch a;
    // Fetch b;
    // cout<<endl<<"Current instr"<<currentInstruction;


    // for(int i = 0; i < 6; i++)
    // Fetch a;
    // cout<<endl<<"decode part\n";
    // Decode b;
    // cout<<endl<<"execute part\n";
    // Execute c;
    // Memory_Acess d;
    // Write_Back e;

    // Fetch a1;
    // cout<<endl<<"decode part\n";
    // Decode b1;
    // cout<<endl<<"execute part\n";
    // Execute c1;
    // Memory_Acess d1;
    // Write_Back e1;

    
    return 0;
}
