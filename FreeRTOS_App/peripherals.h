#ifndef __PRINTF__
#define __PRINTF__

#include "FreeRTOS.h"
#include <stdint.h>

//UART
#define UART0_ADDRESS (0x40328000UL)
#define UART0_DATA UART0_ADDRESS + 4UL
#define UART0_STATE UART0_ADDRESS + 0UL
#define UART0_CTRL UART0_ADDRESS + 0xCUL
#define UART0_BAUDDIV UART0_ADDRESS + 8UL

void UART_init(void);
void uart_printf(const char *fmt, ...);


// SPI
// LPSPI0 base pointer 
#define LPSPI0_BASE (0x40358000UL)

// #define LPSPI0_CTRL (*(((volatile uint32_t *)(LPSPI0_BASE + 0x0UL))))

// LPSPI0 registers 
#define LPSPI0_CTRL (LPSPI0_BASE + 0x0UL)
#define LPSPI0_CTRL2 (LPSPI0_BASE + 0x04UL)
#define LPSPI0_SR (LPSPI0_BASE + 0x08UL)
#define LPSPI0_DR (LPSPI0_BASE + 0xCUL)
#define LPSPI0_CRCPR (LPSPI0_BASE + 0x10UL)
#define LPSPI0_RXCRCR (LPSPI0_BASE + 0x14UL)
#define LPSPI0_TXCRCR (LPSPI0_BASE + 0x18UL)
#define LPSPI0_I2SCFGR (LPSPI0_BASE + 0x1CUL)
#define LPSPI0_I2SPR (LPSPI0_BASE + 0x20UL)




void SPI_init(void);
void SPI_status(void);
void SPI_write(uint8_t);
void SPI_get(uint8_t *);

#define S32K_TPM_BASE 0x40370000

#define TPM_CONTROL_REG (*(volatile uint32_t *)(S32K_TPM_BASE + 0x00))
#define TPM_STATUS_REG  (*(volatile uint32_t *)(S32K_TPM_BASE + 0x04))
#define TPM_CMD_REG     (*(volatile uint32_t *)(S32K_TPM_BASE + 0x08))

void TPM_init(void);
void TPM_generate_key(uint32_t key_index);
void TPM_status(void);
void TPM_read(void);
void TPM_write(void);
// void TPM_read_key(uint32_t , uint8_t *);
// void TPM_secure_boot_verify(void);

#endif
