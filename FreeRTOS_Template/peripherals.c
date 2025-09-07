#include "peripherals.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

// UART
// UART Initialization
void UART_init(void) {

  *((uint32_t *)(UART0_BAUDDIV)) = 16; // Set baud rate divisor for 115200 baud
  *((uint32_t *)(UART0_CTRL)) = 1; // Enable UART0
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
