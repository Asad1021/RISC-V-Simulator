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
int btb_traversor(bitset<32>pc,bool taken){
    //will check if a particular pc is there or not in the branch target buffer
    int i=0;
    int flag=0;
   
   
    for(int i=0;i<1000;i++){
        if(BTB[i].currentPCAdd==pc){
            if((BTB[i].taken)==taken){
            flag=1;
            }
            else{
                flag=-1;
            }
        }
        i++;
        bitset<32>check_pc=BTB[i].currentPCAdd;
    }
    return flag;
}
void btb_runner(bitset<32> pc, bitset<32>ta,bool taken){   
    //will add suitable entries to our BTB ensuring only discrete values crept in  
    int flag=btb_traversor(pc,taken);
    if(flag==0){    
        BTB[BTB_index].currentPCAdd=pc;
        BTB[BTB_index].predictedAdd= ta;
        BTB[BTB_index].taken=taken;
        BTB_index++;
    }
    else if (flag==-1){
        BTB[BTB_index].taken=taken;
    }
    else{
        return;
    }

}

bitset<32> predicted_address(bitset<32>pc){
    //this will give predicted address when the pc is in the BTB, else will give -1, indicating that given pc was not found in the buffer
    int flag=0;
    for(int i=0;i<1000;i++){
        if(BTB[i].currentPCAdd==pc){
            if((BTB[i].taken)==1){
            flag=1;
            return BTB[i].predictedAdd;
            }
            else{
                return -1;
            }
        }
        else{
            return -1;
        }
        bitset<32>check_pc=BTB[i].currentPCAdd;
    }
    return flag;
}


void btb_printer(B_T_B BTB[],int n){
//will print our Branch Target Buffer
cout<<endl<<endl;
cout<<"                                         BRANCH TARGET BUFFER"<<endl<<endl;
cout<<"_________________________________________________________________________________________"<<endl<<endl;
cout<<"|              PC                  |          Target Address          | Taken/Not Taken |"<<endl;
for(int i=0;i<n;i++){
    cout<<"| "<<BTB[i].currentPCAdd<<" | ";
    cout<< BTB[i].predictedAdd<<" | ";
    cout<<"       " <<BTB[i].taken <<"        |";
    cout<<endl;
}
cout<<"_________________________________________________________________________________________"<<endl;
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


btb_printer(BTB,4);
cout<<predicted_address(4)<<endl;

// cout<<BTB[0].currentPCAdd<<endl;



    
return 0;
}