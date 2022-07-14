#include "usart3_task.h"
#include "main.h"
#include "lvgl.h"
#include <stdio.h>

#define USART3_PH_LEN 32 //PH值ringbuffer的长度

extern UART_HandleTypeDef huart3;
extern char ph_str[12];
extern lv_obj_t *ph_label, *ph_bar;

rt_sem_t usart3_ph_sem = NULL;       
rt_ringbuffer_t usart3_ph_cb_t;

/* enter interrupt call it*/
void usart3_it_process(void)
{
    static rt_bool_t usart3_flag = 0;
    int ch = -1;
    
    if ((__HAL_UART_GET_FLAG(&(huart3), UART_FLAG_RXNE) != RESET) &&
        (__HAL_UART_GET_IT_SOURCE(&(huart3), UART_IT_RXNE) != RESET))
    {
        while (1)
        {
            ch = -1;
            if (__HAL_UART_GET_FLAG(&(huart3), UART_FLAG_RXNE) != RESET)
                ch =  huart3.Instance->DR & 0xff;
            if(ch == 'K') {
                usart3_flag = RT_TRUE;//标志位，用于记录是否判断到OK的'K'
                break;
              }
            else if(ch == '\n') {
                if(usart3_flag) {
                  rt_sem_release(usart3_ph_sem);// 读到帧尾，释放信号量
                  usart3_flag = RT_FALSE;
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
            rt_ringbuffer_putchar(usart3_ph_cb_t, ch);
        }
    }
}

void usart3_task_entry(void *parameter)
{
    int32_t ph_show = 0;
  
    rt_thread_mdelay(333);
    usart3_putstr((uint8_t *)"AT+PH\r\n");
    
    while(1) 
    {
        float ph_value = 0.;
        rt_sem_take(usart3_ph_sem, RT_WAITING_FOREVER);
        rt_uint8_t usart3_ph_len = 0;
        rt_uint8_t usart3_ph_temp[USART3_PH_LEN] = {0};
        
        usart3_ph_len = rt_ringbuffer_data_len(usart3_ph_cb_t);
        //有数据则进入
        if(usart3_ph_len > 0) {
          rt_ringbuffer_get(usart3_ph_cb_t, usart3_ph_temp, usart3_ph_len);
          if(usart3_ph_len == 8) {
            ph_value = (usart3_ph_temp[4] - '0') * 1.
                     + (usart3_ph_temp[6] - '0') * 0.1
                     + (usart3_ph_temp[7] - '0') * 0.01;
          }
          else if (usart3_ph_len == 9) {
            ph_value = (usart3_ph_temp[4] - '0') * 10.
                     + (usart3_ph_temp[5] - '0') * 1.
                     + (usart3_ph_temp[7] - '0') * 0.1
                     + (usart3_ph_temp[8] - '0') * 0.01;
          }
          ph_show  = (int32_t)(ph_value * 100. / 14.0);
          sprintf(ph_str, "%.2f", ph_value);
        }
        else {
           sprintf(ph_str, "none");
        }
        lv_label_set_text(ph_label, ph_str);
        lv_bar_set_value(ph_bar, ph_show, LV_ANIM_OFF);
        rt_thread_mdelay(333);
        usart3_putstr((uint8_t *)"AT+PH\r\n");
    }
}

void usart3_task_init(void)
{
    rt_thread_t tid;

    /* 创建串口接收 ringbuffer  */
    usart3_ph_cb_t = rt_ringbuffer_create(USART3_PH_LEN);
    /* 创建串口接收数据的信号量 */
    usart3_ph_sem = rt_sem_create("usart3 ph", 0, 0);
    if (usart3_ph_sem == RT_NULL) {
        rt_kprintf("create usart3 ph semaphore failed.\n");
        return ;
    }
    
    //创建串口3线程 优先级设置为22
    tid = rt_thread_create("usart3 ph task ", usart3_task_entry, RT_NULL, 1536, 22, 5);

    //启动串口3线程
    if (RT_NULL != tid)
        rt_thread_startup(tid);
    
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);//使能串口中断
}
