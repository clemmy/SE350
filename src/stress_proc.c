#include "rtx.h"
#include "k_message.h"
#include "uart_polling.h"

extern char* copyStr(char* src, char* dest);

#define COUNT_REPORT 999
#define WAKEUP10 10

/*void procA(void)
{
  while (1) {
    release_processor();
  }
}

void procB(void)
{
  while (1) {
    release_processor();
  }
}

void procC(void)
{
  while (1) {
    release_processor();
  }
}*/



void procA(void)
{
	MSG_BUF* p = (MSG_BUF*) request_memory_block();
	p->mtype = KCD_REG;
	copyStr("%Z", p->mtext);
	send_message(PID_KCD, p);
	
	int sender_id;
  while (1) {
		p = (MSG_BUF*) receive_message(&sender_id);
		if (p->mtext[0] == '%' && p->mtext[1] == 'Z') {
			release_memory_block((void*) p);
			break;
		}
		else {
			release_memory_block((void*) p);
		}
  }
	
	char num = 0;
	
	while (1) {
		p = (MSG_BUF*) request_memory_block();
		uart1_put_string("A request memory block\n\r");
		p->mtype = COUNT_REPORT;
		p->mtext[0] = num;
		send_message(PID_B, (void*) p);
		num++;
		release_processor();
	}	
}

void procB(void)
{
	int sender_id;
	MSG_BUF* p;
  while (1) {
    p = (MSG_BUF*) receive_message(&sender_id);
		uart1_put_string("B received message\n\r");
		send_message(PID_C, (void*) p);
  }
}

envelope* msg_buf_to_envelope(MSG_BUF* msg) {
	return ((envelope*) msg) - 1;
}

MSG_BUF* envelope_to_msg_buf(envelope* env) {
	return (MSG_BUF*) (env + 1);
}

void procC(void)
{
	envelope* msg_q_head = NULL;
	envelope* msg_q_tail = NULL;
	MSG_BUF* p;
	int sender_id;
	
  while (1) {
    if (msg_q_head == NULL) {
			p = (MSG_BUF*) receive_message(&sender_id);
			uart1_put_string("C receive new message\n\r");
		}
		else {
			p = envelope_to_msg_buf(msg_q_head);
			msg_q_head = msg_q_head->next;
			if (msg_q_head == NULL) {
				msg_q_tail = NULL;
			}
		}
		
		if (p->mtype == COUNT_REPORT && (p->mtext[0] % 20 == 0)) {
			p->mtype = DEFAULT;
			copyStr("Process C\n\r", p->mtext);
			send_message(PID_CRT, p);
			
			MSG_BUF* q = (MSG_BUF*) request_memory_block();
			q->mtype = WAKEUP10;
			delayed_send(PID_C, (void*) q, 10000);
			
			uart1_put_string("C request memory block\n\r");
			
			while (1) {
				p = (MSG_BUF*) receive_message(&sender_id);
				uart1_put_string("C queue message\n\r");
				if (p->mtype == WAKEUP10) {
					break;
				}
				else {
					envelope* env = msg_buf_to_envelope(p);
					env->next = NULL;
					if (msg_q_tail == NULL) {
						msg_q_tail = env;
						msg_q_head = env;
					}
					else {
						msg_q_tail->next = env;
						msg_q_tail = env;
					}
				}
			}			
		}
		release_memory_block((void*) p);
		uart1_put_string("C memory released\n\r");
		release_processor();
  }
}
