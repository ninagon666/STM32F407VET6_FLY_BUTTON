#include "my_cmd.h"
#include "main.h"

void reboot(void)
{
    NVIC_SystemReset();

    return ;
}
FINSH_FUNCTION_EXPORT(reboot, reboot the board);
MSH_CMD_EXPORT(reboot, reboot the board);

static long clear(void)
{
    rt_kprintf("\x1b[2J\x1b[H");

    return 0;
}
FINSH_FUNCTION_EXPORT(clear,clear the terminal screen);
MSH_CMD_EXPORT(clear,clear the terminal screen);

void usart1_enter(void)
{
    printf("\n");
    return;
}
FINSH_FUNCTION_EXPORT(usart1_enter, uasrt1 test);
MSH_CMD_EXPORT(usart1_enter, uasrt1 test);

void usart1_test(int argc, char**argv)
{
    if (argc < 2)
    {
        rt_kprintf("Please input suck like 'usart1_test xxx");
        return;
    }
    
    printf("%s", argv[1]);
    return;
}
FINSH_FUNCTION_EXPORT(usart1_test, uasrt1 test);
MSH_CMD_EXPORT(usart1_test, uasrt1 test);