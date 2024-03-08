#ifndef UART_H
#define UART_H

extern char  uartFormat;

void UART_init(void);
void UART_task(void);
void UART_put(char c);

#endif