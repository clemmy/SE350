/**
 * @brief timer.c - Timer example code. Tiemr IRQ is invoked every 1ms
 * @author T. Reidemeister
 * @author Y. Huang
 * @author NXP Semiconductors
 * @date 2012/02/12
 */

#include <LPC17xx.h>
#include "timer.h"
#include "k_process.h"
#include "k_message.h"

#define BIT(X) (1<<X)

volatile uint32_t g_timer_count = 0; // increment every 1 ms
timerQ Q;

/**
 * @brief: initialize timer. Only timer 0 is supported
 */
uint32_t timer_init(uint8_t n_timer) 
{
	Q.head = NULL;
	Q.tail = NULL;
	
	LPC_TIM_TypeDef *pTimer;
	if (n_timer == 0) {
		/*
		Steps 1 & 2: system control configuration.
		Under CMSIS, system_LPC17xx.c does these two steps
		
		----------------------------------------------------- 
		Step 1: Power control configuration.
		        See table 46 pg63 in LPC17xx_UM
		-----------------------------------------------------
		Enable UART0 power, this is the default setting
		done in system_LPC17xx.c under CMSIS.
		Enclose the code for your refrence
		//LPC_SC->PCONP |= BIT(1);
	
		-----------------------------------------------------
		Step2: Select the clock source, 
		       default PCLK=CCLK/4 , where CCLK = 100MHZ.
		       See tables 40 & 42 on pg56-57 in LPC17xx_UM.
		-----------------------------------------------------
		Check the PLL0 configuration to see how XTAL=12.0MHZ 
		gets to CCLK=100MHZ in system_LPC17xx.c file.
		PCLK = CCLK/4, default setting in system_LPC17xx.c.
		Enclose the code for your reference
		//LPC_SC->PCLKSEL0 &= ~(BIT(3)|BIT(2));	

		-----------------------------------------------------
		Step 3: Pin Ctrl Block configuration. 
		        Optional, not used in this example
		        See Table 82 on pg110 in LPC17xx_UM 
		-----------------------------------------------------
		*/
		pTimer = (LPC_TIM_TypeDef *) LPC_TIM0;

	} else { /* other timer not supported yet */
		return 1;
	}

	/*
	-----------------------------------------------------
	Step 4: Interrupts configuration
	-----------------------------------------------------
	*/

	/* Step 4.1: Prescale Register PR setting 
	   CCLK = 100 MHZ, PCLK = CCLK/4 = 25 MHZ
	   2*(12499 + 1)*(1/25) * 10^(-6) s = 10^(-3) s = 1 ms
	   TC (Timer Counter) toggles b/w 0 and 1 every 12500 PCLKs
	   see MR setting below 
	*/
	pTimer->PR = 12499;  

	/* Step 4.2: MR setting, see section 21.6.7 on pg496 of LPC17xx_UM. */
	pTimer->MR0 = 1;

	/* Step 4.3: MCR setting, see table 429 on pg496 of LPC17xx_UM.
	   Interrupt on MR0: when MR0 mathches the value in the TC, 
	                     generate an interrupt.
	   Reset on MR0: Reset TC if MR0 mathches it.
	*/
	pTimer->MCR = BIT(0) | BIT(1);

	g_timer_count = 0;

	/* Step 4.4: CSMSIS enable timer0 IRQ */
	NVIC_EnableIRQ(TIMER0_IRQn);

	/* Step 4.5: Enable the TCR. See table 427 on pg494 of LPC17xx_UM. */
	pTimer->TCR = 1;

	return 0;
}

/**
 * @brief: use CMSIS ISR for TIMER0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine. 
 *       The actual c_TIMER0_IRQHandler does the rest of irq handling
 */
__asm void TIMER0_IRQHandler(void)
{
	PRESERVE8
	;BL __disable_irq
	IMPORT c_TIMER0_IRQHandler
	PUSH{r4-r11, lr}
	BL c_TIMER0_IRQHandler
	POP{r4-r11}
	IMPORT k_release_processor
	;BL __enable_irq
	BL k_release_processor
}

/**
 * @brief: c TIMER0 IRQ Handler
 */
void c_TIMER0_IRQHandler(void)
{
	/* ack inttrupt, see section  21.6.1 on pg 493 of LPC17XX_UM */
	LPC_TIM0->IR = BIT(0);  
	g_timer_count++;
	
	envelope* env;
  while (1){
		env = k_receive_message_non_blocking(PID_TIMER_IPROC);
		if (env == NULL) {
			break;
		}
		//place envelope in queue sorted by send time
		timer_insert(env);
	}
	//send messages in queue that have expired
	while (message_ready()){
		env = timer_dequeue();
		timer_send_message(env);
	}
}

/**
 * @brief: Returns current time
 */
uint32_t get_time(void)
{
	return (int)g_timer_count;
}

/**
 * @brief: insert envelope into queue (in sorted order)
 */
void timer_insert(envelope* env)
{
	//iterate, insert envelope when the next envelope's send time is later
	envelope* curr = Q.head;
	envelope* next = curr->next;

	//case when queue is empty
	if (curr == NULL){
		Q.head = env;
		Q.tail = env;
		env->next = NULL;
		return;
	}
	//case when inserting at the front
	if (env->send_time <= curr->send_time){
		Q.head = env;
		env->next = curr;
		return;
	}
	//else
	while(1){
		if (next == NULL){
			curr->next = env;
			env->next = NULL;
			Q.tail = env;
			break;
		}
		if (env->send_time <= next->send_time){
			curr->next = env;
			env->next = next;
			break;
		}
		curr = next;
		next = curr->next;
	}
}

/**
 * @brief: return first envelope into queue
 */
envelope* timer_dequeue( void )
{
	//standard dequeue
	envelope* env = Q.head;
	Q.head = env->next;
	env->next = NULL;
	
	if (Q.head == NULL){
		Q.tail = NULL;
	}
	
	return env;
}

/**
 * @brief: return whether the message on top of queue should be sent
 */
int message_ready( void )
{
	//Returns 1 if the first message on a queue exists and has a send time
	//earlier than current time
	return Q.head != NULL && Q.head->send_time < get_time();
}
