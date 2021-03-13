.data
    sizeOfPostFix: .space 10002
    myArray: 
        .align 2
        .space 40000
    

.text
.globl main

main:
    addi $r1,$r1,8
    addi $r2,$r2,10
    sw $r2, 1000 
    lw $r0, 1000 
    #Exiting Program after printing result
    li	$v0, 10
	syscall

