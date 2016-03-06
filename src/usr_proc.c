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

int testsRan;
int testsPassed;
int totalTests;

int ready;

void set_test_procs() {
	int i;
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_priority=MEDIUM;
		g_test_procs[i].m_stack_size=0x100;
	}
  
	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[2].mpf_start_pc = &proc3;
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[4].mpf_start_pc = &proc5;
	g_test_procs[5].mpf_start_pc = &proc6;
	
	testsRan = 0;
	testsPassed = 0;
	totalTests = 3;
	ready = 0;
}

// ONLY WORKS IF 0 <= testNumber < 10
void printTestStatus(int testNumber, int pass){ 
	uart1_put_string("G030_test: test ");
	uart1_put_char(testNumber + '0');
	if (pass){
		uart1_put_string(" OK\n\r");
		testsPassed++;
	}
	else{
		uart1_put_string(" FAIL\n\r");
	}
	testsRan++;
}

/**
 * @brief: a process that prints results and yields the CPU if not done
 *         
 */
void proc1(void){
	
	uart1_put_string("G030_test: START\n\r");
	
	uart1_put_string("G030_test: total ");
	uart1_put_char(totalTests + '0');
	uart1_put_string(" tests\n\r");
	
	while (testsRan < totalTests){
		ready = 1;
		release_processor();
	}
	
	uart1_put_string("G030_test: ");
	uart1_put_char(testsPassed + '0');
	uart1_put_char('/');
	uart1_put_char(totalTests + '0');
	uart1_put_string(" tests OK\n\r");
	
	uart1_put_string("G030_test: ");
	uart1_put_char(totalTests - testsPassed + '0');
	uart1_put_char('/');
	uart1_put_char(totalTests + '0');
	uart1_put_string(" tests FAIL\n\r");
	
	uart1_put_string("G030_test: END\n\r");
	while (1);
}

void proc2(void) {
	
	while (!ready){
		release_processor();
	}
	
	int sender;
	MSG_BUF* msg = (MSG_BUF*) request_memory_block(); 
	msg->mtype = DEFAULT;
	msg->mtext[0] = 'h';
	msg->mtext[1] = 'i';
	msg->mtext[2] = '\0';
	
	send_message(3, msg);
	msg = receive_message(&sender);
	
	if (sender != 3 || msg->mtext[0] != 'b' || msg->mtext[1] != 'y' || msg->mtext[2] != 'e' || msg->mtext[3] != '\0'){
		printTestStatus(1, 0);
	}
	else {
		printTestStatus(1, 1);
	}
	
	release_memory_block(msg);
	
	while(1) {
		release_processor();
	}
}
void proc3(void) {
	
	while (!ready){
		release_processor();
	}
	
	int sender;
	MSG_BUF* msg = (MSG_BUF*) receive_message(&sender);
	
	if (sender != 2 || msg->mtext[0] != 'h' || msg->mtext[1] != 'i' || msg->mtext[2] != '\0'){
		printTestStatus(1, 0);
		release_memory_block(msg);
	}
	
	while (testsRan > 0) {
		release_processor();
	}
	
	msg->mtext[0] = 'b';
	msg->mtext[1] = 'y';
	msg->mtext[2] = 'e';
	msg->mtext[3] = '\0';
	
	send_message(2, msg);
	
	while (1) {
		release_processor();
	}
}

