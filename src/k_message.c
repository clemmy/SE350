#include "k_message.h"
#include "k_rtx.h"
#include "k_process.h"
#include "timer.h"

int k_send_message(int process_id, void* message_envelope) {
	
	PCB* thePCB;
	envelope* env;
	
	env = (envelope*) message_envelope - 1;
	env->next = NULL;
	env->sender_id = gp_current_process->m_pid;
	env->recv_id = process_id;
	
	thePCB = gp_pcbs[env->recv_id];
	
	if (thePCB->msgTail == NULL) {
		thePCB->msgHead = env;
		thePCB->msgTail = env;
	}
	else {
		thePCB->msgTail->next = env;
		thePCB->msgTail = env;
	}
	
	// preemption if blocked on receive and high priority
	// set to ready if not blocked on memory
	
	if (thePCB->m_state == WAIT){
		processEnqueue(ReadyPQ, thePCB);
		thePCB->m_state = RDY;
		if (thePCB->m_priority < gp_current_process->m_priority){
			k_release_processor();
		}
	}
	
	return RTX_OK;
	
}

void* k_receive_message(int* sender_id) {
	PCB* thePCB = gp_current_process;
	while (thePCB->msgHead == NULL) {
		thePCB->m_state = WAIT;
		k_release_processor();
	}
	
	envelope* envelope = thePCB->msgHead;
	
	if (thePCB->msgHead == thePCB->msgTail) {
    thePCB->msgHead = NULL;
    thePCB->msgTail = NULL;
  }
  else {
    thePCB->msgHead = thePCB->msgHead->next;
  }
	*sender_id = envelope->sender_id;
	return (void*) (envelope + 1);
}

int timer_send_message(envelope* env) {
	
	PCB* thePCB;
	
	thePCB = gp_pcbs[env->recv_id];
	
	if (thePCB->msgTail == NULL) {
		thePCB->msgHead = env;
		thePCB->msgTail = env;
	}
	else {
		thePCB->msgTail->next = env;
		thePCB->msgTail = env;
	}
	
	if (thePCB->m_state == WAIT){
		processEnqueue(ReadyPQ, thePCB);
		thePCB->m_state = RDY;
	}
	
	return RTX_OK;
	
}

envelope* timer_receive_message() {
	envelope* envelope = TIMER_PCB->msgHead;
	
	if (envelope == NULL) {
		return NULL;
	}
	
	if (TIMER_PCB->msgHead == TIMER_PCB->msgTail) {
    TIMER_PCB->msgHead = NULL;
    TIMER_PCB->msgTail = NULL;
  }
  else {
    TIMER_PCB->msgHead = TIMER_PCB->msgHead->next;
  }

	return envelope;
}

/*int k_has_message(int process_id) {
	PCB* thePCB = gp_pcbs[process_id];;
	if (thePCB->msgHead == NULL) {
		return 0;
	}
	
	return 1;
}*/

int k_delayed_send(int process_id, void* message_envelope, int delay) {
	
	envelope* env;
	
	env = (envelope*) message_envelope - 1;
	env->next = NULL;
	env->sender_id = gp_current_process->m_pid;
	env->recv_id = process_id;
	env->send_time = get_time() + delay;
	
	if (TIMER_PCB->msgTail == NULL) {
		TIMER_PCB->msgHead = env;
		TIMER_PCB->msgTail = env;
	}
	else {
		TIMER_PCB->msgTail->next = env;
		TIMER_PCB->msgTail = env;
	}
	//No need for pre-emption
	return RTX_OK;
}
