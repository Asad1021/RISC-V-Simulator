//pending: u-type and uncondtional branch
#include<iostream>
using namespace std;

int memarr[2000];
int regarr[32];
int pc; 

void wb(int memwrite, int memread, int resultselect, int rfwrite, int isbranch, int aluresult, int rd, int rs2){
    if(isbranch){
        if(rfwrite){
            
        }else{
            pc+=aluresult;
        }
    }else{
        regarr[rd] = aluresult;
    }
    fetch();
}

void ma(int memwrite, int memread, int resultselect, int rfwrite, int isbranch, int aluresult, int rd, int rs2){
    if(memwrite==0&&memread==0){
        wb(memwrite, memread, resultselect, rfwrite, isbranch, aluresult, rd, rs2);
    }else{
        if(memwrite){//store
            int* ptr = reinterpret_cast<int*>(aluresult); // Declare pointer variable and initialize it with the value of aluresult
            *ptr = regarr[rs2]; 
        }else{//load
            int* ptr = reinterpret_cast<int*>(aluresult); // Declare pointer variable and initialize it with the value of aluresult
            regarr[rd] = *ptr; 
        }
        wb(memwrite, memread, resultselect, rfwrite, isbranch, aluresult, rd, rs2);
    }
}



int main(){
    int resultselect, memwrite, memread, rfwrite, isbranch;
    cout<<memarr;
}
