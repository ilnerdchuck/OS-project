#include "FreeRTOS.h"
#include "projdefs.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "peripherals.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define mainTASK_PRIORITY (tskIDLE_PRIORITY + 2)

void test(void *pvParameters);

int main(int argc, char **argv) {

  (void)argc;
  (void)argv;

  UART_init();
  SPI_init();

  xTaskCreate(test, "Task1", configMINIMAL_STACK_SIZE, NULL,
              mainTASK_PRIORITY, NULL);

  vTaskStartScheduler();
  for (;;)
    ;
}

int pippo = 0;
int pluto = 0;

void test(void *pvParameters) {

  (void)pvParameters;
  for (;;) {
    pippo += 10;
    uart_printf("Sending: %x\n", pippo);

    SPI_write((uint8_t)pippo);

    SPI_get((uint8_t *)&pluto);

    uart_printf("SPI got: %x\n", pluto);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}