#include "usart1_task.h"
#include "main.h"
#include "lvgl.h"
#include <stdio.h>
#include <math.h>

#define USART1_TBDT_LEN 13 //浊度ringbuffer的长度

extern UART_HandleTypeDef huart1;
extern char tbdt_str[12];
extern lv_obj_t *tbdt_label, *tbdt_bar;

rt_sem_t usart1_tbdt_sem = NULL;       
rt_ringbuffer_t usart1_tbdt_cb_t;

/* enter interrupt call it*/
void usart1_it_process(void)
{
    static rt_bool_t usart1_flag = 0;
    int ch = -1;
    
    if ((__HAL_UART_GET_FLAG(&(huart1), UART_FLAG_RXNE) != RESET) &&
        (__HAL_UART_GET_IT_SOURCE(&(huart1), UART_IT_RXNE) != RESET))
    {
        while (1)
        {
            ch = -1;
            if (__HAL_UART_GET_FLAG(&(huart1), UART_FLAG_RXNE) != RESET)
                ch =  huart1.Instance->DR & 0xff;
            if(ch == 'K') {
                usart1_flag = RT_TRUE;//标志位，用于记录是否判断到OK的'K'
                break;
              }
            else if(ch == '\n') {
                if(usart1_flag) {
                  rt_sem_release(usart1_tbdt_sem);// 读到帧尾，释放信号量
                  usart1_flag = RT_FALSE;
                }
                break;
            } 
            else if(ch == '\r')//舍去换行符
                break;
            else if(ch == 'O')//舍去O
                break;
            else if (ch == -1)
                break;
            /* 读取到数据，将数据存入 ringbuffer */
            rt_ringbuffer_putchar(usart1_tbdt_cb_t, ch);
        }
    }
}

void usart1_task_entry(void *parameter)
{
    float tbdt_value = 0.;
    int32_t tbdt_show = 0;
  
    rt_thread_mdelay(333);
    usart1_putstr((uint8_t *)"AT+V\r\n");
    
    while(1) 
    {
        rt_sem_take(usart1_tbdt_sem, RT_WAITING_FOREVER);
        rt_uint8_t usart1_tbdt_len = 0;
        rt_uint8_t usart1_tbdt_temp[USART1_TBDT_LEN] = {0};
        
        usart1_tbdt_len = rt_ringbuffer_data_len(usart1_tbdt_cb_t);
        //有数据则进入
        if(usart1_tbdt_len > 0) {
          rt_ringbuffer_get(usart1_tbdt_cb_t, usart1_tbdt_temp, usart1_tbdt_len);
          tbdt_value = (usart1_tbdt_temp[3] - '0') * 1.
                   + (usart1_tbdt_temp[5] - '0') * 0.1
                   + (usart1_tbdt_temp[6] - '0') * 0.01;
          tbdt_show  = (int32_t)(tbdt_value * 100. / 4.5);
          tbdt_value = (-1120.4 * pow(tbdt_value, 2)) + 5742.3 * tbdt_value - 4352.9;
          sprintf(tbdt_str, "%.2fmg/l", tbdt_value);
        }
        else {
          sprintf(tbdt_str, "none");
        }
        lv_label_set_text(tbdt_label, tbdt_str);
        lv_bar_set_value(tbdt_bar, 100 - tbdt_show, LV_ANIM_OFF);
        rt_thread_mdelay(333);
        usart1_putstr((uint8_t *)"AT+V\r\n");
    }
}

void usart1_task_init(void)
{
    rt_thread_t tid;

    /* 创建串口接收 ringbuffer  */
    usart1_tbdt_cb_t = rt_ringbuffer_create(USART1_TBDT_LEN);
    /* 创建串口接收数据的信号量 */
    usart1_tbdt_sem = rt_sem_create("usart1 turbidity", 0, 0);
    if (usart1_tbdt_sem == RT_NULL) {
        rt_kprintf("create usart1 turbidity semaphore failed.\n");
        return ;
    }
    
    //创建串口1线程 优先级设置为22
    tid = rt_thread_create("usart1 turbidity task ", usart1_task_entry, RT_NULL, 1536, 22, 5);

    //启动串口1线程
    if (RT_NULL != tid)
        rt_thread_startup(tid);
    
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);//使能串口中断
}
