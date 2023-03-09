#include <iostream>
#include <fstream>
#include <string>
#include <bitset>

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
#pragma endregion INSTRUCTION_RELATED_DATA 


#pragma region DECODE_RELATED_DATA

typedef struct if_de_hs_de_ex{
    int Op1,Op2;//oprands
    int Rd;//RF write destinstion
    int imm,immU,immS,immJ,immB;
    int branch_target_select;//0 for immB; 1 for immJ
    int Result_select;//0:PC+4 ; 1:ImmU; 2:Load data; 3: ALU result
    int ALU_Operation;//0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt 9:beq 10:bne 11:blt 12:bge 13:lui 14:jal
    int mem_OP;//0:No operation 1:write 2:read
    int RFWrite;//0: no write operation 1:for write operation
    int Store_op;//0:sb 1:sh 2:sw
    int Mem_Op2;//OP2 input for memmory
} If_DE;

If_DE hs_de_ex;

int RF[32];//Register file
#pragma endregion DECODE_RELATED_DATA

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
            addToBitset();
        }
        else if(flag == 1)
        {
            currentPCAdd = nextPCAdd;
            ifstream tempRead("input.mc");
            string tempHexa;
            tempRead>>tempHexa;
            bitset<32> tempBitset(HexStringToBitset(tempHexa));
            while(tempBitset != nextPCAdd)
            {
                tempRead>>tempHexa;
                tempRead>>tempHexa;
                tempBitset = HexStringToBitset(tempHexa); 
            }

            tempRead>>hex_str;
        }
        //stoul converts string of type 0x012312 to its decimal value
        return HexStringToBitset(hex_str);

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

    bitset<32> HexStringToBitset(string hex_instr)
    {
        unsigned long hex_to_dec_val = stoul(hex_instr, nullptr, 16);
        bitset<32> binary_num(hex_to_dec_val);
        return binary_num;
        
    }

    public:
    Fetch(int flag = 0)
    {
        currentInstruction = fetch_instruction(flag);
    }
    

};
//makes .mc file
void make_file()
{
        cout << "Enter input filename: ";
        // cin >> filename;
        filename = "input.txt";


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
            cout << "0x" << hex<<offset << " " << line << endl; 
            //skip 0x part
            string hex_str = line;
            //stoul converts string of type 0x012312 to its decimal value
            unsigned long hex_to_dec_val = stoul(hex_str, nullptr, 16);
            bitset<32> binary_num(hex_to_dec_val);
            cout<<binary_num<<endl;
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
        bitset<5> opcode;
        char type_of_instruction;

        for (int i = 0; i < 5; i++)//ignoring last two bit as these are always 11
        {
            opcode[i]=currentInstruction[i+2];
        }
        // cout<<opcode;

        if((!opcode[4])&&opcode[3]&&opcode[2]&&(!opcode[1])&&(!opcode[0]))//01100
        type_of_instruction = 'R';

        else if((!opcode[4])&&(!opcode[3])&&(!opcode[1])&&(!opcode[0]))//00X00
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
                        }
                        else{
                            hs_de_ex.ALU_Operation=0;
                            
                        }
                    }
                        break;
                    case 4:
                        hs_de_ex.ALU_Operation=2;
                    
                        break;
                    case 6:
                        hs_de_ex.ALU_Operation=3;
                        break;
                    case 7:
                        hs_de_ex.ALU_Operation=4;
                        break;
                    case 1:
                        hs_de_ex.ALU_Operation=5;
                        break;
                    case 5:
                        {
                            if (currentInstruction[30])
                            {
                                hs_de_ex.ALU_Operation=7;
                            }
                            else{
                                hs_de_ex.ALU_Operation=6;
                            }
                        }
                        break;
                    case 2:
                        hs_de_ex.ALU_Operation=8;
                        break;
                                          
                    default:
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

                    hs_de_ex.Op1 = RF[rs1.to_ulong()];//value of rs1
                    hs_de_ex.Op2 = RF[rs2.to_ulong()];//value of rs2
                    hs_de_ex.Rd = rd.to_ulong();//address of RD
                
               }
                break;

            case 'I':
                {
                    bitset<3> func3;
                    func3[0]=currentInstruction[12];
                    func3[1]=currentInstruction[13];
                    func3[2]=currentInstruction[14];

                    switch (func3.to_ulong())//alu op:: 0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt
                    {
                    case 0:
                        hs_de_ex.ALU_Operation=0;
                        break;
                    case 4:
                        hs_de_ex.ALU_Operation=2;
                    
                        break;
                    case 6:
                        hs_de_ex.ALU_Operation=3;
                        break;
                    case 7:
                        hs_de_ex.ALU_Operation=4;
                        break;
                    case 1:
                        hs_de_ex.ALU_Operation=5;
                        break;
                    case 5:
                        {
                            if (currentInstruction[30])
                            {
                                hs_de_ex.ALU_Operation=7;
                            }
                            else{
                                hs_de_ex.ALU_Operation=6;
                            }
                        }
                        break;
                    case 2:
                        hs_de_ex.ALU_Operation=8;
                        break;
                                          
                    default:
                        break;
                    }

                    hs_de_ex.Result_select=3;
                    hs_de_ex.mem_OP=0;
                    hs_de_ex.RFWrite=1;

                    bitset <5> rs1;
                    bitset <5> rd;

                    for (int i = 0; i < 5; i++)
                    {
                        rs1[i]=currentInstruction[i+15];
                        rd[i]=currentInstruction[i+7];     
                    }

                    bitset <12> imm;
                    for (int i = 0; i < 12; i++)
                    {
                        imm[i] = currentInstruction[i+20];
                    }

                    hs_de_ex.Op1 = RF[rs1.to_ulong()];//value of rs1
                    hs_de_ex.Op2 = imm.to_ulong();//value of rs2
                    hs_de_ex.Rd = rd.to_ulong();//address of RD
                }
                break;

            case 'S':
                {
                    
                    bitset<3> func3;
                    func3[0]=currentInstruction[12];
                    func3[1]=currentInstruction[13];
                    func3[2]=currentInstruction[14];
                    
                    hs_de_ex.ALU_Operation=0;//add
                    hs_de_ex.Store_op = func3.to_ulong();
                    hs_de_ex.mem_OP = 1;//write operation
                    hs_de_ex.RFWrite=0;

                    bitset <5> rs1;
                    bitset <5> rs2;

                    for (int i = 0; i < 5; i++)
                    {
                        rs1[i]=currentInstruction[i+15];
                        rs2[i]=currentInstruction[i+20];
                    }

                    
                    bitset<12> immS;
                     for (int i = 0; i < 5; i++)
                     {
                        immS[i] = currentInstruction[7+i];
                     }

                     for (int i = 5; i < 12; i++)
                     {
                        immS[i] = currentInstruction[i+20];
                     }

                     hs_de_ex.Op1 = RF[rs1.to_ulong()];
                     hs_de_ex.Op2 = immS.to_ulong();
                     hs_de_ex.Mem_Op2 = RF[rs2.to_ulong()];

                }
                break;
            case 'B':
               {
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

                    bitset<14> immB;

                    immB[0]=0;//lsb is 0
                    immB[11]=currentInstruction[7];
                    immB[12]=currentInstruction[31];

                    for (int i = 1; i < 5; i++)
                    {
                        immB[i] = currentInstruction[7+i];
                    }

                    for (int i = 5; i < 11; i++)
                    {
                        immB[i] = currentInstruction[20+i];
                    }

                    hs_de_ex.immB = immB.to_ulong();

                    bitset<3> func3;
                    func3[0]=currentInstruction[12];
                    func3[1]=currentInstruction[13];
                    func3[2]=currentInstruction[14];

                    switch (func3.to_ulong())//aluOP== 9:beq 10:bne 11:blt 12:bge
                    {
                    case 0:
                        {
                            hs_de_ex.ALU_Operation=9;
                        }
                        break;
                    case 1:
                        {
                            hs_de_ex.ALU_Operation=10;
                        }
                        break;
                    case 4:
                        {
                            hs_de_ex.ALU_Operation=11;
                            
                        }
                        break;
                    case 5:
                        {
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
     
               }
                break;
            
            case 'U':
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

                hs_de_ex.immU = immU.to_ulong();
                hs_de_ex.mem_OP=0;
                hs_de_ex.RFWrite=1;
                hs_de_ex.Result_select= 1;
                                
                
               }
                break;
            
            case 'J':
                {
                    bitset <21> ImmJ;

                    for (int i = 12; i <20 ; i++)
                    {
                        ImmJ[i]=currentInstruction[i];
                    }

                    ImmJ[11]=currentInstruction[20];
                    ImmJ[0]=0;

                    for (int i = 1; i < 11; i++)
                    {
                        ImmJ[i]=currentInstruction[i+20];
                    }

                    ImmJ[20]=currentInstruction[31];

                    // cout<<"IMMj="<<ImmJ;

                    bitset <5> rd;

                    for (int i = 0; i < 5; i++)
                    {
                        rd[i]=currentInstruction[i+7];
                    }
                    

                    hs_de_ex.immJ = ImmJ.to_ulong();
                    hs_de_ex.mem_OP=0;
                    hs_de_ex.branch_target_select=1;//immj
                    hs_de_ex.Rd = rd.to_ulong();
                    hs_de_ex.ALU_Operation=14;//jal
                    hs_de_ex.Result_select=0;
                    hs_de_ex.RFWrite=1;
                    
                }
                break;

            
            default: 
                {
                    cerr << "Error: Could not identify instruction.\n";
                    exit(0);
                }
                break;
        }
        

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
//#########################################################################################
typedef struct ex_ma_handshake{
   //handshake register between execute and memory access 
    int ALU_result;//0:add 1:sub 2:XOR 3:OR 4:AND 5:sll 6:srl 7:sra 8:slt
    int isBranch;//will tell whether to branch or not

} EX_MA;
EX_MA hs_ex_ma;

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


