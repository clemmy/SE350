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
#include "k_rtx.h"
#include "timer.h"
#include "k_message.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
PCB **gp_pcbs;                  /* array of pcbs */
PCB *gp_current_process = NULL; /* always point to the current RUN process */
PCB* null_pcb;

PCBQ ReadyPQ[NUM_OF_PRIORITIES];
PCBQ BlockPQ[NUM_OF_PRIORITIES];

U32 g_switch_flag = 0;          /* whether to continue to run the process before the UART receive interrupt */
                                /* 1 means to switch to another process, 0 means to continue the current process */
        /* this value will be set by UART handler */

/* process initialization table */
PROC_INIT g_proc_table[NUM_PROCS];

extern char* copyStr(char* src, char* dest);

extern void procA(void);
extern void procB(void);
extern void procC(void);
extern void setPriorityProc(void);
extern void wallClockProc(void);
extern void kcdProc(void);
extern void crtProc(void);
extern void TIMER0_IRQHandler(void);
extern void UART0_IRQHandler(void);


/**
 * @brief: a process that
 *         yields the cpu.
 *//*
void procA(void)
{
  while (1) {
    k_release_processor();
  }
}

void procB(void)
{
  while (1) {
    k_release_processor();
  }
}

void procC(void)
{
  while (1) {
    k_release_processor();
  }
}*/

/**
 * @brief: Enqueues the PCB into its corresponding queue (based on priority)
 */
void processEnqueue(PCBQ pq[], PCB* thePCB)
{
  int priority = thePCB->m_priority; //priority is 0, 1, 2 or 3

  if (pq[priority].tail == NULL){
    pq[priority].tail = thePCB;
    pq[priority].head = thePCB;
    return;
  }

  pq[priority].tail->nextPCB = thePCB;
  pq[priority].tail = thePCB;
}

/**
 * @brief: Dequeues the first element from the queue with the highest priority
 */
PCB* processDequeue(PCBQ pq[])
{
  PCB* returnPCB;
  int i;
  for (i = 0; i < NUM_OF_PRIORITIES; i++){
    if (pq[i].head != NULL){
      returnPCB = pq[i].head;
      pq[i].head = pq[i].head->nextPCB;
      returnPCB->nextPCB = NULL;
      if (pq[i].head == NULL){
        pq[i].tail = NULL;
      }
      return returnPCB;
    }
  }
  return NULL;
}

int exists_higher_priority_ready_process() {
	for (int i = 0; i < NUM_OF_PRIORITIES; i++){
    if (ReadyPQ[i].head != NULL && i < gp_current_process->m_priority){
      return 1;
    }
  }
	return 0;
}

/**
 * @brief: Transfers the highest priority PCB from the block queue to the ready queue
 */
void makeReady()
{
	while (!blockPQIsEmpty()) {
		PCB* thePCB = processDequeue(BlockPQ);
		thePCB->m_state = RDY;
		processEnqueue(ReadyPQ, thePCB);
	}
	if(exists_higher_priority_ready_process()){
		k_release_processor();
	}
}

/**
 * @brief: Sets state of current process to blocked
 */
void makeBlock()
{
  gp_current_process->m_state = BLK;
  k_release_processor();
}

/**
 * @brief: Checks if the queue is empty
 */
int queueIsEmpty(PCBQ pq[]) {
  int i;
  for (i = 0; i < NUM_OF_PRIORITIES; i++) {
		if (pq[i].head != NULL) {
			return 0;
		}
  }
  return 1;
}

/**
 * @brief: Checks if the block queue is empty
 */
int blockPQIsEmpty() {
  return queueIsEmpty(BlockPQ);
}

/**
 * @brief: Initializes priority queues, block queues, PCBs, and process tables
 */
