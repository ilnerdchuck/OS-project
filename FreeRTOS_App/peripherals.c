#include "peripherals.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

// UART
// UART Initialization
void UART_init(void) {
  // Set baud rate
  *((uint32_t *)(UART0_BAUDDIV)) = 16;
  // Enable transmitter and receiver
  *((uint32_t *)(UART0_CTRL)) = 1;
}

static void uart_send_char(char c)
{
  *((uint32_t *)(UART0_DATA)) = (unsigned int)c;
}

static void uart_send_string(const char *s)
{
    while (*s) {
        uart_send_char(*s++);
    }
}

void uart_printf(const char *fmt, ...)
{
    char buffer[128];
    char *p = buffer;
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'c': {
                    char c = (char)va_arg(args, int);
                    *p++ = c;
                    break;
                }
                case 's': {
                    char *str = va_arg(args, char*);
                    while (*str) *p++ = *str++;
                    break;
                }
                case 'd': {
                    int val = va_arg(args, int);
                    char num[16];
                    bool neg = false;
                    if (val < 0) { neg = true; val = -val; }
                    int i = 0;
                    do { num[i++] = (val % 10) + '0'; val /= 10; } while (val > 0);
                    if (neg) *p++ = '-';
                    while (i--) *p++ = num[i];
                    break;
                }
                case 'x':
                case 'X': {
                    unsigned val = va_arg(args, unsigned);
                    char num[16];
                    int i = 0;
                    do { 
                        int digit = val % 16;
                        num[i++] = (digit < 10) ? '0' + digit : ((*fmt == 'x') ? 'a' : 'A') + (digit - 10);
                        val /= 16;
                    } while (val > 0);
                    *p++ = '0';
                    *p++ = 'x';
                    while (i--) *p++ = num[i];
                    break;
                }
                default:
                    *p++ = '%';
                    *p++ = *fmt;
            }
        } else {
            *p++ = *fmt;
        }
        fmt++;
    }
    *p = '\0';

    va_end(args);
    uart_send_string(buffer);
}

// SPI

// SPI Initialization
void SPI_init(void) {
  *((uint32_t *)(LPSPI0_CTRL)) = 1;
  *((uint32_t *)(LPSPI0_CRCPR)) = 7;        
  *((uint32_t *)(LPSPI0_CTRL2)) = 0;        // Default configuration
  *((uint32_t *)(LPSPI0_I2SCFGR)) = 0;      // Disable I2S
  *((uint32_t *)(LPSPI0_I2SPR)) = 0;        // Default configuration
  *((uint32_t *)(LPSPI0_CTRL)) |= (1 << 6); // Enable SPI
  *((uint32_t *)(LPSPI0_CTRL)) |= (1 << 2); // Master mode
}

// SPI all register status read
void SPI_status(void) {
  uart_printf("---------- SPI Registers Status ----------\n");
  uart_printf("LPSPI0_CTRL: %X\n", *((uint32_t *)(LPSPI0_CTRL)));
  uart_printf("LPSPI0_CTRL2: %x\n", *((uint32_t *)(LPSPI0_CTRL2)));
  uart_printf("LPSPI0_SR: %X\n", *((uint32_t *)(LPSPI0_SR)));
  uart_printf("LPSPI0_DR: %X\n", *((uint32_t *)(LPSPI0_DR)));
  uart_printf("LPSPI0_CRCPR: %X\n", *((uint32_t *)(LPSPI0_CRCPR)));
  uart_printf("LPSPI0_RXCRCR: %X\n", *((uint32_t *)(LPSPI0_RXCRCR)));
  uart_printf("LPSPI0_TXCRCR: %X\n", *((uint32_t *)(LPSPI0_TXCRCR)));
  uart_printf("LPSPI0_I2SCFGR: %X\n", *((uint32_t *)(LPSPI0_I2SCFGR)));
  uart_printf("LPSPI0_I2SPR: %X\n", *((uint32_t *)(LPSPI0_I2SPR)));
  uart_printf("------------------------------------------\n");
}

// SPI Write 
void SPI_write(uint8_t s) {
  while (!(*((uint32_t *)(LPSPI0_SR)) & 0x2))
    ; // Wait until TX ready

    *((uint32_t *)(LPSPI0_DR)) = s;
}

// SPI Read
void SPI_get(uint8_t *s) {
  while (!(*((uint32_t *)(LPSPI0_SR)) & 0x1))
    ; // Wait until RX ready
  
  *s = (uint8_t)(*((uint32_t *)(LPSPI0_DR)) & 0xFF);
}


void TPM_init(void) {
  uart_printf("TPM: Initializing TPM peripheral...\n");
  TPM_CONTROL_REG = 0x0;
  TPM_STATUS_REG = 0x0;
}

void TPM_generate_key(uint32_t key_index) {
  uart_printf("TPM: Generating key at index %d...\n", key_index);

  TPM_CONTROL_REG = key_index & 0xFF; // select key slot
  TPM_CMD_REG = 0x01; // trigger key generation (handled in QEMU write)
  vTaskDelay(pdMS_TO_TICKS(100));

  uint32_t status = TPM_STATUS_REG;
  uart_printf("TPM: Key generation command complete, status=0x%x\n", status);
}

void TPM_status(void) {
  uint32_t ctrl = TPM_CONTROL_REG;
  uint32_t stat = TPM_STATUS_REG;

  uart_printf("TPM: CONTROL=0x%x STATUS=0x%x\n", ctrl, stat);
}

void TPM_read(void) {
  uart_printf("TPM: Reading current registers...\n");
  TPM_status();
}

void TPM_write(void) {
  uart_printf("TPM: Writing test values...\n");
  TPM_CONTROL_REG = 0xABCD;
  TPM_STATUS_REG = 0x1234;
  TPM_status();
}