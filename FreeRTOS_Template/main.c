#include "FreeRTOS.h"
#include "projdefs.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <string.h>
#include <stdint.h>
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
  SPI_init(); 

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

uint8_t pippo = 0;
uint8_t pluto = 0;
char buff[100];
void producer(void *pvParameters) {

	(void) pvParameters;
    for (;;) {
        pippo += 10;
        /*UART_printf("Di carlo <3\n");*/
        SPI_write(pippo);
        /*vTaskDelay(pdMS_TO_TICKS(1000));*/
        /*for (int i=0; i<1000; i++) {*/
        /*  UART_printf("aaaa\n");*/
        /*} */
        
        SPI_get(&pluto);
        /*sprintf(buff, "%d\n", pluto);*/
        UART_printf("hello");
    }
}


