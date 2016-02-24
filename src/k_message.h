#ifndef K_MESSAGE_H_
#define K_MESSAGE_H_

#include "rtx.h"
// gets MSG_BUF struct


typedef struct _envelope envelope;
struct _envelope {
	envelope* next;
	int sender_id;
	int recv_id;
};

extern PCB **gp_pcbs;
extern PCB *gp_current_process;
extern PCBQ ReadyPQ[NUM_OF_PRIORITIES];


int k_send_message(int process_id, void* message_envelope);

void* k_receive_message(int* sender_id);

int k_delayed_send(int process_id, void* message_envelope, int delay);


#endif