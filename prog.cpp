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
        cin >> filename;


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

void reset_pointer()
{

}
int main()
{
    make_file();
    //whenever you create an object pc increments by 4 and new instruction is loaded in currentInstruction global var
    Fetch a;
    Fetch b;
    // cout<<endl<<"Current instr "<<currentInstruction<<endl;
    bitset<32> nextadd(8);
    nextPCAdd = nextadd;
    Fetch c(1);
    cout<<endl<<"current pc add"<<currentPCAdd<<"Current instr "<<currentInstruction<<endl;
    return 0;
}