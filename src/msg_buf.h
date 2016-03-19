#ifndef MSG_BUF_H_
#define MSG_BUF_H_

typedef struct msgbuf
{
	int mtype;              /* user defined message type */
	char mtext[1];          /* body of the message */
} MSG_BUF;

#endif
