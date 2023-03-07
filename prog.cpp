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
    int currentPCAdd = 0;
    bitset<32> currentInstruction;
#pragma endregion INSTRUCTION_RELATED_DATA 


#pragma region DECODE_RELATED_DATA
typedef struct if_de_handshake{
    

} IfDE;

IfDE hs1;
#pragma endregion DECODE_RELATED_DATA

class Fetch
{
    bitset<32> fetch_instruction()
    {
        string line;
        //to ignore the first part before space
        readFile>>line;
        readFile>>line;
        string hex_str = line;
        //stoul converts string of type 0x012312 to its decimal value
        unsigned long hex_to_dec_val = stoul(hex_str, nullptr, 16);
        bitset<32> binary_num(hex_to_dec_val);
        cout<<binary_num;
        return binary_num;

    }
    public:
    Fetch()
    {
        //everytime object created increase the value of PC Address by 4
        currentPCAdd += 4;
        currentInstruction = fetch_instruction();
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
            // cout << "0x" << offset << " " << line << endl; 
            //skip 0x part
            string hex_str = line;
            //stoul converts string of type 0x012312 to its decimal value
            unsigned long hex_to_dec_val = stoul(hex_str, nullptr, 16);
            bitset<32> binary_num(hex_to_dec_val);
            // cout<<binary_num<<endl;
            outfile << "0x" << offset << " " << line << endl;
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
                
               }
                break;

            case 'I':
                {
                    
                }
                break;
    
            case 'B':
               {
                
               }
                break;
            
            case 'U':
               {
                
               }
                break;
            
            case 'J':
                {
                    
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