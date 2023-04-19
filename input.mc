0x0 00100613
0x4 19000293
0x8 25800413
0xc 00000313
0x10 00a00193
0x14 02335063
0x18 07c000ef
0x1c 00c2a023
0x20 00c42023
0x24 00428293
0x28 00440413
0x2c 00130313
0x30 fe0002e3
0x34 008000ef
0x38 0900006f
0x3c fd840413
0x40 00300233
0x44 00000313
0x48 00000293
0x4c fff20193
0x50 0442d063
0x54 405181b3
0x58 02335263
0x5c 00042783
0x60 00442803
0x64 04f84a63
0x68 00f42023
0x6c 01042223
0x70 00440413
0x74 00130313
0x78 fe0000e3
0x7c 000201b3
0x80 25800413
0x84 00000313
0x88 00128293
0x8c fc0002e3
0x90 00008067
0x94 00c00533
0x98 00251613
0x9c 00c54533
0xa0 00355613
0xa4 00c54533
0xa8 00451613
0xac 00c54533
0xb0 00a00633
0xb4 00008067
0xb8 00080fb3
0xbc 00078833
0xc0 000f87b3
0xc4 fa0002e3
0xc8 00000033
0xcc 00000033
0xd0 00000033
0xd4 00000033
0xd8 00000033
0xdc FFFFFFFF
0xe0 
0xe4 .main:
0xe8 addi x12,x0,1 #x12 is seed with value 12
0xec #lui x5 0x10001 
0xf0 addi x5 x0 0x190 #400//x5 stored the starting address of array i.e. 0x10001000
0xf4 #lui x8 0x10002
0xf8 addi x8 x0 0x258 #600//x8 stored the starting address of sorted array i.e. 0x10002000
0xfc addi x6 x0 0 #x6 will store the starting index i i.e. 0
0x100 addi x3 x0 10 # store number of elements
0x104 
0x108 .loop: # will execute SortedArray[i]=Array[i]=random() according to the template program in the problem statement
0x10c bge x6 x3 .loopexit
0x110 jal x1,.random # calling random to store random number in x12
0x114 sw x12 0(x5) # storing x12 in memory
0x118 sw x12 0(x8) # storing x12 in memory
0x11c addi x5 x5 4 #incrementing memory address
0x120 addi x8 x8 4 #incrementing memory address
0x124 addi x6 x6 1 #incrementing index register i
0x128 beq x0 x0 .loop # looping back to original
0x12c .loopexit:  #we have successfully created our array in the memory
0x130 
0x134 jal x1 .bubbleSort # calling bubble sort on sorted array location
0x138 
0x13c jal x0,.exit # exiting the program after performing all the computations
0x140 
0x144 
0x148 
0x14c 
0x150 .bubbleSort:
0x154 
0x158 addi x8 x8 -40 # x8 again points to the starting location of our array
0x15c add x4 x0 x3 #n is stored in register x4
0x160 addi x6 x0 0 #x6 will store the starting index j i.e. 0
0x164 addi x5 x0 0 #x6 will store the starting index i i.e. 0
0x168 addi x3 x4 -1
0x16c .loop_outer:
0x170 bge x5 x4 .loop_outer_exit # running n times
0x174 sub x3 x3 x5
0x178 .loop_inner:
0x17c bge x6 x3 .loop_inner_exit
0x180 lw x15 0(x8)#we have derived the number from memory and have stored in x15
0x184 lw x16 4(x8)#we have derived the number from memory and have stored in x16
0x188 blt x16 x15 .swap
0x18c 
0x190 .continue_sort:
0x194 sw x15 0(x8)#we have stored the number into memory from register x15
0x198 sw x16 4(x8)#we have stored the number into memory from register x16
0x19c 
0x1a0 addi x8 x8 4 #incrementing memory address
0x1a4 addi x6 x6 1 #incrementing index register i
0x1a8 beq x0 x0 .loop_inner
0x1ac .loop_inner_exit:# will exit the inner loop
0x1b0 add x3 x4 x0
0x1b4 #lui x8 0x10002
0x1b8 addi x8 x0 0x258 
0x1bc #addi x3 x0 9
0x1c0 addi x6 x0 0 #x6 will store the starting index j i.e. 0
0x1c4 addi x5 x5 1 #incrementing index register i
0x1c8 beq x0 x0 .loop_outer
0x1cc .loop_outer_exit:
0x1d0 jalr x0 x1 0#returning back to main from bubbleSort Function
0x1d4 
0x1d8 .random:#it will generate random numbers from a particular given seed value
0x1dc add x10 x0 x12 # copied value of x12 in x10 i.e. x10 is x
0x1e0 slli x12 x10 2
0x1e4 xor x10 x10 x12
0x1e8 srli x12 x10 3 
0x1ec xor x10 x10 x12
0x1f0 slli x12 x10 4
0x1f4 xor x10 x10 x12
0x1f8 add x12 x0 x10
0x1fc jalr x0 x1 0
0x200 
0x204 .swap:#it will swap two numbers as and when needed
0x208 add x31 x16 x0
0x20c add x16 x15 x0
0x210 add x15 x31 x0
0x214 beq x0 x0 .continue_sort
0x218 
0x21c  .exit: #final exit from program
