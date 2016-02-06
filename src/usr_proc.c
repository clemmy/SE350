/**
 * @file:   usr_proc.c
 * @brief:  Two user processes: proc1 and proc2
 * @author: Yiqing Huang
 * @date:   2014/02/28
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];

void set_test_procs() {
	int i;
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_priority=MEDIUM;
		g_test_procs[i].m_stack_size=0x400;
	}
  
	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[2].mpf_start_pc = &proc3;
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[4].mpf_start_pc = &proc5;
	g_test_procs[5].mpf_start_pc = &proc6;
}


/**
 * @brief: a process that prints 5x6 uppercase letters
 *         and then yields the cpu.
 */
void proc1(void)
{
	int i = 0;
	int ret_val = 10;
	int x = 0;
	
	
	//void* blocks[249];
	
// 	for (j = 0; ; j++) {
// 		curBlock = request_memory_block();
// 		if (curBlock == NULL) {
// 			break;
// 		}
// 		else if ((unsigned int)curBlock >= 0x10007000) {
// 			1+1;
// 		}
		
		//printf("%x\n", &blocks[j]);
		//blocks[j] = curBlock;
	//}
	
	int j;
	void* blocks[3];
	
	for (j=0;j<3;j++) {
		blocks[j] = request_memory_block();
	}
	
	for (j=2;j>=0;j--) {
		release_memory_block(blocks[j]);
	}

	// curBlock = request_memory_block();

	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart1_put_string(" 1\n\r");
			
			if ( i%30 == 0 ) {
				ret_val = release_processor();
#ifdef DEBUG_0
				printf("proc1: ret_val=%d\n", ret_val);
			
#endif /* DEBUG_0 */
			}
			for ( x = 0; x < 100000; x++); // some artifical delay
		}
		uart1_put_char('A' + i%26);
		i++;
		
	}
}

/**
 * @brief: a process that prints 5x6 numbers
 *         and then yields the cpu.
 */
void proc2(void)
{
	int i = 0;
	int ret_val = 20;
	int x = 0;
	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart1_put_string(" 2\n\r");
			
			if ( i%30 == 0 ) {
				ret_val = release_processor();
#ifdef DEBUG_0
				printf("proc2: ret_val=%d\n", ret_val);
			
#endif /* DEBUG_0 */
			}
			for ( x = 0; x < 100000; x++); // some artifical delay
		}
		uart1_put_char('0' + i%10);
		i++;
		
	}
}

/**
 * @brief: a process that prints five numbers in reverse order
 *         and then yields the cpu.
 */
void proc4(void)
{
	int i = 0;
	int ret_val = 40;
	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart1_put_string(" 4\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc4: ret_val=%d\n", ret_val);
			get_process_priority(4);
#endif /* DEBUG_0 */
		}
		uart1_put_char('9' - i%10);
		i++;
	}
}

/**
 * @brief: a process that prints three sixes
 *         and then yields the cpu.
 */
void proc5(void)
{
	int i = 0;
	int ret_val = 50;
	while ( 1) {
		if ( i != 0 && i%3 == 0 ) {
			uart1_put_string(" 5\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc5: ret_val=%d\n", ret_val);
			get_process_priority(5);
#endif /* DEBUG_0 */
		}
		uart1_put_char('6');
		i++;
	}
}

/**
 * @brief: This process is too cool to tell you what it does.
 */
void temp6(void)
{
	int i = 0;
	int ret_val = 50;
	while (1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 6\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc6: ret_val=%d\n", ret_val);
			get_process_priority(6);
#endif /* DEBUG_0 */
		}
		uart1_put_char('7');
		i++;
	}
}

/**
 * @brief: a process that sets its priority higher than the other processes and
 *         yields the cpu.
 */
void takeOver(void) //takeOver
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 3\n\r");
			set_process_priority(3, 0);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(3);
#endif /* DEBUG_0 */
		}
		uart1_put_char('a');
		i++;
	}
}

/**
 * @brief: a process that sets its priority lower than the other processes and
 *         yields the cpu.
 */