void proc4(void) {
	while (testsRan < 1 || !ready){
		release_processor();
	}
	
	int sender;
	
	MSG_BUF* msg = (MSG_BUF*) request_memory_block(); 
	msg->mtype = DEFAULT;
	msg->mtext[0] = '2';
	msg->mtext[1] = '0';
	msg->mtext[2] = '0';
	msg->mtext[3] = '\0';
	
	MSG_BUF* msg2 = (MSG_BUF*) request_memory_block(); 
	msg2->mtype = DEFAULT;
	msg2->mtext[0] = '1';
	msg2->mtext[1] = '0';
	msg2->mtext[2] = '0';
	msg2->mtext[3] = '\0';
	
	MSG_BUF* msg3 = (MSG_BUF*) request_memory_block(); 
	msg3->mtype = DEFAULT;
	msg3->mtext[0] = '5';
	msg3->mtext[1] = '0';
	msg3->mtext[2] = '\0';
	
	MSG_BUF* msg4 = (MSG_BUF*) request_memory_block(); 
	msg4->mtype = DEFAULT;
	msg4->mtext[0] = '2';
	msg4->mtext[1] = '0';
	msg4->mtext[2] = '1';
	msg4->mtext[3] = '\0';
	
	delayed_send(4, msg, 200);
	delayed_send(4, msg2, 100);
	delayed_send(4, msg3, 50);
	delayed_send(4, msg4, 201);
	
	MSG_BUF* rec = receive_message(&sender);
	MSG_BUF* rec2 = receive_message(&sender);
	MSG_BUF* rec3 = receive_message(&sender);
	MSG_BUF* rec4 = receive_message(&sender);
	
	if (rec->mtext[0] != '5' || rec->mtext[1] != '0'){
		printTestStatus(2, 0);
	}
	else if (rec2->mtext[0] != '1' || rec2->mtext[1] != '0' || rec2->mtext[2] != '0'){
		printTestStatus(2, 0);
	}
	else if (rec3->mtext[0] != '2' || rec3->mtext[1] != '0' || rec3->mtext[2] != '0'){
		printTestStatus(2, 0);
	}
	else if (rec4->mtext[0] != '2' || rec4->mtext[1] != '0' || rec4->mtext[2] != '1'){
		printTestStatus(2, 0);
	}
	else {
		printTestStatus(2, 1);
	}
	
	release_memory_block(rec);
	release_memory_block(rec2);
	release_memory_block(rec3);
	release_memory_block(rec4);
	
	while (1) {
		release_processor();
	}
}

void proc5(void) {
	while (testsRan < 2 || !ready){
		release_processor();
	}
	
	MSG_BUF* msg = (MSG_BUF*) request_memory_block(); 
	msg->mtype = KCD_REG;
	msg->mtext[0] = '%';
	msg->mtext[1] = 'A';
	msg->mtext[2] = '\0';
	send_message(PID_KCD, msg);
	
	MSG_BUF* msg2 = (MSG_BUF*) request_memory_block(); 
	msg2->mtype = DEFAULT;
	msg2->mtext[0] = '%';
	msg2->mtext[1] = 'A';
	msg2->mtext[2] = ' ';
	msg2->mtext[3] = 'A';
	msg2->mtext[4] = 'B';
	msg2->mtext[5] = 'C';
	msg2->mtext[6] = 'D';
	msg2->mtext[7] = '\0';
	send_message(PID_KCD, msg2);
	
	int sender;
	MSG_BUF* msg3 = (MSG_BUF*) receive_message(&sender);
	
	if (msg3->mtext[0] != '%' || msg3->mtext[1] != 'A' || msg3->mtext[2] != ' ' || msg3->mtext[3] != 'A'){
		printTestStatus(3, 0);
	}
	else if (msg3->mtext[4] != 'B' || msg3->mtext[5] != 'C' || msg3->mtext[6] != 'D' || msg3->mtext[7] != '\0'){
		printTestStatus(3, 0);
	}
	else{
		printTestStatus(3, 1);
	}
	
	release_memory_block(msg3);
	
	while (1) {		
		release_processor();
	}
}

void proc6(void){
	while(1){
		release_processor();
	}
}

void blegh(void) {
	int ignore;
	
	MSG_BUF* msg = (MSG_BUF*)receive_message(&ignore);
	//uart1_put_string("pid:");
	//uart1_put_char(ignore+'0');
	send_message(PID_CRT, msg);
	
	MSG_BUF* msg2 = (MSG_BUF*)receive_message(&ignore);
	//uart1_put_string("pid:");
	//uart1_put_char(ignore+'0');
	send_message(PID_CRT, msg2);
	
	MSG_BUF* msg3 = (MSG_BUF*)receive_message(&ignore);
	send_message(PID_CRT, msg3);
	
	MSG_BUF* msg4 = (MSG_BUF*)receive_message(&ignore);
	send_message(PID_CRT, msg4);
	
	while (1) {
		//uart1_put_string("SENT");
		release_processor();
	}
}

void procA(void) {
	while (1) {
		release_processor();
	}
}

void procB(void) {
	while (1) {
		release_processor();
	}
}

void procC(void) {
	while (1) {
		release_processor();
	}
}

