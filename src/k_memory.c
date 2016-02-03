/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_memory.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

/* ----- Global Variables ----- */
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address */
	       /* stack grows down. Fully decremental stack */
U8 *p_end;
MemQueue memQueue;
unsigned int numOfBlocks;


/**
 * @brief: Initialize RAM as follows:

0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK                                 |
          |---------------------------|<--- gp_stack
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|
					|        PQueueLast[3]      |
          |---------------------------|
					|        PQueueLast[2]      |
          |---------------------------|
					|        PQueueLast[1]      |
          |---------------------------|
					|        PQueueLast[0]      |
          |---------------------------|
					|        PQueueFirst[3]     |
          |---------------------------|
					|        PQueueFirst[2]     |
          |---------------------------|
					|        PQueueFirst[1]     |
          |---------------------------|
					|        PQueueFirst[0]     |
          |---------------------------|
					|        null PCB           |
          |---------------------------|
					|        PCB 5              |
          |---------------------------|
					|        PCB 4              |
          |---------------------------|
					|        PCB 3              |
          |---------------------------|
          |        PCB 2              |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB pointers       |
          |---------------------------|<--- gp_pcbs
          |        Padding            |
          |---------------------------|  
          |Image$$RW_IRAM1$$ZI$$Limit |
          |...........................|          
          |       RTX  Image          |
          |                           |
0x10000000+---------------------------+ Low Address

*/

void memory_init(void)
{
	int i;
	p_end = (U8 *)&Image$$RW_IRAM1$$ZI$$Limit;
  
	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	gp_pcbs = (PCB **)p_end;
	p_end += NUM_TEST_PROCS * sizeof(PCB *);
  
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		
		p_end += sizeof(PCB); 
	}
	
	PQueueFirst = (PCB ***)p_end;
	for ( i = 0; i < 4; i++ ) {
		PQueueFirst[i] = (PCB **)p_end;
		p_end += sizeof(PCB*);
	}
	
	PQueueLast = (PCB ***)p_end;
	for ( i = 0; i < 4; i++ ) {
		PQueueLast[i] = (PCB **)p_end;
		p_end += sizeof(PCB*);
	}
	
	BlockedQueueFirst = (PCB **)p_end;
	p_end += sizeof(PCB*);
	
	BlockedQueueLast = (PCB **)p_end;
	p_end += sizeof(PCB*);
#ifdef DEBUG_0  
	printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n", gp_pcbs[1]);
	printf("gp_pcbs[2] = 0x%x \n", gp_pcbs[2]);
	printf("gp_pcbs[3] = 0x%x \n", gp_pcbs[3]);
	printf("gp_pcbs[4] = 0x%x \n", gp_pcbs[4]);
	printf("gp_pcbs[5] = 0x%x \n", gp_pcbs[5]);
#endif
	
	/* prepare for alloc_stack() to allocate memory for stacks */
	
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack; 
	}
}

void heap_init() {
	U32 block_head;
	MemBlock* memBlock;
	MemBlock* lastBlock;
	
	memQueue.head = (MemBlock*) p_end;
	numOfBlocks = 0;
	for (block_head = (U32) memQueue.head; block_head + BLOCK_SIZE < (U32) gp_stack; block_head += BLOCK_SIZE) {
			memBlock = (MemBlock*) block_head;
			memBlock->next = (MemBlock*) (block_head + BLOCK_SIZE);
			numOfBlocks++;
	}
	lastBlock = (MemBlock*) (block_head - BLOCK_SIZE);
	lastBlock->next = NULL;
	
	memQueue.tail = lastBlock;
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

U32 *alloc_stack(U32 size_b) 
{
	U32 *sp;
	sp = gp_stack; /* gp_stack is always 8 bytes aligned */
	
	/* update gp_stack */
	gp_stack = (U32 *)((U8 *)sp - size_b);
	
	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack; 
	}
	return sp;
}

void *k_request_memory_block(void) {
	MemBlock* prevHead;
	
	
#ifdef DEBUG_0 
	printf("k_request_memory_block: entering...\n");
#endif /* ! DEBUG_0 */
	prevHead = memQueue.head;
	
	if (memQueue.head == NULL) {
		prevHead = NULL;
	} else if (memQueue.head == memQueue.tail) {
		memQueue.tail = NULL;
		memQueue.head = NULL;
	} 
	else {
		memQueue.head = memQueue.head->next;
	}
	
	
	return (void *) prevHead; // NULL if there is no memory left
}

int k_release_memory_block(void *p_mem_blk) {
	MemBlock * newTail;
#ifdef DEBUG_0 
	printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
#endif /* ! DEBUG_0 */
	
	newTail = (MemBlock *) p_mem_blk;
	newTail->next = NULL;
	
	if (memQueue.tail != NULL) {
		memQueue.tail->next = newTail;	
		memQueue.tail = newTail;
	} else {
		memQueue.head = newTail;
		memQueue.tail = newTail;
	}
	
	return RTX_OK;
}
