#include <microkit.h>

extern uintptr_t uart_base;

#define REG_PTR(offset) ((volatile uint32_t *)((uart_base) + (offset)))

#ifdef CONFIG_PLAT_IMX8MM_EVK

#define UART_STATUS 0x98
#define TRANSMIT 0x40
#define STAT_TDRE (1 << 14)

void _putchar(char character)
{
    while (!(*REG_PTR(UART_STATUS) & STAT_TDRE)) { }
    *REG_PTR(TRANSMIT) = character;
}

#endif

#ifdef CONFIG_PLAT_ODROIDC4

#define UART_STATUS 0xC
#define UART_WFIFO 0x0
#define UART_TX_FULL (1 << 21)

void _putchar(char character)
{
    while ((*REG_PTR(UART_STATUS) & UART_TX_FULL)) {}
    *REG_PTR(UART_WFIFO) = character & 0x7f;
}

#endif

#ifdef CONFIG_PLAT_PC99

void _putchar(char character)
{
    microkit_mr_set(0, (uint64_t)character);
    microkit_ppcall(13, microkit_msginfo_new(0, 1));
}

#endif
