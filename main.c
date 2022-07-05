
#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include"semphr.h"

#define CCM_RAM __attribute__((section(".ccmram")))




// ----------------------------------------------------------------------------

#include "led.h"



static void Sender1_TimerCallback( TimerHandle_t xTimer );
static void Sender2_TimerCallback( TimerHandle_t xTimer );
static void Receiver_TimerCallback( TimerHandle_t xTimer );

/*-----------------------------------------------------------*/


static TimerHandle_t xTimer1 = NULL;
static TimerHandle_t xTimer2 = NULL;
static TimerHandle_t xTimer3 = NULL;

BaseType_t xTimer1Started, xTimer2Started,xTimer3Started;

void SenderTask1(void*p);
void SenderTask2(void*p);
void ReceiverTask(void*p);
void Reset_fun();


SemaphoreHandle_t SemSender1;
SemaphoreHandle_t SemSender2;
SemaphoreHandle_t SemReceiver;


int blocked=0;
int transmitted=0;
int received=0;


#define queue_size 2
#define MaxReceived 500

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


QueueHandle_t Queue=0;


int
main(int argc, char* argv[])
{
	// Add your code here.

	Queue=xQueueCreate(queue_size,queue_size*sizeof(char[10]));  //we created a queue

	xTaskCreate(SenderTask1,"Sender1",1024,NULL,1,0);
	xTaskCreate(SenderTask2,"Sender2",1024,NULL,1,0);
	xTaskCreate(ReceiverTask,"Receiver",1024,NULL,2,0);

	vSemaphoreCreateBinary(SemSender1);
	vSemaphoreCreateBinary(SemSender2);
	vSemaphoreCreateBinary(SemReceiver);

	xTimer1 = xTimerCreate( "Timer1", ( pdMS_TO_TICKS(100) ), pdTRUE, ( void * ) 0, Sender1_TimerCallback);
	xTimer2 = xTimerCreate( "Timer2", ( pdMS_TO_TICKS(100) ), pdTRUE, ( void * ) 0, Sender2_TimerCallback);
	xTimer3 = xTimerCreate( "Timer3", ( pdMS_TO_TICKS(100) ), pdTRUE, ( void * ) 1, Receiver_TimerCallback);
	Reset_fun();

	if( ( xTimer1 != NULL ) && ( xTimer2 != NULL ) && ( xTimer3 != NULL ) )
	{
		xTimer1Started = xTimerStart( xTimer1, 0 );
		xTimer2Started = xTimerStart( xTimer2, 0 );
		xTimer3Started = xTimerStart( xTimer3, 0 );
	}

	if( xTimer1Started == pdPASS && xTimer2Started == pdPASS && xTimer3Started == pdPASS)  //if 3 timers created successfully
	{
		vTaskStartScheduler();  // we start scheduler
	}

	return 0;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------

void SenderTask1(void*p)
{
	char tx[10];  //string to store in it
	TickType_t xTimeNow;
	while(1){

		if(xSemaphoreTake(SemSender1,portMAX_DELAY) )   //take semaphore
		{

			xTimeNow =xTaskGetTickCount();  //obtain current tick count

			sprintf(tx,"\nTime is : %d\n", xTimeNow);   //store the word "Time is : " and the current tick count
                                                        //inside the string tx

		if(!xQueueSend(Queue,tx,0))       //we send the string tx to the queue
		{
			puts("Sender1 Blocked");           //if sender1 failed to send we print "Sender1 Blocked"
			blocked++;
		}
		else                          //if sender1 send successfully we print "Sender1 *Sends*"
		{                            //and we increment the total number of transmitted messages
			printf("Sender1 *Sends*\n");
			transmitted++;
		}

		}
	}
}

void SenderTask2(void*p)
{
	char tx2[10];
	TickType_t xTimeNow;

	while(1){

			if(xSemaphoreTake(SemSender2,portMAX_DELAY) )
			{

				xTimeNow =xTaskGetTickCount();  //obtain current tick count
				sprintf(tx2,"\nTime is %d\n", xTimeNow);

			if(!xQueueSend(Queue,tx2,0))
			{
				puts("Sender2 Blocked");
				blocked++;
			}
			else
			{
				printf("Sender2 *Sends*\n");
				transmitted++;
			}

			}
		}
}

void ReceiverTask(void*p)
{   char rx[10];
		while(1)
		{
			if(xSemaphoreTake(SemReceiver,portMAX_DELAY))   //take receiver semaphore
			{
				if( xQueueReceive(Queue,rx,100) )    //receive from queue
				{
				printf(" %s\n\n",rx);       //if successfull receive , it prints the Time sended by sendertasks
				received++;                 //increments total number of received messages
				}
				else
					puts("Receiver Blocked");     //if not recieved successfully , prints "Receiver Blocked"
			}
		}
}




static void Sender1_TimerCallback( TimerHandle_t xTimer )
{
	xSemaphoreGive(SemSender1);//give semaphore
}

static void Sender2_TimerCallback( TimerHandle_t xTimer )
{

	xSemaphoreGive(SemSender2);//give semaphore


}

static void Receiver_TimerCallback( TimerHandle_t xTimer )
{
	xSemaphoreGive(SemReceiver);  //give semaphore
	if(received==MaxReceived)  {Reset_fun();}
}

void Reset_fun()
{
	static int Reset_call_count=0;
	int random_no=0;
	int lower_bound[]={50,80,110,140,170,200};
	int upper_bound[]={150,200,250,300,350,400};
	srand(time(NULL));

	//below line will generate a random no bet upper and lower bounds every time Reset_fun is called
	random_no= (rand() % (upper_bound[Reset_call_count]-lower_bound[Reset_call_count]+1) )
			+lower_bound[Reset_call_count];

	if(received==MaxReceived)
	{
		printf("\n\nReceived 500 !!!!!!!! for the %d Time\n",Reset_call_count);
		printf("The number of successfully send messages = %d\n",transmitted);
		printf("The Total Number of Blocked messages = %d\n",blocked);
		transmitted=0; //reset total no. of transmitted messages
		blocked=0; //reset total no. of blocked messages
		received=0; //reset total no. of received messages
		xQueueReset(Queue); //clears queue
	}
	if (Reset_call_count==6)  //if all values of array is used , destroy program
	{
		xTimerDelete(xTimer1,(TickType_t) 0);
		xTimerDelete(xTimer2,(TickType_t) 0);
		xTimerDelete(xTimer3,(TickType_t) 0);
		printf("**************Game Over**************\r\n");
		vTaskEndScheduler();
		exit(0);
		return;
	}
	xTimerChangePeriod(xTimer1,random_no,(TickType_t) 0);
	xTimerChangePeriod(xTimer2,random_no,(TickType_t) 0);
	printf("------Sender Time now is %d -------",random_no);
	puts("");
	Reset_call_count++;
}




void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}

void vApplicationTickHook(void) {
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

