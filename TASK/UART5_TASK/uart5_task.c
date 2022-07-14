#include "uart5_task.h"
#include "main.h"
#include "lvgl.h"
#include <stdio.h>

#define UART5_TDS_LEN 10 //TDSֵringbuffer�ĳ���

extern UART_HandleTypeDef huart5;
extern char tds_str[12], temp_str[12];
extern lv_obj_t *temp_label, *tds_label, *temp_bar, *tds_bar;

rt_sem_t uart5_tds_sem = NULL;       
rt_ringbuffer_t uart5_tds_cb_t;

float unitize(float max, float min, float value) 
{
	float val = value;
	
	val = (val > max) ? max : val;
	val = (val < min) ? min : val;//����
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
              /* ��ȡ�����ݣ������ݴ��� ringbuffer */
              rt_ringbuffer_putchar(uart5_tds_cb_t, ch);
              if(uart5_times == 6) {
                uart5_flag = RT_FALSE;
                rt_sem_release(uart5_tds_sem);
              } //����6�Σ��ͷ��ź���
              break;
            }
            if(ch == 0x55) {
                uart5_flag = RT_TRUE;// ����֡ͷ����ʼ����
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
        //�����������
        if(uart5_tds_len > 0) {
          double sensor_temp = 0.;
          uint16_t sensor_tds = 0;
          
          rt_ringbuffer_get(uart5_tds_cb_t, uart5_tds_temp, uart5_tds_len);
          if((0x55 + uart5_tds_temp[0] + uart5_tds_temp[1] + uart5_tds_temp[2] + uart5_tds_temp[3]) == (uart5_tds_temp[4] * 256 + uart5_tds_temp[5])) {
             sensor_temp = (uart5_tds_temp[0] * 256. + uart5_tds_temp[1] * 1.) / 10.;	  //�����¶� ��λ��0.1�� ��temp=235  ���¶���23.5��
             sensor_tds = (uart5_tds_temp[2] * 256 + uart5_tds_temp[3]) / 10;    //����TDS   ��λ��1ppm
             temp_show = (int32_t)(100 * unitize(60., 0., sensor_temp));
             tds_show = (int32_t)(100 * unitize(1000., 0., sensor_tds));
          }	//��֤У���
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

    /* �������ڽ��� ringbuffer  */
    uart5_tds_cb_t = rt_ringbuffer_create(UART5_TDS_LEN);
    /* �������ڽ������ݵ��ź��� */
    uart5_tds_sem = rt_sem_create("uart5 tds", 0, 0);
    if (uart5_tds_sem == RT_NULL) {
        rt_kprintf("create uart5 tds semaphore failed.\n");
        return ;
    }
    
    //��������3�߳� ���ȼ�����Ϊ22
    tid = rt_thread_create("uart5 tds task ", uart5_task_entry, RT_NULL, 1024, 22, 5);

    //��������3�߳�
    if (RT_NULL != tid)
        rt_thread_startup(tid);
    
    __HAL_UART_ENABLE_IT(&huart5, UART_IT_RXNE);//ʹ�ܴ����ж�
}

