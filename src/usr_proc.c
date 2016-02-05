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
		g_test_procs[i].m_stack_size=0x100;
	}
	
	g_test_procs[5].m_priority=LOWEST; //Null Process has LOWEST priority
  
	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[2].mpf_start_pc = &proc3;
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[4].mpf_start_pc = &proc5;
	g_test_procs[5].mpf_start_pc = &nullProc;
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
// 		curBlock = k_request_memory_block();
// 		if (curBlock == NULL) {
// 			break;
// 		}
// 		else if ((unsigned int)curBlock >= 0x10007000) {
// 			1+1;
// 		}
		
		//printf("%x\n", &blocks[j]);
		//blocks[j] = curBlock;
	//}
	
/*	int j;
	void* curBlock;
	void* blocks[3];
	
	for (j=0;j<3;j++) {
		blocks[j] = k_request_memory_block();
	}
	
	for (j=2;j>=0;j--) {
		k_release_memory_block(blocks[j]);
	}
*/
	// curBlock = k_request_memory_block();

	while ( 1) {
		if ( i != 0 && i%5 == 0 ) {
			uart1_put_string("\n\r");
			
			if ( i%30 == 0 ) {
				ret_val = release_processor();
#ifdef DEBUG_0
				printf("proc1: ret_val=%d\n", ret_val);
			
#endif /* DEBUG_0 */
			}
			for ( x = 0; x < 500000; x++); // some artifical delay
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
			uart1_put_string("\n\r");
			
			if ( i%30 == 0 ) {
				ret_val = release_processor();
#ifdef DEBUG_0
				printf("proc2: ret_val=%d\n", ret_val);
			
#endif /* DEBUG_0 */
			}
			for ( x = 0; x < 500000; x++); // some artifical delay
		}
		uart1_put_char('0' + i%10);
		i++;
		
	}
}

/**
 * @brief: a process that prints four a's (setting a new priority each time if debugging)
 *         and then yields the cpu.
 */
void proc3(void)
{
	int i = 0;
	int ret_val = 30;
	while ( 1) {
		if ( i != 0 && i%4 == 0 ) {
			uart0_put_string("\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc3: ret_val=%d\n", ret_val);
			get_process_priority(3);
#endif /* DEBUG_0 */
		}
#ifdef DEBUG_0
		//set_process_priority(3, 0);
#endif /* DEBUG_0 */
		uart0_put_char('a');
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
			uart0_put_string("\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc4: ret_val=%d\n", ret_val);
			get_process_priority(4);
#endif /* DEBUG_0 */
		}
		uart0_put_char('9' - i%10);
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
			uart0_put_string("\n\r");
			ret_val = release_processor();
#ifdef DEBUG_0
			printf("proc5: ret_val=%d\n", ret_val);
			get_process_priority(5);
#endif /* DEBUG_0 */
		}
		uart0_put_char('6');
		i++;
	}
}

/**
 * @brief: a process that
 *         yields the cpu.
 */
void nullProc(void)
{
	int ret_val = 666;
	while ( 1) {
#ifdef DEBUG_0
			printf("nullProc: ret_val=%d\n", ret_val);
			get_process_priority(6);
#endif /* DEBUG_0 */
			ret_val = release_processor();
	}

}