void process_init()
{
  int i;
  U32 *sp;

  /* fill out the initialization table */
  set_test_procs();

  g_proc_table[PID_NULL].m_pid = PID_NULL;
  g_proc_table[PID_NULL].m_priority = NULL_PRIORITY;
  g_proc_table[PID_NULL].m_stack_size = 0x100;
  g_proc_table[PID_NULL].mpf_start_pc = &nullProc;

	// init test processes
  for ( i = 1; i <= NUM_TEST_PROCS; i++ ) {
    g_proc_table[i].m_pid = g_test_procs[i-1].m_pid;
    g_proc_table[i].m_priority = g_test_procs[i-1].m_priority;
    g_proc_table[i].m_stack_size = g_test_procs[i-1].m_stack_size;
    g_proc_table[i].mpf_start_pc = g_test_procs[i-1].mpf_start_pc;
  }
	
	g_proc_table[PID_A].m_pid = PID_A;
  g_proc_table[PID_A].m_priority = LOW;
  g_proc_table[PID_A].m_stack_size = 0x100;
  g_proc_table[PID_A].mpf_start_pc = &procA;
	
	g_proc_table[PID_B].m_pid = PID_B;
  g_proc_table[PID_B].m_priority = LOW;
  g_proc_table[PID_B].m_stack_size = 0x100;
  g_proc_table[PID_B].mpf_start_pc = &procB;
	
	g_proc_table[PID_C].m_pid = PID_C;
  g_proc_table[PID_C].m_priority = LOW;
  g_proc_table[PID_C].m_stack_size = 0x100;
  g_proc_table[PID_C].mpf_start_pc = &procC;
	
	g_proc_table[PID_SET_PRIO].m_pid = PID_SET_PRIO;
  g_proc_table[PID_SET_PRIO].m_priority = LOW;
  g_proc_table[PID_SET_PRIO].m_stack_size = 0x100;
  g_proc_table[PID_SET_PRIO].mpf_start_pc = &setPriorityProc;
	
	g_proc_table[PID_CLOCK].m_pid = PID_CLOCK;
  g_proc_table[PID_CLOCK].m_priority = LOW;
  g_proc_table[PID_CLOCK].m_stack_size = 0x200;
  g_proc_table[PID_CLOCK].mpf_start_pc = &wallClockProc;
	
	g_proc_table[PID_KCD].m_pid = PID_KCD;
  g_proc_table[PID_KCD].m_priority = LOW;
  g_proc_table[PID_KCD].m_stack_size = 0x200;
  g_proc_table[PID_KCD].mpf_start_pc = &kcdProc;
	
	g_proc_table[PID_CRT].m_pid = PID_CRT;
  g_proc_table[PID_CRT].m_priority = LOW;
  g_proc_table[PID_CRT].m_stack_size = 0x200;
  g_proc_table[PID_CRT].mpf_start_pc = &crtProc;
	
	g_proc_table[PID_TIMER_IPROC].m_pid = PID_TIMER_IPROC;
  g_proc_table[PID_TIMER_IPROC].m_priority = LOW;
  g_proc_table[PID_TIMER_IPROC].m_stack_size = 0x200;
  g_proc_table[PID_TIMER_IPROC].mpf_start_pc = &TIMER0_IRQHandler;
	
	g_proc_table[PID_UART_IPROC].m_pid = PID_UART_IPROC;
  g_proc_table[PID_UART_IPROC].m_priority = LOW;
  g_proc_table[PID_UART_IPROC].m_stack_size = 0x200;
  g_proc_table[PID_UART_IPROC].mpf_start_pc = &UART0_IRQHandler;

  /* initialize exception stack frame (i.e. initial context) for each process */
  for ( i = 0; i < NUM_PROCS; i++ ) {
    int j;
    (gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
    (gp_pcbs[i])->m_state = NEW;
    (gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
    (gp_pcbs[i])->nextPCB = NULL;
		(gp_pcbs[i])->msgHead = NULL;
		(gp_pcbs[i])->msgTail = NULL;

    sp = alloc_stack((g_proc_table[i]).m_stack_size);
    *(--sp)  = INITIAL_xPSR;      // user process initial xPSR
    *(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
    for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
      *(--sp) = 0x0;
    }
    (gp_pcbs[i])->mp_sp = sp;
  }

  for ( i = 0; i < NUM_OF_PRIORITIES; i++ ) {
    ReadyPQ[i].head = NULL;
    ReadyPQ[i].tail = NULL;
    BlockPQ[i].head = NULL;
    BlockPQ[i].tail = NULL;
  }

  /* initialize priority queue */
  for ( i = 0; i <= PID_CRT; i++ ) {
    //insert PCB[i] into priority queue
#ifdef DEBUG_0
    printf("iValue 0x%x \n", gp_pcbs[i]);
#endif
    processEnqueue(ReadyPQ, gp_pcbs[i]);
  }
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
PCB *scheduler(void)
{
  if (gp_current_process != NULL) {
    if (gp_current_process->m_state == BLK){
      processEnqueue(BlockPQ, gp_current_process);
    }
    else if (gp_current_process->m_state == RUN) {
      processEnqueue(ReadyPQ, gp_current_process);
    }
  }
  return processDequeue(ReadyPQ);
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
      if (p_pcb_old->m_state != BLK && p_pcb_old->m_state != WAIT){
        p_pcb_old->m_state = RDY;
      }
      p_pcb_old->mp_sp = (U32 *) __get_MSP();
    }
    gp_current_process->m_state = RUN;
    __set_MSP((U32) gp_current_process->mp_sp);
    __rte();  // pop exception stack frame from the stack for a new processes
  }

  /* The following will only execute if the if block above is FALSE */
  if (gp_current_process != p_pcb_old) {
    if (state == RDY){
      if (p_pcb_old->m_state != BLK && p_pcb_old->m_state != WAIT){
        p_pcb_old->m_state = RDY;
      }
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
 * @brief glorified scheduler() and process_switch()
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
	__enable_irq();
  return RTX_OK;
}

/**
 * @brief moves pcb to its correct queue (for the case where a process changes another process' priority)
 */
void moveProcessToPriority(PCB* thePCB, int old_priority) {
  PCBQ* pq;
   if (thePCB->m_state == BLK) {
     pq = BlockPQ;
   }
   else {
    pq = ReadyPQ;
  }

  // remove from queue (many cases to consider)
  if (pq[old_priority].head == NULL) { // empty
    return; // error
  } else if (pq[old_priority].head == pq[old_priority].tail) { // 1 element
    if (thePCB != pq[old_priority].head) {
      return; // error
    }
    pq[old_priority].head = NULL;
    pq[old_priority].tail = NULL;
  } else if (pq[old_priority].head == thePCB) { // 1st element in LL with length > 1
    pq[old_priority].head = pq[old_priority].head->nextPCB;
  } else { // middle of the linked list
    PCB* current;
    for (current = pq[old_priority].head; current != NULL; current=current->nextPCB) {
      if (current->nextPCB == thePCB) {
        current->nextPCB = thePCB->nextPCB;

        if (thePCB == pq[old_priority].tail) {
          pq[old_priority].tail = current;
        }
        break;
      }
    }
  }

  thePCB->nextPCB = NULL;
  processEnqueue(pq, thePCB);
}

/**
 * @brief Sets process priority, then calls release_processor()
 * @return RTX_ERR on error and RTX_OK on success
 */
int k_set_process_priority(int process_id, int priority){
  int i;
  int old_priority;
  PCB* thePCB;
  if (HIGH <= priority && priority <= LOWEST) {
    for (i = 1; i <= PID_CRT; i++){
      thePCB = gp_pcbs[i];
      if (thePCB->m_pid == process_id){
        #ifdef DEBUG_0
        printf("Setting Process Priority: %d\n", priority);
        #endif /* DEBUG_0 */
        if (thePCB->m_priority != priority) {
          old_priority = thePCB->m_priority;
          thePCB->m_priority = priority;

          if (process_id != gp_current_process->m_pid) {
            moveProcessToPriority(thePCB, old_priority);
          }
          k_release_processor();
        }
        return RTX_OK;
      }
    }
  }

#ifdef DEBUG_0
  printf("Setting process priority failed.");
#endif /* DEBUG_0 */

  return RTX_ERR;
}

/**
 * @brief Gets process priority
 * @return process priority
 */
int k_get_process_priority(int process_id){
  int i;
  int priority = -1;
  for (i = 0; i < NUM_PROCS; i++){
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

/**
 * @brief: a process that
 *         yields the cpu.
 */
void nullProc(void)
{
  while (1) {
    k_release_processor();
  }
}

/**
 * @brief: a function that
 *         returns all pcbs in a state in an envelope.
 */
MSG_BUF* pcbs_in_state (int state){
	MSG_BUF* msg = (MSG_BUF *)k_request_memory_block_non_blocking();
	if (msg == NULL){
		return NULL;
	}
	msg->mtype = DEFAULT;
	
	char* str_end = copyStr("PID PRI\n\r", msg->mtext);
	int str_index = str_end - msg->mtext;
	
	for (int i = 0; i < NUM_PROCS; i++){
			if (gp_pcbs[i]->m_state == state){
				msg->mtext[str_index] = gp_pcbs[i]->m_pid / 10 + '0';
				str_index++;				
				msg->mtext[str_index] = gp_pcbs[i]->m_pid % 10 + '0';
				str_index++;

				msg->mtext[str_index] = ' ';
				str_index++;

				msg->mtext[str_index] = gp_pcbs[i]->m_priority + '0';
				str_index++;
				msg->mtext[str_index] = '\n';
				str_index++;
				msg->mtext[str_index] = '\r';
				str_index++;
			}
	}
	msg->mtext[str_index] = '\0';
	
	return msg;
}

