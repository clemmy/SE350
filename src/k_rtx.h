/**
 * @file:   k_rtx.h
 * @brief:  kernel deinitiation and data structure header file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */

#ifndef K_RTX_H_
#define K_RTX_H_

// #include "k_message.h"

/*----- Definitations -----*/

#define RTX_ERR -1
#define RTX_OK  0

#define NULL 0
#define NUM_TEST_PROCS 6
#define NUM_PROCS 16

/* Process IDs */
#define PID_NULL 0
#define PID_P1   1
#define PID_P2   2
#define PID_P3   3
#define PID_P4   4
#define PID_P5   5
#define PID_P6   6
#define PID_A    7
#define PID_B    8
#define PID_C    9
#define PID_SET_PRIO     10
#define PID_CLOCK        11
#define PID_KCD          12
#define PID_CRT          13
#define PID_TIMER_IPROC  14
#define PID_UART_IPROC   15

#define HIGH    0
#define MEDIUM  1
#define LOW     2
#define LOWEST  3
#define NULL_PRIORITY 4

/* Message Types */
#define DEFAULT 0
#define KCD_REG 1

#ifdef DEBUG_0
#define USR_SZ_STACK 0x200         /* user proc stack size 512B   */
#else
#define USR_SZ_STACK 0x100         /* user proc stack size 218B  */
#endif /* DEBUG_0 */

/*----- Types -----*/
typedef unsigned char U8;
typedef unsigned int U32;

/*---- Forward Declarations ----*/
typedef struct _envelope envelope;

/* process states, note we only assume three states in this example */
//BLK means that the process is blocked on memory.
//WAIT means that the process is waiting for a message.
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
