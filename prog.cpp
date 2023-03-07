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
            cout << "0x" << offset << " " << line << endl; 
            //skip 0x part
            string hex_str = line;
            //stoul converts string of type 0x012312 to its decimal value
            unsigned long hex_to_dec_val = stoul(hex_str, nullptr, 16);
            bitset<32> binary_num(hex_to_dec_val);
            cout<<binary_num<<endl;
            outfile << "0x" << offset << " " << line << endl;
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
    cout<<endl<<"Current instr"<<currentInstruction;
    return 0;
}