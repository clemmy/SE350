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

int k_send_message_non_preempt(int process_id, void* message_envelope) {
	
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
	
	// set to ready if not blocked on memory
	
	if (thePCB->m_state == WAIT){
		processEnqueue(ReadyPQ, thePCB);
		thePCB->m_state = RDY;
	}
	
	return RTX_OK;
	
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

envelope* k_receive_message_non_blocking(int proc_id) {
	envelope* envelope = gp_pcbs[proc_id]->msgHead;
	
	if (envelope == NULL) {
		return NULL;
	}
	
	if (gp_pcbs[proc_id]->msgHead == gp_pcbs[proc_id]->msgTail) {
    gp_pcbs[proc_id]->msgHead = NULL;
    gp_pcbs[proc_id]->msgTail = NULL;
  }
  else {
    gp_pcbs[proc_id]->msgHead = gp_pcbs[proc_id]->msgHead->next;
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
	
	if (gp_pcbs[PID_TIMER_IPROC]->msgTail == NULL) {
		gp_pcbs[PID_TIMER_IPROC]->msgHead = env;
		gp_pcbs[PID_TIMER_IPROC]->msgTail = env;
	}
	else {
		gp_pcbs[PID_TIMER_IPROC]->msgTail->next = env;
		gp_pcbs[PID_TIMER_IPROC]->msgTail = env;
	}
	//No need for pre-emption
	return RTX_OK;
}
