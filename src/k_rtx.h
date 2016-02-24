/**
 * @file:   k_rtx.h
 * @brief:  kernel deinitiation and data structure header file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */

#ifndef K_RTX_H_
#define K_RTX_H_

#include "k_message.h";

/*----- Definitations -----*/

#define RTX_ERR -1
#define RTX_OK  0

#define NULL 0
#define NUM_TEST_PROCS 6
#define NUM_PROCS (NUM_TEST_PROCS + 1)

#define NULL_PRIORITY 4

#ifdef DEBUG_0
#define USR_SZ_STACK 0x200         /* user proc stack size 512B   */
#else
#define USR_SZ_STACK 0x100         /* user proc stack size 218B  */
#endif /* DEBUG_0 */

/*----- Types -----*/
typedef unsigned char U8;
typedef unsigned int U32;

/* process states, note we only assume three states in this example */
typedef enum {NEW = 0, RDY, RUN, BLK, WAIT} PROC_STATE_E;

/*
  PCB data structure definition.
  You may want to add your own member variables
  in order to finish P1 and the entire project
*/
typedef struct pcb PCB;

struct pcb
{
  U32 *mp_sp;    /* stack pointer of the process */
  U32 m_pid;    /* process id */
  int m_priority; /* process priority */
  PROC_STATE_E m_state;   /* state of the process */
  PCB* nextPCB; /* pointer to next PCB, if PCB is in a queue */
	envelope* msgHead;
	envelope* msgTail;
};

/* initialization table item */
typedef struct proc_init
{
  int m_pid;          /* process id */
  int m_priority;         /* initial priority, not used in this example. */
  int m_stack_size;       /* size of stack in words */
  void (*mpf_start_pc) ();/* entry point of the process */
} PROC_INIT;

#endif // ! K_RTX_H_
