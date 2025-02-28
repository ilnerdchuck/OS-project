#pragma once


//TODO: define all memory regions
//FLASH
//TODO: ATTENZIONE DA FARE PRESENTE AL PROF S32K356 non ha flash 3
//TODO: sia della flash che della ram ci sono piu blocchi 
//che facciamo li dividiamo?
#define S32K3x8_FLASH0_BASE 0x00400000
#define S32K3x8_FLASH0_SIZE 2048*1024

//SRAM
#define S32K3x8_SRAM0_BASE 0x20400000
#define S32K3x8_SRAM0_SIZE 256*1024
