/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_memory.h"
#include "k_process.h"
#include "k_message.h"

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

int numBlockChange = 0;

/**
 * @brief: Initialize RAM as follows:

0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK           |
          |---------------------------|<--- gp_stack
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|
          |        PCB 6              |
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
          |        null PCB           |
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
  p_end += NUM_PROCS * sizeof(PCB *);

  for ( i = 0; i < NUM_PROCS; i++ ) {
    gp_pcbs[i] = (PCB *)p_end;
    p_end += sizeof(PCB);
  }

  /* 4 bytes padding */
  p_end += 4;

#ifdef DEBUG_0
  for (i = 0; i < NUM_PROCS; i++){
    printf("gp_pcbs[%d] = 0x%x \n", i, gp_pcbs[i]);
  }
#endif

  /* prepare for alloc_stack() to allocate memory for stacks */
  gp_stack = (U32 *)RAM_END_ADDR;
  if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
    --gp_stack;
  }
}

// maps the memory blocks in the heap so that each of them point to the next, and correspond to the defined BLOCK_SIZE
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

// pops an available memory block from the linked list of available memory blocks in the heap
void *k_request_memory_block(void) {
  MemBlock* prevHead;

#ifdef DEBUG_0
  printf("k_request_memory_block: entering...\n");
#endif /* ! DEBUG_0 */
  prevHead = memQueue.head;

  while (memQueue.head == NULL) {
    makeBlock();
  }

  if (memQueue.head == memQueue.tail) {
    memQueue.tail = NULL;
    memQueue.head = NULL;
  }
  else {
    memQueue.head = memQueue.head->next;
  }
	
  return (void *) ((envelope*) prevHead + 1);
}

void *k_request_memory_block_non_blocking(void) {
  MemBlock* prevHead;

#ifdef DEBUG_0
  printf("k_request_memory_block_non_blocking: entering...\n");
#endif /* ! DEBUG_0 */
  prevHead = memQueue.head;

  if (memQueue.head == NULL) {
    return NULL;
  }

  if (memQueue.head == memQueue.tail) {
    memQueue.tail = NULL;
    memQueue.head = NULL;
  }
  else {
    memQueue.head = memQueue.head->next;
  }
	
  return (void *) ((envelope*) prevHead + 1);
}

// adds the specified block back into the linked list of available memory blocks in the heap
int k_release_memory_block(void *p_mem_blk) {
  MemBlock * newTail;
	
	p_mem_blk = (void*) ((envelope*) p_mem_blk - 1);
	
#ifdef DEBUG_0
  printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
#endif /* ! DEBUG_0 */

	// check that the block is BLOCK_SIZE-aligned and between start and end of the PCBS and start of the stack
  if (!(p_end <= p_mem_blk && p_mem_blk < gp_stack && ((U32)p_mem_blk - (U32)p_end) % BLOCK_SIZE == 0)) {
		return RTX_ERR;
  }

  newTail = (MemBlock *) p_mem_blk;
  newTail->next = NULL;

  if (memQueue.tail != NULL) {
    memQueue.tail->next = newTail;
    memQueue.tail = newTail;
  } else {
    memQueue.head = newTail;
    memQueue.tail = newTail;
  }

  if (!blockPQIsEmpty()) {
    makeReady();
  }

  return RTX_OK;
}
