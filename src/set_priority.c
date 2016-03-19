#include "rtx.h"

extern char* nextNonWhitespace(char* cur);
extern int charToInt(char c);
extern char intToChar(int i);
extern void copyStr(char* src, char* dest);

void error_handler(void) {
	MSG_BUF* msg = (MSG_BUF*) request_memory_block();
	msg->mtype = DEFAULT;
	
	copyStr("Error: Invalid Parameter", msg->mtext);
	
	send_message(PID_CRT, msg);
}


void setPriorityProc(void) {
	MSG_BUF* msg = (MSG_BUF*) request_memory_block();
	msg->mtype = KCD_REG;
	copyStr("%C", msg->mtext);
	send_message(PID_KCD, msg);
	
	int sender_id;
	while (1) {		
		MSG_BUF* msg = (MSG_BUF*) receive_message(&sender_id);
		char* procIdStart = nextNonWhitespace(msg->mtext + 2);
		if (procIdStart == NULL) {
			error_handler();
			continue;
		}
		
		int procId;
		
		int procIdChar1 = charToInt(*procIdStart);
		if (0 <= procIdChar1 && procIdChar1 <= 9) {
			procId = procIdChar1;
			
			procIdStart++;
			
			int procIdChar2 = charToInt(*procIdStart);
			if (0 <= procIdChar2 && procIdChar2 <= 9) {
				procId = procId * 10 + procIdChar2;
			}
			else if (!(procIdChar2 == ' ' || procIdChar2 == '\t' ||
							 procIdChar2 == '\n' || procIdChar2 == '\r')) {
				error_handler();
				continue;
			}
			
			procIdStart++;
			
			char* newPriorityChar = nextNonWhitespace(procIdStart);
			if (newPriorityChar == NULL) {
				error_handler();
				continue;
			}
			
			int newPriority = charToInt(*newPriorityChar);
			
			int returnCode = set_process_priority(procId, newPriority);
			
			if (returnCode == RTX_ERR) {
				error_handler();
				continue;
			}
		}
		else {
			error_handler();
			continue;
		}		
	}	
}
