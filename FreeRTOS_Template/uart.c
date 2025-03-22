#include "uart.h"

void UART_init( void )
{
    UART0_BAUDDIV = 16;
    UART0_CTRL = 1;
}

void UART_printf(const char *s) {
    while(*s != '\0') {
        UART0_DATA = (unsigned int)(*s);
        s++;
    }
}

void SPI_init( void )
{
    LPSPI0_CTRL = 1;
}

void SPI_write(uint8_t s) {
    
    LPSPI0_DR = s;
}

void SPI_get(uint8_t *s) {
      *s = LPSPI0_DR;
}

