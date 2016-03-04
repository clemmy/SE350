#include "rtx.h"

void copyStr(char* src, char* dest) {
    while (1) {
        *dest = *src;

        if (*dest == '\0') {
            break;
        }
        src++;
        dest++;
    }
}

MSG_BUF* copyMessage(MSG_BUF* msg) {
    MSG_BUF* copy = (MSG_BUF*) request_memory_block();
    copyStr(msg->mtext, copy->mtext);
    return copy;
}

void kcdProc() {
    const int maxNumIdentifiers = 16;
    char identifiers[maxNumIdentifiers];
    int processes[maxNumIdentifiers];
    int numIdentifiers = 0;

    while (1) {
        int sender_id;
        MSG_BUF* msg = (MSG_BUF*) receive_message(&sender_id);
        if (msg->mtype == KCD_REG) {
            if (numIdentifiers < maxNumIdentifiers) {
                identifiers[numIdentifiers] = msg->mtext[1]; // msg->mtext[0] == '%', msg->mtext[2] == '\0'
                processes[numIdentifiers] = sender_id;

                numIdentifiers++;
            }
            release_memory_block((void*) msg);
        }
        else {
            int recv_id = -1;
            if (msg->mtext[0] == '%') {
                for (int i = 0; i < numIdentifiers; i++) {
                    if (identifiers[i] == msg->mtext[1]) {
                        recv_id = processes[i];
                        break;
                    }
                }
            }

            if (recv_id == -1) {
                msg->mtype = DEFAULT;
                send_message(PID_CRT, (void*) msg);
            }
            else {
                msg->mtype = DEFAULT;
                MSG_BUF* copy = copyMessage(msg);

                send_message(PID_CRT, (void*) msg);

                send_message(recv_id, (void*) copy);
            }
        }
    }
}

void crtProc() {
    while (1) {
        int sender_id;
        MSG_BUF *msg = (MSG_BUF *) receive_message(&sender_id);
        char* string = msg->mtext;

        // TODO print string using UART interrupt process

        release_memory_block((void*) msg);
    }
}
