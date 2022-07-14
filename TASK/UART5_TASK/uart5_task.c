#include "uart5_task.h"
#include "main.h"
#include "lvgl.h"
#include <stdio.h>

#define UART5_TDS_LEN 10 //TDS值ringbuffer的长度

extern UART_HandleTypeDef huart5;
extern char tds_str[12], temp_str[12];
extern lv_obj_t *temp_label, *tds_label, *temp_bar, *tds_bar;

rt_sem_t uart5_tds_sem = NULL;       
rt_ringbuffer_t uart5_tds_cb_t;

float unitize(float max, float min, float value) 
{
	float val = value;
	
	val = (val > max) ? max : val;
	val = (val < min) ? min : val;//限缩
	val = (val - min) / (max - min);
	
	return val;
}

/* enter interrupt call it*/
void uart5_it_process(void)
{
    static rt_bool_t uart5_flag = RT_FALSE; 
    static rt_uint8_t uart5_times = 0;
    int ch = -1;
    
    if ((__HAL_UART_GET_FLAG(&(huart5), UART_FLAG_RXNE) != RESET) &&
        (__HAL_UART_GET_IT_SOURCE(&(huart5), UART_IT_RXNE) != RESET))
    {
        while (1)
        {
            ch = -1;
            if (__HAL_UART_GET_FLAG(&(huart5), UART_FLAG_RXNE) != RESET)
                ch =  huart5.Instance->DR & 0xff;
            if(uart5_flag) {
              uart5_times++;
              /* 读取到数据，将数据存入 ringbuffer */
              rt_ringbuffer_putchar(uart5_tds_cb_t, ch);
              if(uart5_times == 6) {
                uart5_flag = RT_FALSE;
                rt_sem_release(uart5_tds_sem);
              } //计数6次，释放信号量
              break;
            }
            if(ch == 0x55) {
                uart5_flag = RT_TRUE;// 读到帧头，开始计数
                uart5_times = 0;
                break;
              }
            else if (ch == -1)
                break;
        }
    }
}

void uart5_task_entry(void *parameter)
{
    int32_t temp_show, tds_show;
    
    rt_sprintf(temp_str, "  C");
    rt_sprintf(tds_str, "  ppm");
    while(1) 
    {
        rt_sem_take(uart5_tds_sem, RT_WAITING_FOREVER);
        rt_uint8_t uart5_tds_len = 0;
        rt_uint8_t uart5_tds_temp[UART5_TDS_LEN] = {0};
        
        uart5_tds_len = rt_ringbuffer_data_len(uart5_tds_cb_t);
        //有数据则进入
        if(uart5_tds_len > 0) {
          double sensor_temp = 0.;
          uint16_t sensor_tds = 0;
          
          rt_ringbuffer_get(uart5_tds_cb_t, uart5_tds_temp, uart5_tds_len);
          if((0x55 + uart5_tds_temp[0] + uart5_tds_temp[1] + uart5_tds_temp[2] + uart5_tds_temp[3]) == (uart5_tds_temp[4] * 256 + uart5_tds_temp[5])) {
             sensor_temp = (uart5_tds_temp[0] * 256. + uart5_tds_temp[1] * 1.) / 10.;	  //计算温度 单位是0.1度 若temp=235  则温度是23.5度
             sensor_tds = (uart5_tds_temp[2] * 256 + uart5_tds_temp[3]) / 10;    //计算TDS   单位是1ppm
             temp_show = (int32_t)(100 * unitize(60., 0., sensor_temp));
             tds_show = (int32_t)(100 * unitize(1000., 0., sensor_tds));
          }	//验证校验和
          sprintf(temp_str, "%.2lfC", sensor_temp);
          sprintf(tds_str, "%dppm", sensor_tds);
        }
        else {
          sprintf(temp_str, "none");
          sprintf(tds_str, "none");
        }
        lv_label_set_text(temp_label, temp_str);
        lv_label_set_text(tds_label, tds_str);
        lv_bar_set_value(temp_bar, temp_show, LV_ANIM_OFF);
        lv_bar_set_value(tds_bar, tds_show, LV_ANIM_OFF);
    }
}

void uart5_task_init(void)
{
    rt_thread_t tid;

    /* 创建串口接收 ringbuffer  */
    uart5_tds_cb_t = rt_ringbuffer_create(UART5_TDS_LEN);
    /* 创建串口接收数据的信号量 */
    uart5_tds_sem = rt_sem_create("uart5 tds", 0, 0);
    if (uart5_tds_sem == RT_NULL) {
        rt_kprintf("create uart5 tds semaphore failed.\n");
        return ;
    }
    
    //创建串口3线程 优先级设置为22
    tid = rt_thread_create("uart5 tds task ", uart5_task_entry, RT_NULL, 1024, 22, 5);

    //启动串口3线程
    if (RT_NULL != tid)
        rt_thread_startup(tid);
    
    __HAL_UART_ENABLE_IT(&huart5, UART_IT_RXNE);//使能串口中断
}

