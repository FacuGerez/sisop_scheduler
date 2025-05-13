.globl context_switch;
context_switch:
    // Make stack point to Trapframe address
    movl 4(%esp), %esp
    // Restore all general-purpose registers
    popal
    // Restore %es and %ds
    pop %es
    pop %ds
    // Discard tf_trapno and tf_err
    add $8, %esp
    // Return to user space
    iret
spin:
	jmp spin
