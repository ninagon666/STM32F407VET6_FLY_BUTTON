#include "usart.h"
#include "stm32f4xx_hal_uart.h"
#include "my_usart.h"

void usart1_putchar(uint8_t data)
{
    while(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC) != SET)
      ;   //�ȴ����ͽ���
    HAL_UART_Transmit(&huart1, &data, 1, 1000);//�򴮿�1��������
}
void usart2_putchar(uint8_t data)
{
    while(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_TC) != SET)
      ;   //�ȴ����ͽ���
    HAL_UART_Transmit(&huart2, &data, 1, 1000);//�򴮿�2��������
}

void usart3_putchar(uint8_t data)
{
    while(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_TC) != SET)
      ;   //�ȴ����ͽ���
    HAL_UART_Transmit(&huart3, &data, 1, 1000);//�򴮿�3��������
}
void uart4_putchar(uint8_t data)
{
    while(__HAL_UART_GET_FLAG(&huart4,UART_FLAG_TC) != SET)
      ;   //�ȴ����ͽ���
    HAL_UART_Transmit(&huart4, &data, 1, 1000);//�򴮿�4��������
}

void uart5_putchar(uint8_t data)
{
    while(__HAL_UART_GET_FLAG(&huart5,UART_FLAG_TC) != SET)
      ;   //�ȴ����ͽ���
    HAL_UART_Transmit(&huart5, &data, 1, 1000);//�򴮿�5��������
}
void usart6_putchar(uint8_t data)
{
    while(__HAL_UART_GET_FLAG(&huart6,UART_FLAG_TC) != SET)
      ;   //�ȴ����ͽ���
    HAL_UART_Transmit(&huart6, &data, 1, 1000);//�򴮿�6��������
}

void usart1_putstr(const uint8_t *str)
{
    while (*str)
    {
        usart1_putchar(*str++);
    }
}
void usart2_putstr(const uint8_t *str)
{
    while (*str)
    {
        usart2_putchar(*str++);
    }
}

void usart3_putstr(const uint8_t *str)
{
    while (*str)
    {
        usart3_putchar(*str++);
    }
}
void uart4_putstr(const uint8_t *str)
{
    while (*str)
    {
        uart4_putchar(*str++);
    }
}

void uart5_putstr(const uint8_t *str)
{
    while (*str)
    {
        uart5_putchar(*str++);
    }
}

void usart6_putstr(const uint8_t *str)
{
    while (*str)
    {
        usart6_putchar(*str++);
    }
}
