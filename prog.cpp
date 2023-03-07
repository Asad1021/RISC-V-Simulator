#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main()
{
    string filename;
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
        return 1;
    }

    string line;
    int offset = 0;
    
    while (getline(infile, line))
    {
        cout << "0x" << offset << " " << line << endl; 
        outfile << "0x" << offset << " " << line << endl;
        offset += 4; // increase offset by 4 characters
    }

    infile.close();
    outfile.close();
    return 0;
}
