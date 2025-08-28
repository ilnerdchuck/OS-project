#ifndef __PRINTF__
#define __PRINTF__

#include "FreeRTOS.h"
#include <stdint.h>

#define UART0_ADDRESS                         ( 0x40328000UL )
#define UART0_DATA                            ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 4UL ) ) ) )
#define UART0_STATE                           ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 0UL ) ) ) )
#define UART0_CTRL                            ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 0xC ) ) ) )
#define UART0_BAUDDIV                         ( *( ( ( volatile uint32_t * ) ( UART0_ADDRESS + 8UL ) ) ) )

void UART_init(void);
void UART_printf(const char *s);

#define LPSPI0_BASE                              (0x40358000UL)
/** Peripheral LPSPI0 base pointer */
#define LPSPI0_DR                              ( *( ( ( volatile uint32_t * ) ( LPSPI0_BASE + 0xC ) ) ) )
#define LPSPI0_CTRL                            ( *( ( ( volatile uint32_t * ) ( LPSPI0_BASE + 0x0 ) ) ) )


void SPI_init( void );
void SPI_write(uint8_t);
void SPI_get(uint8_t *);
#endif
