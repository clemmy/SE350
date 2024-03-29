/**
 * @file:   k_process.h
 * @brief:  process management hearder file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/01/17
 * NOTE: Assuming there are only two user processes in the system
 */

#ifndef K_PROCESS_H_
#define K_PROCESS_H_

#include "k_rtx.h"

/* ----- Definitions ----- */

#define INITIAL_xPSR 0x01000000        /* user process initial xPSR value */
#define NUM_OF_PRIORITIES 5


typedef struct PCBQ PCBQ;
struct PCBQ {
    PCB* head;
    PCB* tail;
};

/* ----- Global Variables ----- */
extern PCBQ ReadyPQ[NUM_OF_PRIORITIES];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];

/* ----- Functions ----- */

void process_init(void);               /* initialize all procs in the system */
PCB *scheduler(void);                  /* pick the pid of the next to run process */
int k_release_processor(void);           /* kernel release_process function */
int set_process_priority(int process_id, int priority); /* sets priority of this process to this priority */
int get_process_priority(int process_id);                /* returns the priority of the specified process. Returns -1 if failed */
void nullProc(void);
void processEnqueue(PCBQ pq[], PCB* thePCB);

extern U32 *alloc_stack(U32 size_b);   /* allocate stack for a process */
extern void __rte(void);               /* pop exception stack frame */
extern void set_test_procs(void);      /* test process initial set up */
extern void makeBlock(void);					/* Sets state of current process to blocked */
extern void makeReady(void);					/* Transfers the highest priority PCB from the block queue to the ready queue*/
extern int blockPQIsEmpty(void);
extern void timer_i_process(void);
extern void* k_request_memory_block_non_blocking( void );

#endif
