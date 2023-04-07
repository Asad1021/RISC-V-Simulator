void btb_printer(B_T_B BTB[],int n){
//will print our Branch Target Buffer
cout<<endl<<endl;
cout<<"                                         BRANCH TARGET BUFFER"<<endl<<endl;
cout<<"               PC                |          Target Address          | Taken/Not Taken"<<endl;
for(int i=0;i<n;i++){
    cout<<BTB[i].currentPCAdd<<" | ";
    cout<< BTB[i].predictedAdd<<" | ";
    cout<<"     " <<BTB[i].taken;
    cout<<endl;
   
}
}