void remissive(void) //remissive
{
	int i = 0;
	int ret_val = 30;
	int gpp_ret_val = 0;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 3\n\r");
			set_process_priority(6, 2);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			gpp_ret_val = get_process_priority(6);
#endif /* DEBUG_0 */
		}
		uart1_put_char('a');
		i++;
	}
}

/**
 * @brief: a process that sets its own priority to itself
 */
void unchangeItself(void) //unchangeItself
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 3\n\r");
			set_process_priority(3, 1);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(3);
#endif /* DEBUG_0 */
		}
		uart1_put_char('a');
		i++;
	}
}

/**
 * @brief: a process that sets another process' priority higher than the other processes and
 *         yields the cpu.
 */
void giveTime(void) //giveTime
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 3\n\r");
			set_process_priority(2, 0);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(3);
#endif /* DEBUG_0 */
		}
		uart1_put_char('a');
		i++;
	}
}

/**
 * @brief: a process that sets its priority lower than the other processes and
 *         yields the cpu.
 */
void makeLame(void) //makeLame
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 3\n\r");
			set_process_priority(2, 2);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(3);
#endif /* DEBUG_0 */
		}
		uart1_put_char('a');
		i++;
	}
}

/**
 * @brief: a process that sets its own priority to itself
 */
void unchangeOther(void) //unchangeOther
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string("\n\r");
			set_process_priority(3, 1);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(3);
#endif /* DEBUG_0 */
		}
		uart1_put_char('a');
		i++;
	}
}


/**
 * @brief: a process that eats all memory.
 */
void memoryHog(void) //memoryHog
{
	while ( 1) {
		uart1_put_string("Eating memory!\n\r");
		request_memory_block();
	}
}

/**
 * @brief: a process that sets its priority lower than the other processes and
 *         yields the cpu.
 */
void twinA(void) //twinA //proc3
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 3\n\r");
			set_process_priority(4, 0);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(3);
#endif /* DEBUG_0 */
		}
		uart1_put_char('a');
		i++;
	}
}

/**
 * @brief: a process that sets its priority lower than the other processes and
 *         yields the cpu.
 */
void twinB(void) //twinB //proc4
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 4\n\r");
			set_process_priority(3, 0);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(4);
#endif /* DEBUG_0 */
		}
		uart1_put_char('b');
		i++;
	}
}

/**
 * @brief: a process that sets all processes to the highest priority and
 *         yields the cpu.
 */
void allUp(void) //allUp //proc3
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 3\n\r");
			set_process_priority(3, 0);
			set_process_priority(1, 0);
			set_process_priority(2, 0);
			set_process_priority(4, 0);
			set_process_priority(5, 0);
			set_process_priority(6, 0);
			set_process_priority(0, 0);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(3);
#endif /* DEBUG_0 */
		}
		uart1_put_char('b');
		i++;
	}
}

/**
 * @brief: a process that sets all processes to the lowest priority and
 *         yields the cpu.
 */
void allDown(void) //allDown //proc3
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 3\n\r");
			set_process_priority(6, 3);
			set_process_priority(1, 3);
			set_process_priority(2, 3);
			set_process_priority(4, 3);
			set_process_priority(5, 3);
			set_process_priority(0, 3);
			set_process_priority(3, 3);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(3);
#endif /* DEBUG_0 */
		}
		uart1_put_char('b');
		i++;
	}
}

/**
 * @brief: a process that sets all processes to the highest priority and
 *         yields the cpu.
 */
void testGPP(void) //testGPP //proc3
{
	int i = 0;
	int ret_val = 30;
	int gpp = 10;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 3\n\r");
			gpp = get_process_priority(3);
			set_process_priority(3, 0);
			gpp = get_process_priority(3);
			gpp = get_process_priority(1);
			set_process_priority(1, 0);
			gpp = get_process_priority(1);
			set_process_priority(2, 0);
			set_process_priority(4, 0);
			set_process_priority(5, 0);
			set_process_priority(6, 0);
			set_process_priority(0, 0);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(3);
#endif /* DEBUG_0 */
		}
		uart1_put_char('b');
		i++;
		if (gpp == 0){
			set_process_priority(3, 0);
		}
	}
}

/**
 * @brief: a process that eats all memory.
 */
void memoryHog1(void) //memoryHog1 //proc1
{
	while ( 1) {
		uart1_put_string("Eating memory!\n\r");
		request_memory_block();
	}
}

