//Asad Alam
//2021CSB1271
#include<iostream>
#include<string>
#include<math.h>
#include<bitset>

using namespace std;
typedef struct BranchTargetBuffer
{   bitset<32> currentPCAdd;
    bitset<32> predictedAdd;
    bool taken;
    bool valid;
}B_T_B;

B_T_B BTB[1000];
int BTB_index=0;



void btb_nuller(B_T_B BTB[],int n){
//this will flush everything out of our Branch target buffer

for(int i=0;i<n;i++){
    BTB[i].currentPCAdd=0;
    BTB[i].predictedAdd=0;
    BTB[i].taken=false;
}
}
int btb_traversor(bitset<32>pc){
    //will check if a particular pc is there or not in the branch target buffer
    int i=0;
    int flag=0;
   
   
    for(int i=0;i<1000;i++){
        if(BTB[i].currentPCAdd==pc){
            flag=1;
        }
        i++;
        bitset<32>check_pc=BTB[i].currentPCAdd;
    }
    return flag;
}
void btb_runner(bitset<32> pc, bitset<32>ta,bool taken){   
    //will add suitable entries to our BTB ensuring only discrete values crept in  
    int flag=btb_traversor(pc);
    if(flag==0){    
        BTB[BTB_index].currentPCAdd=pc;
        BTB[BTB_index].predictedAdd= ta;
        BTB[BTB_index].taken=taken;
        BTB_index++;
    }
    else{
        return;
    }

}
void btb_printer(B_T_B BTB[],int n){
//will print our Branch Target Buffer

for(int i=0;i<n;i++){
   cout<<BTB[i].currentPCAdd<<endl;
   cout<< BTB[i].predictedAdd<<endl;
   cout<< BTB[i].taken<<endl;
   cout<<endl;
   cout<<endl;
}
}
int main(){

bitset<32>a=5;
bitset<32>b=1;
bitset<32>c=21;
// btb_printer(BTB,10);
btb_nuller(BTB,10);

btb_runner(a,b,1);
btb_runner(a,b,1);
btb_runner(b,c,1);


btb_printer(BTB,10);

// cout<<BTB[0].currentPCAdd<<endl;



    
return 0;
}