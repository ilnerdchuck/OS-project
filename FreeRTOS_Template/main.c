#include "FreeRTOS.h"
#include "projdefs.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include "uart.h"

#define mainTASK_PRIORITY    ( tskIDLE_PRIORITY + 2 )

void producer(void *pvParameters);
/*void consumer(void *pvParameters);*/

/*SemaphoreHandle_t xRead,xWrite;*/

/*uint16_t buffer[10] = {0};*/
/*uint16_t head, tail = 0; */

int main(int argc, char **argv){

	(void) argc;
	(void) argv;

  UART_init();
  

	xTaskCreate(
		producer,
		"Task1",
		configMINIMAL_STACK_SIZE,
		NULL,
		mainTASK_PRIORITY,
		NULL
	);


	vTaskStartScheduler();
    for( ; ; );
}

int pippo = 0;
void producer(void *pvParameters) {

	(void) pvParameters;
    for (;;) {
        pippo += 10;
        UART_printf("Di carlo <3\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


