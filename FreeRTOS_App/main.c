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
void TPM_test(void *pvParameters);
void SecureBoot_test(void *pvParameters);

int main(int argc, char **argv) {

  (void)argc;
  (void)argv;

  // Peripherals initialization
  UART_init();
  SPI_init();

  // Test Tasks creation
  xTaskCreate(UART_test, "UART_test", configMINIMAL_STACK_SIZE, NULL, mainTASK_PRIORITY, NULL);

  xTaskCreate(SPI_test, "SPI_test", configMINIMAL_STACK_SIZE, NULL, mainTASK_PRIORITY+3, NULL);

  xTaskCreate(TPM_test, "TMP_test", configMINIMAL_STACK_SIZE, NULL, mainTASK_PRIORITY+4, NULL);

  xTaskCreate(SecureBoot_test, "SecureBoot_test", configMINIMAL_STACK_SIZE, NULL, mainTASK_PRIORITY+5, NULL);
  
  vTaskStartScheduler();
  for (;;)
    ;
}
void UART_test(void *pvParameters) {

  (void)pvParameters;

  uart_printf("------------- Starting UART Test -------------\n");

    uart_printf("Hello from UART of Group11!\n");

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
  }

  SPI_status();

  uart_printf("---------- Ending SPI Test -------------\n");
  vTaskDelete(NULL);

}

void TPM_test(void *pvParameters) {
  (void)pvParameters;
  uart_printf("------------- Starting TPM Extended Test -------------\n");

  TPM_init();

  // Generate keys
  TPM_generate_key(0);

  TPM_generate_key(1);

  uart_printf("-------------- TPM Extended Test Completed -------------\n");
  vTaskDelete(NULL);
}

void SecureBoot_test(void *pvParameters) {
  (void)pvParameters;

  uart_printf("------------- Starting Secure Boot Test -------------\n");

  // Example bootloader hash (must match the one in the TPM for success)
  uint8_t valid_boot_hash[32] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 
                                 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 
                                 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 
                                 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};

  // Write the valid hash to the TPM's DATA_IN register
  memcpy((void *)(S32K_TPM_BASE + 0x10), valid_boot_hash, sizeof(valid_boot_hash));

  // Trigger the secure boot verification command
  TPM_CMD_REG = 0x10;  // CMD_VERIFY_BOOT_HASH

  // Check the result in the STATUS register
  if (TPM_STATUS_REG & 0x01) {  // STATUS_READY
    uart_printf("Secure Boot Test: FAILURE (valid hash)\n");
  } else {
    uart_printf("Secure Boot Test: SUCCESS (valid hash)\n");
  }

  // Test with an invalid hash
  uint8_t invalid_boot_hash[32] = {0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 
                                   0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00, 
                                   0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88, 
                                   0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00};

  // Write the invalid hash to the TPM's DATA_IN register
  memcpy((void *)(S32K_TPM_BASE + 0x10), invalid_boot_hash, sizeof(invalid_boot_hash));

  // Trigger the secure boot verification command
  TPM_CMD_REG = 0x10;  // CMD_VERIFY_BOOT_HASH

  // Check the result in the STATUS register
  if (TPM_STATUS_REG & 0x01) {  // STATUS_READY
    uart_printf("Secure Boot Test: SUCCESS (invalid hash accepted)\n");
  } else {
    uart_printf("Secure Boot Test: FAILURE (invalid hash rejected)\n");
  }

  uart_printf("------------- Secure Boot Test Completed -------------\n");

  vTaskDelete(NULL);
}
