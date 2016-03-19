#include "rtx.h"
#include "uart_polling.h"
#define TIME_DELAY 1000

extern char* nextNonWhitespace(char* cur);
extern int charToInt(char c);
extern char intToChar(int i);
extern void copyStr(char* src, char* dest);

/**
 * Time format: hh:mm:ss
 * Returns time in seconds
 */
int parseTime(char* timeStr) {
		if (timeStr[2] != ':' || timeStr[5] != ':') {
				return 0;
		}
    int hours = charToInt(timeStr[0]) * 10 + charToInt(timeStr[1]);
    int minutes = charToInt(timeStr[3]) * 10 + charToInt(timeStr[4]);
    int seconds = charToInt(timeStr[6]) * 10 + charToInt(timeStr[7]);
    return hours * 60 * 60 + minutes * 60 + seconds;
}

/**
 * Gets time in seconds and writes its str representation in hh:mm:ss to dest
 * dest must have at least 11 bytes!!!
 */
void timeToStr(int time, char* dest) {
    int hours = time / (60 * 60);
    time %= 60 * 60;
    int minutes = time / 60;
    int seconds = time % 60;

    hours %= 24; // ensures hours is between 0 and 99

    dest[0] = intToChar(hours / 10);
    dest[1] = intToChar(hours % 10);
    dest[2] = ':';
    dest[3] = intToChar(minutes / 10);
    dest[4] = intToChar(minutes % 10);
    dest[5] = ':';
    dest[6] = intToChar(seconds / 10);
    dest[7] = intToChar(seconds % 10);
		dest[8] = '\n';
		dest[9] = '\r';
    dest[10] = '\0';
}

void wallClockProc() {
    MSG_BUF* msg = (MSG_BUF*) request_memory_block();
    msg->mtype = KCD_REG;
		copyStr("%W", msg->mtext);
    send_message(PID_KCD, msg);

    int time = 0;
    char incrementorID = 0;

    while (1) {
        int sender_id;
        MSG_BUF* msg = (MSG_BUF*) receive_message(&sender_id);
        char command = msg->mtext[2];

        if (command == 'R') {
            incrementorID++;
            time = 0;
            msg->mtype = DEFAULT;
            msg->mtext[0] = '%';
            msg->mtext[1] = 'W';
            msg->mtext[2] = 'I';
            msg->mtext[3] = '\0';
            msg->mtext[4] = incrementorID;
            delayed_send(PID_CLOCK, msg, TIME_DELAY);

            MSG_BUF* printMsg = (MSG_BUF*) request_memory_block();
            printMsg->mtype = DEFAULT;
            timeToStr(time, printMsg->mtext);
            send_message(PID_CRT, printMsg);
        }
        else if (command == 'I') {
            if (msg->mtext[4] == incrementorID) {
                time++;
                delayed_send(PID_CLOCK, msg, TIME_DELAY);

                MSG_BUF* printMsg = (MSG_BUF*) request_memory_block();
                printMsg->mtype = DEFAULT;
                timeToStr(time, printMsg->mtext);
                send_message(PID_CRT, printMsg);
            }
            else {
                release_memory_block((void*) msg);
            }
        }
        else if (command == 'S') {
            incrementorID++;
            char* timeStart = nextNonWhitespace(msg->mtext + 3);
						if (timeStart == NULL) {
							release_memory_block((void*) msg);
							continue;
						}
            time = parseTime(timeStart);
            msg->mtype = DEFAULT;
            msg->mtext[0] = '%';
            msg->mtext[1] = 'W';
            msg->mtext[2] = 'I';
            msg->mtext[3] = '\0';
            msg->mtext[4] = incrementorID;
            delayed_send(PID_CLOCK, msg, TIME_DELAY);

            MSG_BUF* printMsg = (MSG_BUF*) request_memory_block();
            printMsg->mtype = DEFAULT;
            timeToStr(time, printMsg->mtext);
            send_message(PID_CRT, printMsg);
        }
        else if (command == 'T') {
            incrementorID++;
            release_memory_block((void*) msg);
        }
        else {
            release_memory_block((void*) msg);
        }
    }
}
