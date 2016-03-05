/**
 * @brief timer.h - Timer header file
 * @author Y. Huang
 * @date 2013/02/12
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#include <LPC17xx.h>
#include "k_message.h"

typedef struct timerQ timerQ;
struct timerQ {
    envelope* head;
    envelope* tail;
};

extern uint32_t timer_init ( uint8_t n_timer );  /* initialize timer n_timer */
extern uint32_t get_time( void ); /* Get current time */

extern void timer_insert( envelope* );
extern envelope* timer_dequeue( void );
extern int message_ready( void );
extern uint32_t get_time(void) ;

#endif /* ! _TIMER_H_ */
