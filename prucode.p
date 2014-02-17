// prucode.p

.origin 0
.entrypoint START

#include "prucode.hp"

#define PWM_PERIOD_CYCLES 4000000
#define GPIO_CLEARDATAOUT 0x190
#define GPIO_SETDATAOUT 0x194


START:

    // Enable OCP master port
    LBCO      r0, CONST_PRUCFG, 4, 4
    CLR     r0, r0, 4         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    SBCO      r0, CONST_PRUCFG, 4, 4

    // Configure the programmable pointer register for PRU0 by setting c28_pointer[15:0]
    // field to 0x0120.  This will make C28 point to 0x00012000 (PRU shared RAM).
    MOV     r0, 0x00000120
    MOV       r1, CTPPR_0
    ST32      r0, r1

    // Configure the programmable pointer register for PRU0 by setting c31_pointer[15:0]
    // field to 0x0010.  This will make C31 point to 0x80001000 (DDR memory).
    MOV     r0, 0x00100000
    MOV       r1, CTPPR_1
    ST32      r0, r1

    //Load values from external DDR Memory into Registers R0/R1/R2
    LBCO      r0, CONST_DDR, 0, 12

    //Store values from read from the DDR memory into PRU shared RAM
    SBCO      r0, CONST_PRUSHAREDRAM, 0, 12

    // test GP output
    MOV r1, 10 // loop 10 times
OUTERLOOP:
    // reload values from memory into r0/r1/r2
    LBCO       r0, CONST_DDR, 0, 12
    //check close register
    QBLT EXIT, r2, 0 
    SET r30.t5
INNERLOOP:
    QBGE OUTERLOOP, r0, 0
    QBLT PWM2, r1, 0
    CLR r30.t5
PWM2:
    SUB r1, r1, 4  //decrement each PWM counter   
    SUB r0, r0, 4	//decrement period counter by number of operations
    JMP INNERLOOP


EXIT:
    // Send notification to Host for program completion
    MOV       r31.b0, PRU0_ARM_INTERRUPT+16

    // Halt the processor
    HALT