// /**
//  * @brief: a process that prints results and yields the CPU if not done
//  *         
//  */
// void proc1(void){
// 	uart1_put_string("G030_test: START\n\r");
// 	
// 	uart1_put_string("G030_test: total ");
// 	uart1_put_char(totalTests + '0');
// 	uart1_put_string(" tests\n\r");
// 	
// 	while (testsRan < totalTests){
// 		release_processor();
// 	}
// 	
// 	uart1_put_string("G030_test: ");
// 	uart1_put_char(testsPassed + '0');
// 	uart1_put_char('/');
// 	uart1_put_char(totalTests + '0');
// 	uart1_put_string(" tests OK\n\r");
// 	
// 	uart1_put_string("G030_test: ");
// 	uart1_put_char(totalTests - testsPassed + '0');
// 	uart1_put_char('/');
// 	uart1_put_char(totalTests + '0');
// 	uart1_put_string(" tests FAIL\n\r");
// 	
// 	uart1_put_string("G030_test: END\n\r");
// 	while (1);
// }

// //ADD CHECKS FOR TESTSPASSED IN OTHER PROCESSES
// //SET INCORRECTRUN IN OTHER PROCESSES IF THEY AREN'T SUPPOSED TO RUN

// /**
//  * @brief: Checks if get_process_priority succeeds.
//  *         
//  */
// void proc2(void)
// {
// 	int i = 0;
// 	int gpp = 10;
// 	int pass = 1;
// 			
// 	gpp = get_process_priority(2);
// 	if (gpp != 1){
// 		pass = 0;
// 	}
// 	set_process_priority(2, HIGH);
// 	gpp = get_process_priority(2);
// 	if (gpp != 0){
// 		pass = 0;
// 	}
// 	
// 	gpp = get_process_priority(3);
// 	if (gpp != 1){
// 		pass = 0;
// 	}
// 	set_process_priority(3, HIGH);
// 	gpp = get_process_priority(3);
// 	if (gpp != 0){
// 		pass = 0;
// 	}
// 	
// 	printTestStatus(1, pass);
// 	
// 	for (i = 6; i >= 1; i--){
// 		set_process_priority(i, MEDIUM);
// 	}
// 	
// 	while(1){
// 		incorrectRun = 1;
// 		release_processor();
// 	}
// }

// /**
//  * @brief: Tests if setting the priority of this process higher than
//  *         other processes prevents them from running
//  */
// void proc3(void)
// {
// 	int i;
// 	
// 	while (testsRan < 1){
// 		release_processor();
// 	}
// 	
// 	incorrectRun = 0;
// 	
// 	set_process_priority(3, HIGH);
// 	for (i = 0; i < 5; i++){
// 		release_processor();
// 	}
// 	
// 	printTestStatus(2, !incorrectRun);
// 	
// 	set_process_priority(3, MEDIUM);
// 	
// 	while (1){
// 		release_processor();
// 	}
// }

// /**
//  * @brief: Allocates some memory, writes some data, then frees the memory
//  *         
//  */
// void proc4(void)
// {
// 	int j;
// 	int pass = 1;
// 	void* blocks[3];
// 	
// 	while (testsRan < 2){
// 		incorrectRun = 1;
// 		release_processor();
// 	}
// 	
// 	for (j=0;j<3;j++) {
// 		blocks[j] = request_memory_block();
// 		// sets the memory at address blocks[j] + 4 to 9000 + j
// 		*(((U32*)blocks[j]) + 4) = 9000 + j;
// 	}
// 	
// 	for (j=2;j>=0;j--) {
// 		if (*(((U32*)blocks[j]) + 4) != 9000 + j) {
// 			pass = 0;
// 		}
// 		release_memory_block(blocks[j]);
// 	}
// 	
// 	printTestStatus(3, pass);
// 	
// 	while (1){
// 		release_processor();
// 	}
// }

// /**
//  * @brief: requests one block of memory, releases processor, then releases
//  * that block of memory.  
//  */
// void proc5(void)
// {
// 	void* memBlk;
// 	
// 	while (testsRan < 3){
// 		incorrectRun = 1;
// 		release_processor();
// 	}

// 	memBlk = request_memory_block();
// 	
// 	release_processor();
// 	
// 	release_memory_block(memBlk);
// 	
// 	printTestStatus(4, 1);
// 	
// 	while (1){
// 		release_processor();
// 	}
// }

// /**
//  * @brief: Uses up all memory
//  *         
//  */
// void proc6(void)
// {
// 	void* memBlk;
// 	
// 	while (testsRan < 3){
// 		incorrectRun = 1;
// 		release_processor();
// 	}

// 	while (1) {
// 		memBlk = request_memory_block();
// 	}
// }
