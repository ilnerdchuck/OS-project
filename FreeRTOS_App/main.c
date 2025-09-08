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

void UART_test(void *pvParameters);
void SPI_test(void *pvParameters);

int main(int argc, char **argv) {

  (void)argc;
  (void)argv;

  // Peripherals initialization
  UART_init();
  SPI_init();

  // Test Tasks creation
  xTaskCreate(UART_test, "UART_test", configMINIMAL_STACK_SIZE, NULL,
    mainTASK_PRIORITY, NULL);

  xTaskCreate(SPI_test, "SPI_test", configMINIMAL_STACK_SIZE, NULL,
              mainTASK_PRIORITY, NULL);

  vTaskStartScheduler();
  for (;;)
    ;
}
void UART_test(void *pvParameters) {

  (void)pvParameters;

  uart_printf("------------- Starting UART Test -------------\n");

    uart_printf("Hello from UART of Group10!\n");

  uart_printf("-------------- Ending UART Test -------------\n");

  vTaskDelete(NULL);
}

void SPI_test(void *pvParameters) {

  (void)pvParameters;
  uart_printf("------------- Starting SPI Test -------------\n");

  SPI_status();

  uint32_t pippo = 0x00;
  uint32_t pluto = 0x00;
  for (int i=0; i<10; ++i) {
    uart_printf("---------- Write Test -------------\n");

    uart_printf("Sending: %x\n", pippo);
    SPI_write((uint8_t)pippo);
    
    uart_printf("---------- Read Test -------------\n");
    
    SPI_get((uint8_t *)&pluto);
    pippo = pluto;

    uart_printf("SPI read: %x\n", pluto);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  SPI_status();

  uart_printf("---------- Ending SPI Test -------------\n");
}