void execute(){
    int op1 =hs_de_ex.Op1;
    int op2 =hs_de_ex.Op2;
    int ALU_operation=hs_de_ex.ALU_Operation;

    switch (ALU_operation){
    case 0:     //it will perform addition in ALU
    hs_ex_ma.ALU_result=op1+op2;
    break;
    case 1:     //it will perform subtraction in ALU
    hs_ex_ma.ALU_result=op1-op2;
    hs_ex_ma.isBranch=0;
    break;
    case 2:     //it will perform logical XOR in ALU
    hs_ex_ma.ALU_result=op1^op2;
    hs_ex_ma.isBranch=0;
    break;
    case 3:     //it will perform logical OR in ALU
    hs_ex_ma.ALU_result=op1|op2;
    hs_ex_ma.isBranch=0;
    break;
    case 4:     //it will perform logical AND in ALU
    hs_ex_ma.ALU_result=op1&op2;
    hs_ex_ma.isBranch=0;
    break;
    case 5:     //it will perform shift left logical in ALU
    hs_ex_ma.ALU_result=op1<<op2;
    hs_ex_ma.isBranch=0;
    break;
    case 6:     //it will perform shift right logical in ALU
    hs_ex_ma.ALU_result=srl(op1,op2);
    hs_ex_ma.isBranch=0;
    break;
    case 7:     //it will perform shift right arithmetic in ALU
    hs_ex_ma.ALU_result=op1>>op2;
    hs_ex_ma.isBranch=0;
    break;
    
    case 8:     //it will perform set less than in ALU
    if(op1<op2){
        hs_ex_ma.ALU_result=1;
    }
    else hs_ex_ma.ALU_result=0;
    hs_ex_ma.isBranch=0;
    break;
    //for branching, we are assigning 0 for no branch, 1 for branch target adress, and 2 for ALU result, i.e for JALR

    case 9:     //will check for beq
    hs_ex_ma.ALU_result==op1-op2;
    if (hs_ex_ma.ALU_result==0){
        hs_ex_ma.isBranch=1;

        nextPCAdd=hs_de_ex.immB+currentPCAdd.to_ulong(); //making pc=pc+immb
       
    }
    else
        hs_ex_ma.isBranch=0;
    break;

    case 10:     //will check for bne
    hs_ex_ma.ALU_result==op1-op2;
    if (hs_ex_ma.ALU_result==0){
        hs_ex_ma.isBranch=0;
    }
    else{
        hs_ex_ma.isBranch=1;
        nextPCAdd=hs_de_ex.immB+currentPCAdd.to_ulong(); //making pc=pc+immb
    }
    break;


    case 11:     //will check for blt
    hs_ex_ma.ALU_result==op1-op2;
    if (hs_ex_ma.ALU_result<0){
        hs_ex_ma.isBranch=1;
        nextPCAdd=hs_de_ex.immB+currentPCAdd.to_ulong(); //making pc=pc+immb
    }
    else
        hs_ex_ma.isBranch=0;
    break;
    

    case 12:     //will check for bge
    hs_ex_ma.ALU_result==op1-op2;
    if (hs_ex_ma.ALU_result>=0){
        hs_ex_ma.isBranch=1;
        nextPCAdd=hs_de_ex.immB+currentPCAdd.to_ulong(); //making pc=pc+immb
    }
    else
        hs_ex_ma.isBranch=0;
    break;

    case 13:  //jalr
    hs_ex_ma.ALU_result=op1+op2;
    hs_ex_ma.isBranch=2;
    nextPCAdd=hs_de_ex.immJ+currentPCAdd.to_ulong(); //making pc=pc+immb

    case 14:
    case 15:



    default:
    cout<<"some error has occured in decode!!";
    }


int main()
{
    make_file();
    //whenever you create an object pc increments by 4 and new instruction is loaded in currentInstruction global var
    // Fetch a;
    // Fetch b;
    // cout<<endl<<"Current instr"<<currentInstruction;

    Fetch a;
    cout<<endl;
    Decode A;

    return 0;
}