/**
 * @brief: a process that eats all memory.
 */
void memoryHog2(void) //memoryHog2 //proc2
{
	while ( 1) {
		uart1_put_string("Eating memory!\n\r");
		request_memory_block();
	}
}

/**
 * @brief: a process that eats all memory.
 */
void memoryHog3(void) //memoryHog3 //proc3
{
	while ( 1) {
		uart1_put_string("Eating memory!\n\r");
		request_memory_block();
	}
}

/**
 * @brief: a process that eats all memory.
 */
void memoryHog4(void) //memoryHog4 //proc4
{
	while ( 1) {
		uart1_put_string("Eating memory!\n\r");
		request_memory_block();
	}
}

/**
 * @brief: a process that eats all memory.
 */
void memoryHog5(void) //memoryHog5 //proc5
{
	while ( 1) {
		uart1_put_string("Eating memory!\n\r");
		request_memory_block();
	}
}

/**
 * @brief: a process that eats all memory.
 */
void proc6(void) //memoryHog6 //proc6
{
	while ( 1) {
		uart1_put_string("Eating memory!\n\r");
		request_memory_block();
	}
}

/**
 * @brief: a process that eats all memory.
 */
void memoryNice(void) //memoryNice //proc3
{
	int i;
	void* memBlocks[5];
	
	for (i = 0; i < 5; i++) {
		uart1_put_string("Eating little memory!\n\r");
		memBlocks[i] = request_memory_block();
	}
	
	
	for (i = 0; i < 5; i++) {
		release_processor();
		uart1_put_string("Releasing little memory!\n\r");
		release_memory_block(memBlocks[i]);
	}
	
	while ( 1) {
		release_processor();
	}
}

/**
 * @brief: a process that eats all memory.
 */
void eatPukeEat(void) //eatPukeEat //proc3
{
	int i;
	const int numOfBlocks = 197;
	void* memBlocks[numOfBlocks];
	
	for (i = 0; i < numOfBlocks; i++) {
		printf("Ate %d blocks\n\r", i);
		memBlocks[i] = request_memory_block();
	}
	
	for (i = 0; i < numOfBlocks; i++) {
		printf("Spat out %d blocks\n\r", i);
		release_memory_block(memBlocks[i]);
	}
	
	for (i = 0; i < numOfBlocks; i++) {
		printf("Ate %d blocks again\n\r", i);
		memBlocks[i] = request_memory_block();
	}
	
	while ( 1) {
		release_processor();
	}
}

/**
 * @brief: a process that sets priority of proc4 higher then
 *         yields the cpu.
 */
void boostFour(void) //boostFour //proc6
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 6\n\r");
			set_process_priority(4, 0);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(6);
#endif /* DEBUG_0 */
		}
		uart1_put_char('a');
		i++;
	}
}

/**
 * @brief: a process that sets priority of proc4 higher then
 *         yields the cpu.
 */
void boostThree(void) //boostThree //proc6
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 6\n\r");
			set_process_priority(3, 0);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(6);
#endif /* DEBUG_0 */
		}
		uart1_put_char('a');
		i++;
	}
}

/**
 * @brief: a process that sets priority of proc4 higher then
 *         yields the cpu.
 */
void boostFive(void) //boostFive //proc6
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart1_put_string(" 6\n\r");
			set_process_priority(5, 0);
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(6);
#endif /* DEBUG_0 */
		}
		uart1_put_char('a');
		i++;
	}
}

/**
 * @brief: a process that eats up all the memory, then it releases processor.
 * Then on the next run, it releases a bit of memory so that a blocked process 
 * is put on the ready q. Then it requests that memory back, and releases processor.
 */
void proc3(void) //danglingCarrot //proc3
{
	void* memoryBlock = request_memory_block();
	release_processor();
	set_process_priority(6, 3);
	release_memory_block(memoryBlock);
	//memoryBlock = request_memory_block();
	set_process_priority(6, 0);
	
	set_process_priority(6, 3);
	release_memory_block(memoryBlock);
	memoryBlock = request_memory_block();
	set_process_priority(6, 0);
	
	while(1) {
		release_processor();
	}
}

