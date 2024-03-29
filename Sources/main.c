/* ###################################################################
**     Filename    : main.c
**     Project     : BasePEForMarco
**     Processor   : MK20DN64VLH5
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2014-09-27, 12:01, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.01
** @brief
**         Main module.
**         This module contains user's application code.
*/         
/*!
**  @addtogroup main_module main module documentation
**  @{
*/         
/* MODULE main */

#include <string.h>

/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "AS1.h"
#include "ASerialLdd1.h"
#include "CI2C0.h"
#include "IntI2cLdd1.h"
#include "AS0.h"
#include "ASerialLdd2.h"
#include "SM1.h"
#include "SMasterLdd1.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "MS5611Driver.h"

void vAssertCalled( void )
{
	PE_DEBUGHALT();
}

int i = 0;
int j = 0;

char receivedBuffer [10];
QueueHandle_t  uart1RcvQueue;
QueueHandle_t  uart0RcvQueue;

SemaphoreHandle_t dbgUARTDataReady;
SemaphoreHandle_t btUARTDataReady;

void btRcvCallback(char *data)
{
	i++;
	BaseType_t woken = pdFALSE;
	//xQueueSendFromISR(uart0RcvQueue, data, &woken);
	xSemaphoreGiveFromISR(btUARTDataReady, &woken);
}

/* User includes (#include below this line is not maintained by Processor Expert) */
void btSerial( void *pvParameters )
{
	char uart0Data;
	word sent;

	AS0_SetRXCallback (&btRcvCallback);

	while(1) {
		xSemaphoreTake(btUARTDataReady, portMAX_DELAY);
		AS0_RecvChar(&uart0Data);
		AS1_SendBlock(&uart0Data, 1, &sent);
	}
}

void dbgRcvCallback(char *data)
{
	i++;
	BaseType_t woken = pdFALSE;
	//xQueueSendFromISR(uart0RcvQueue, data, &woken);
	xSemaphoreGiveFromISR(dbgUARTDataReady, &woken);
}

/* User includes (#include below this line is not maintained by Processor Expert) */
void debugSerial( void *pvParameters )
{
	char *helloWorld = "Hello World! ";
	char *retChar = "\nbtcontrol > ";
	char receivedData[80];
	int select = 0;

	word sent;

	AS1_SetRXCallback (&dbgRcvCallback);
	AS1_SendBlock(helloWorld, 13, &sent);
	AS1_SendBlock(retChar, 13, &sent);

	while(1) {
		//xQueueReceive(uart0RcvQueue, &uart1Data, portMAX_DELAY);
		xSemaphoreTake(dbgUARTDataReady, portMAX_DELAY);
		AS1_RecvChar(&(receivedData[select]));
		AS1_SendBlock(&(receivedData[select]), 1, &sent);
		select ++;

		if(receivedData[select - 1] == 0xd) {

			AS0_SendBlock(receivedData, select - 1, &sent);
			AS1_SendBlock(retChar, 13, &sent);

			select = 0;
		}

		//AS0_SendBlock(&uart1Data, 1, &sent);
	}
}

void I2CTestTask( void *pvParameters )
{
	uint32 readTemp = 0;
	uint32 readPress = 0;
	EPROM_5611 *myEpromData;
	word sent;

	const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

	ms5611Init();
	myEpromData = ms5611GetEprom();

	while (1) {
		readTemp = ms5611ReadTemperature();
		readPress = ms5611ReadPressure();

		AS0_SendBlock((char*)&readPress, 4, &sent);
		// Wait 1s
		vTaskDelay(xDelay);

	}
}

/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
	/* Write your local variable definition here */

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
#if 0
    dbgUARTDataReady = xSemaphoreCreateCounting( 10, 0 );
    btUARTDataReady = xSemaphoreCreateCounting( 10, 0 );

	/* Start the two tasks as described in the comments at the top of this
	file. */
	xTaskCreate( btSerial,				/* The function that implements the task. */
				"BTSerial", 								/* The text name assigned to the task - for debug only as it is not used by the kernel. */
				configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task.  Not actually used as a stack in the Win32 simulator port. */
				NULL,								/* The parameter passed to the task - not used in this example. */
				( tskIDLE_PRIORITY + 2 ), 	/* The priority assigned to the task. */
				NULL );								/* The task handle is not required, so NULL is passed. */

	xTaskCreate( debugSerial,				/* The function that implements the task. */
				"DebugSerial", 								/* The text name assigned to the task - for debug only as it is not used by the kernel. */
				configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task.  Not actually used as a stack in the Win32 simulator port. */
				NULL,								/* The parameter passed to the task - not used in this example. */
				( tskIDLE_PRIORITY + 2 ), 	/* The priority assigned to the task. */
				NULL );								/* The task handle is not required, so NULL is passed. */

#else
	// This is to bringup I2C

	xTaskCreate( I2CTestTask,				/* The function that implements the task. */
				"I2C", 								/* The text name assigned to the task - for debug only as it is not used by the kernel. */
				configMINIMAL_STACK_SIZE, 			/* The size of the stack to allocate to the task.  Not actually used as a stack in the Win32 simulator port. */
				NULL,								/* The parameter passed to the task - not used in this example. */
				( tskIDLE_PRIORITY + 2 ), 	/* The priority assigned to the task. */
				NULL );								/* The task handle is not required, so NULL is passed. */

#endif

	/* Start the tasks and timer running. */
	vTaskStartScheduler();

  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.4 [05.09]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
