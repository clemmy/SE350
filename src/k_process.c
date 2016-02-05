/**
 * @file:   k_process.c  
 * @brief:  process management C file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/02/28
 * NOTE: The example code shows one way of implementing context switching.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes only two simple user processes and NO HARDWARE INTERRUPTS. 
 *       The purpose is to show how context switch could be done under stated assumptions. 
 *       These assumptions are not true in the required RTX Project!!!
 *       If you decide to use this piece of code, you need to understand the assumptions and
 *       the limitations. 
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "uart_polling.h"
#include "k_process.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
PCB **gp_pcbs;                  /* array of pcbs */
PCB *gp_current_process = NULL; /* always point to the current RUN process */
PCB ***PQueueFirst;
PCB ***PQueueLast;
/* PCB **BlockedQueueFirst;
PCB **BlockedQueueLast; */

U32 g_switch_flag = 0;          /* whether to continue to run the process before the UART receive interrupt */
                                /* 1 means to switch to another process, 0 means to continue the current process */
				/* this value will be set by UART handler */

/* process initialization table */
PROC_INIT g_proc_table[NUM_TEST_PROCS];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];

/**
 * @biref: initialize all processes in the system
 * NOTE: We assume there are only two user processes in the system in this example.
 */
 
 void processEnqueue(PCB* thePCB)
{
	int priority = thePCB->m_priority; //priority is 0, 1, 2 or 3
	PCB** pqf = (PCB**)(PQueueFirst);
	PCB** pql = (PCB**)(PQueueLast);
	
	if (pqf[priority] == NULL){
		pqf[priority] = thePCB;
		pql[priority] = thePCB;
		return;
	}
	
	pql[priority]->nextPCB = thePCB;
	pql[priority] = thePCB;
}

PCB* processDequeue()
{
	PCB** pqf = (PCB**)(PQueueFirst);
	PCB** pql = (PCB**)(PQueueLast);
	int i;
	PCB* returnPCB;
	
	for (i = 0; i < 4; i++){
		if (pqf[i] != NULL){
			returnPCB = pqf[i];
			pqf[i] = pqf[i]->nextPCB;
			returnPCB->nextPCB = NULL;
			if (pqf[i] == NULL){
				pql[i] = NULL;
			}
			return returnPCB;
		}
	}
	return NULL;
}
 
/*void blockedEnqueue(PCB* thePCB)
{
	
	if (BlockedQueueFirst == NULL){
		BlockedQueueFirst = thePCB;
		BlockedQueueLast = thePCB;
		return;
	}
	
	BlockedQueueLast->nextPCB = thePCB;
	BlockedQueueLast = thePCB;
}

PCB* blockedDequeue()
{
	int i;
	PCB* returnPCB;
	
	if (BlockedQueueFirst != NULL){
		returnPCB = BlockedQueueFirst;
		BlockedQueueFirst = BlockedQueueFirst->nextPCB;
		returnPCB->nextPCB = NULL;
		if (BlockedQueueFirst == NULL){
			BlockedQueueLast = NULL;
		}
		return returnPCB;
	}
	return NULL;
} 

int blockedIsEmpty() {
	
	
}*/

/*void makeReady()
{
	PCB* thePCB = blockedDequeue();
	thePCB->m_state = RDY;
	processEnqueue(thePCB);
	k_release_processor();
}

void makeBlock()
{
	gp_current_process->m_state = BLK;
	blockedEnqueue(gp_current_process);
	gp_current_process = NULL;
	k_release_processor();
} */
 
void process_init() 
{
	int i;
	U32 *sp;
	PCB** pqf = (PCB**)(*PQueueFirst);
	PCB** pql = (PCB**)(*PQueueLast);
  
        /* fill out the initialization table */
	set_test_procs();
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_proc_table[i].m_pid = g_test_procs[i].m_pid;
		g_proc_table[i].m_priority = g_test_procs[i].m_priority;
		g_proc_table[i].m_stack_size = g_test_procs[i].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_test_procs[i].mpf_start_pc;
	}
  
	/* initilize exception stack frame (i.e. initial context) for each process */
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		int j;
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
		(gp_pcbs[i])->nextPCB = NULL;
		
		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;
	}
	
	for ( i = 0; i < 4; i++ ) {
		//insert PCB[i] into priority queue
		pqf[i] = NULL;
		pql[i] = NULL;
	} //PQueueFirst[0] PQueueFirst[1] PQueueFirst[2] PQueueFirst[3] 
	
	for ( i = 0; i < NUM_TEST_PROCS - 1; i++ ) {
		//insert PCB[i] into priority queue
		printf("iValue 0x%x \n", gp_pcbs[i]);
		processEnqueue(gp_pcbs[i]);
	}
	
	/* initialize priority queue */
	
	
} //gp_pcbs[0] gp_pcbs[1] gp_pcbs[2] gp_pcbs[3] gp_pcbs[4] gp_pcbs[5]

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */

PCB *scheduler(void)
{
	if (gp_current_process == NULL) {
		printf("HI");
		return gp_pcbs[5];
	}

	processEnqueue(gp_current_process);
	return processDequeue();
	
}

/*@brief: switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 *@param: p_pcb_old, the old pcb that was in RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_pcb_old and gp_current_process are pointing to valid PCBs.
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
int process_switch(PCB *p_pcb_old) 
{
	PROC_STATE_E state;
	
	state = gp_current_process->m_state;

	if (state == NEW) {
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
			p_pcb_old->m_state = RDY;
			p_pcb_old->mp_sp = (U32 *) __get_MSP();
		}
		gp_current_process->m_state = RUN;
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	} 
	
	/* The following will only execute if the if block above is FALSE */

	if (gp_current_process != p_pcb_old) {
		if (state == RDY){ 		
			p_pcb_old->m_state = RDY; 
			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
			gp_current_process->m_state = RUN;
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack    
		} else {
			gp_current_process = p_pcb_old; // revert back to the old proc on error
			return RTX_ERR;
		} 
	}
	return RTX_OK;
}
/**
 * @brief release_processor(). 
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void)
{
	PCB *p_pcb_old = NULL;
	
	p_pcb_old = gp_current_process;
	gp_current_process = scheduler();
	
	if ( gp_current_process == NULL  ) {
		gp_current_process = p_pcb_old; // revert back to the old process
		return RTX_ERR;
	}
  if ( p_pcb_old == NULL ) {
		p_pcb_old = gp_current_process;
	}
	process_switch(p_pcb_old);
	return RTX_OK;
}

int set_process_priority(int process_id, int priority){
	int i;
	for (i = 0; i < NUM_TEST_PROCS; i++){
		if (gp_pcbs[i]->m_pid == process_id){
			#ifdef DEBUG_0
			printf("Setting Process Priority: %d\n", priority);
			#endif /* DEBUG_0 */
			gp_pcbs[i]->m_priority = priority;
			return RTX_OK;
		}
	}
	#ifdef DEBUG_0
			printf("Setting process priority failed.");
	#endif /* DEBUG_0 */
	k_release_processor();
	return RTX_ERR;
}

int get_process_priority(int process_id){
	int i;
	int priority = -1;
	for (i = 0; i < NUM_TEST_PROCS; i++){
		if (gp_pcbs[i]->m_pid == process_id){
			priority = gp_pcbs[i]->m_priority;
			#ifdef DEBUG_0
			printf("Getting Process Priority: %d\n", priority);
			#endif /* DEBUG_0 */
			return priority;
		}
	}
	#ifdef DEBUG_0
			printf("Getting Process Priority: %d\n", priority);
	#endif /* DEBUG_0 */
	return priority;
}
