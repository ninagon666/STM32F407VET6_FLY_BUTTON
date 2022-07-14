#include "lvgl_task.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "lvgl_pic_sample.h"
#include <stdio.h>
#include <rtthread.h>

extern uint16_t ADC_Value[4];
char temp_str[12], ph_str[12], tds_str[12], tbdt_str[12], level_str[12];

static rt_bool_t task1_flag = RT_TRUE;
rt_sem_t loading_sem = RT_NULL;

//tbdt浊度 PH tds temp温度
static lv_obj_t *level_bar, *arc;
lv_obj_t *temp_label, *ph_label, *tbdt_label, *tds_label, 
         *temp_bar,   *ph_bar,   *tbdt_bar,   *tds_bar;

void lv_temp_bar(void)
{
  static lv_style_t style_temp;
  lv_style_init(&style_temp);
  lv_style_set_bg_opa(&style_temp, LV_OPA_COVER);
  lv_style_set_bg_color(&style_temp, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_bg_grad_color(&style_temp, lv_palette_main(LV_PALETTE_RED));
  lv_style_set_bg_grad_dir(&style_temp, LV_GRAD_DIR_HOR);
  
  temp_bar = lv_bar_create(lv_scr_act());
  lv_obj_add_style(temp_bar, &style_temp, LV_PART_INDICATOR);
  lv_obj_set_size(temp_bar, 128, 10);
  lv_bar_set_value(temp_bar, 0, LV_ANIM_OFF);
  lv_obj_set_pos(temp_bar, 0, 16);
}

void lv_tbdt_bar(void)
{
  static lv_style_t style_tbdt;
  lv_style_init(&style_tbdt);
  lv_style_set_bg_opa(&style_tbdt, LV_OPA_COVER);
  lv_style_set_bg_color(&style_tbdt, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_bg_grad_color(&style_tbdt, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_bg_grad_dir(&style_tbdt, LV_GRAD_DIR_HOR);
  
  tbdt_bar = lv_bar_create(lv_scr_act());
  lv_obj_add_style(tbdt_bar, &style_tbdt, LV_PART_INDICATOR);
  lv_obj_set_size(tbdt_bar, 128, 10);
  lv_bar_set_value(tbdt_bar, 0, LV_ANIM_OFF);
  lv_obj_set_pos(tbdt_bar, 0, 45);
}

void lv_tds_bar(void)
{
  static lv_style_t style_tds;
  lv_style_init(&style_tds);
  lv_style_set_bg_opa(&style_tds, LV_OPA_COVER);
  lv_style_set_bg_color(&style_tds, lv_palette_main(LV_PALETTE_LIGHT_GREEN));
  
  tds_bar = lv_bar_create(lv_scr_act());
  lv_obj_add_style(tds_bar, &style_tds, LV_PART_INDICATOR);
  lv_obj_set_size(tds_bar, 128, 10);
  lv_bar_set_value(tds_bar, 0, LV_ANIM_OFF);
  lv_obj_set_pos(tds_bar, 0, 73);
}

void lv_level_bar(void)
{
  static lv_style_t style_level;
  lv_style_init(&style_level);
  lv_style_set_bg_opa(&style_level, LV_OPA_COVER);
  lv_style_set_bg_color(&style_level, lv_palette_main(LV_PALETTE_ORANGE));
  
  level_bar = lv_bar_create(lv_scr_act());
  lv_obj_add_style(level_bar, &style_level, LV_PART_INDICATOR);
  lv_obj_set_size(level_bar, 128, 10);
  lv_bar_set_value(level_bar, 0, LV_ANIM_OFF);
  lv_obj_set_pos(level_bar, 0, 101);
}

void lv_ph_bar(void)
{
  static lv_style_t style_ph;
  lv_style_init(&style_ph);
  lv_style_set_bg_opa(&style_ph, LV_OPA_COVER);
  lv_style_set_bg_color(&style_ph, lv_palette_main(LV_PALETTE_DEEP_ORANGE));
  lv_style_set_bg_grad_color(&style_ph, lv_palette_main(LV_PALETTE_DEEP_PURPLE));
  lv_style_set_bg_grad_dir(&style_ph, LV_GRAD_DIR_HOR);
  
  ph_bar = lv_bar_create(lv_scr_act());
  lv_obj_add_style(ph_bar, &style_ph, LV_PART_INDICATOR);
  lv_obj_set_size(ph_bar, 128, 10);
  lv_bar_set_value(ph_bar, 0, LV_ANIM_OFF);
  lv_obj_set_pos(ph_bar, 0, 125);
}

void lv_loading(void)
{
  arc = lv_arc_create(lv_scr_act());
  lv_obj_set_size(arc, 48, 48);
  lv_arc_set_rotation(arc, 270);
  lv_arc_set_bg_angles(arc, 0, 360);
  lv_obj_remove_style(arc, NULL, LV_PART_KNOB); 
  lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_pos(arc, 40, 96);
  lv_arc_set_range(arc, 0, 100);
  lv_arc_set_value(arc, 0);
}

void lvgl_task_entry(void *parameter)
{
  int32_t i = 0, j = 0;
  char load_str[12];
  
  loading_sem = rt_sem_create("loading", 0, RT_IPC_FLAG_PRIO);
  if (loading_sem == RT_NULL)
  {
      rt_kprintf("create loading semaphore failed.\n");
      return ;
  }
  
  rt_sprintf(load_str, "0%");
  rt_sprintf(ph_str, ":  ");
  rt_sprintf(tbdt_str, "  mg/l");
  rt_sprintf(level_str, "  cm");
  
  lv_init();
  lv_port_disp_init();
  lv_loading();
  
  lv_obj_t * load_label = lv_label_create(lv_scr_act());
  lv_label_set_text(load_label, load_str);
  lv_obj_set_pos(load_label, 54, 144);
  
  lv_obj_t * img1 = lv_img_create(lv_scr_act());
  lv_img_set_src(img1, &logo);
  lv_obj_set_pos(img1, 0, 0);
  
  while(1)
  {
    lv_task_handler();
    rt_thread_mdelay(10);
    if(task1_flag) {
      if(++i >= 6) {
        if(++j > 50) {
          lv_obj_del(img1);
          lv_obj_del(load_label);
          lv_obj_del(arc);
          i = 0;
          rt_sem_release(loading_sem);
          task1_flag = RT_FALSE;
        }
        lv_arc_set_value(arc, j * 2);
        rt_sprintf(load_str, "%d%", j * 2);
        lv_label_set_text(load_label, load_str);
      }
    }
    else {
        lv_obj_t * img2 = lv_img_create(lv_scr_act());
        lv_img_set_src(img2, &background);
        lv_obj_set_pos(img2, 0, 0);
      
        temp_label = lv_label_create(lv_scr_act());
        lv_label_set_text(temp_label, temp_str);
        lv_obj_set_pos(temp_label, 36, 0);
      
        tbdt_label = lv_label_create(lv_scr_act());
        lv_label_set_text(tbdt_label, tbdt_str);
        lv_obj_set_pos(tbdt_label, 36, 28);
      
        tds_label = lv_label_create(lv_scr_act());
        lv_label_set_text(tds_label, tds_str);
        lv_obj_set_pos(tds_label, 32, 56);
      
        lv_obj_t * level_label = lv_label_create(lv_scr_act());
        lv_label_set_text(level_label, level_str);
        lv_obj_set_pos(level_label, 36, 85);
      
        ph_label = lv_label_create(lv_scr_act());
        lv_label_set_text(ph_label, tds_str);
        lv_obj_set_pos(ph_label, 28, 110);
      
        lv_level_bar();
        lv_ph_bar();
        lv_tds_bar();
        lv_tbdt_bar();
        lv_temp_bar();
        
        while(1) {
          static int32_t level_value = 0;
          lv_task_handler();
          rt_thread_mdelay(10);
          if(++i >= 20) {
            level_value = (int32_t)(ADC_Value[0] * 100 / 4096);
            lv_bar_set_value(level_bar, level_value, LV_ANIM_OFF);
            sprintf(level_str, "%.1fmm", ADC_Value[0] / 10.);
            lv_label_set_text(level_label, level_str);
            i = 0;
          };
        }
    }
  }
}

void lvgl_task_init(void)
{
  rt_thread_t tid;

  //创建lvgl线程 优先级设置为10
  tid = rt_thread_create("lvgl task", lvgl_task_entry, RT_NULL, 4096, 8, 5);

  //启动lvgl线程
  if (RT_NULL != tid)
      rt_thread_startup(tid);
}