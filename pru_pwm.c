/*
 * mytest.c
 */

/******************************************************************************
* Include Files                                                               *
******************************************************************************/

// Standard header files
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

// Driver header file
#include "prussdrv.h"
#include <pruss_intc_mapping.h>

/******************************************************************************
* Explicit External Declarations                                              *
******************************************************************************/

/******************************************************************************
* Local Macro Declarations                                                    *
******************************************************************************/

#define PRU_NUM 	 0
//number of PRU cycles to achieve 50Hz
#define PERIOD_CYCLES	 	 4000000
//PWM value of 1060us
#define DUTY_CYCLES		 212000
//trigger value to tell the PRU to stop
#define STOP_FLAG		 0

//memory addresses
#define DDR_BASEADDR     0x80000000
#define OFFSET_DDR	 0x00001000
#define OFFSET_SHAREDRAM 2048		//equivalent with 0x00002000

#define PRUSS0_SHARED_DATARAM    4

/******************************************************************************
* Local Typedef Declarations                                                  *
******************************************************************************/


/******************************************************************************
* Local Function Declarations                                                 *
******************************************************************************/

static int LOCAL_exampleInit ( );
static unsigned short LOCAL_examplePassed ( unsigned short pruNum );

/******************************************************************************
* Local Variable Definitions                                                  *
******************************************************************************/


/******************************************************************************
* Intertupt Service Routines                                                  *
******************************************************************************/


/******************************************************************************
* Global Variable Definitions                                                 *
******************************************************************************/

static int mem_fd;
static void *ddrMem, *sharedMem;

static unsigned int *sharedMem_int;

/******************************************************************************
* Global Function Definitions                                                 *
******************************************************************************/

void intHandler(int dummy){
    exit(1);
}

int main (void)
{
    unsigned int ret;
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    printf("\nINFO: Starting single channel PWM on P9_27.\r\n");

    /* Initialize the PRU */
    prussdrv_init ();

    /* Open PRU Interrupt */
    ret = prussdrv_open(PRU_EVTOUT_0);
    if (ret)
    {
        printf("prussdrv_open open failed\n");
        return (ret);
    }

    /* Get the interrupt initialized */
    prussdrv_pruintc_init(&pruss_intc_initdata);

    /* Initialize example */
    printf("\tINFO: Initializing PWM with default values.\r\n");
    //LOCAL_exampleInit(PRU_NUM);


//////////////////// LOAD VALUES INTO MEMORY /////////////////
    void *DDR_regaddr1, *DDR_regaddr2, *DDR_regaddr3;

    /* open the device */
    mem_fd = open("/dev/mem", O_RDWR);
    if (mem_fd < 0) {
        printf("Failed to open /dev/mem (%s)\n", strerror(errno));
        return 1;
    }

    /* map the DDR memory */
    ddrMem = mmap(0, 0x0FFFFFFF, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, DDR_BASEADDR);
    if (ddrMem == NULL) {
        printf("Failed to map the device (%s)\n", strerror(errno));
        close(mem_fd);
        return 1;
    }

    /* Store Addends in DDR memory location */
    DDR_regaddr1 = ddrMem + OFFSET_DDR;
    DDR_regaddr2 = ddrMem + OFFSET_DDR + 0x00000004;
    DDR_regaddr3 = ddrMem + OFFSET_DDR + 0x00000008;

    *(unsigned int*) DDR_regaddr1 = PERIOD_CYCLES;
    *(unsigned int*) DDR_regaddr2 = DUTY_CYCLES;
    *(unsigned int*) DDR_regaddr3 = STOP_FLAG;

    /* Execute example on PRU */
    printf("\tINFO: Starting PWM output on P9_27.\r\n");
    prussdrv_exec_program(PRU_NUM, "./prucode.bin"); // load pru firmware

    signal(SIGINT, intHandler);

    // again, keep supposedly safe values for start (middle of range shouldn't arm anything)
    int last_input = 1500;
    int input = 1500;
    int converted = 300000;
    while(input){
        *(unsigned int*) DDR_regaddr2 = converted; // set register value in memory
        printf("Value entered: %i\n Value sent: %i\n", input, converted);
        last_input = input;
        scanf(" %i", &input); // wait for new input
        if(input > 1010 && input < 1900){
            // sane value
            converted = input * 1000 / 5;  //input in microseconds converted to nanoseconds and then divided by 5 to get cycles
        } else if (input != 0) {
            // restore sanity
            input = last_input;
        }
    }
    *(unsigned int*) DDR_regaddr3 = 1;  // set close register, wait for PRU to halt

    /* Wait until PRU0 has finished execution */
    printf("\tINFO: Waiting for HALT command.\r\n");
    prussdrv_pru_wait_event (PRU_EVTOUT_0);
    printf("\tINFO: PRU completed transfer.\r\n");
    prussdrv_pru_clear_event (PRU0_ARM_INTERRUPT, PRU_EVTOUT_0);

    /* Disable PRU and close memory mapping*/
    prussdrv_pru_disable(PRU_NUM);
    prussdrv_exit ();
    munmap(ddrMem, 0x0FFFFFFF);
    close(mem_fd);

    return(0);
}
