#ifndef K_MESSAGE_H_
#define K_MESSAGE_H_

// #include "k_rtx.h"

/* ---- Forward Declarations ---- */
typedef struct pcb PCB;
typedef struct PCBQ PCBQ;

/* message buffer */
typedef struct msgbuf
{
	int mtype;              /* user defined message type */
	char mtext[1];          /* body of the message */
} MSG_BUF;


typedef struct _envelope envelope;
struct _envelope {
	envelope* next;
	int sender_id;
	int recv_id;
};

extern PCB **gp_pcbs;
extern PCB *gp_current_process;
extern PCBQ ReadyPQ[];


int k_send_message(int process_id, void* message_envelope);

void* k_receive_message(int* sender_id);

int k_delayed_send(int process_id, void* message_envelope, int delay);


#endif