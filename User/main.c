#include "config.h"
#include "App.h"



// void main_task() _task_ 0 {
//     sys_init();

//     os_create_task(KEY_TASK_ID);
//     os_create_task(UART1_TASK_ID);
//     os_create_task(UART2_TASK_ID);

//     while (1) {
//         os_wait(K_TMO, 100, 0);   // 常驻，定期让出 CPU
//     }
// }
void main_task() _task_ 0 {
    sys_init();   // 这里只留最基础的初始化
	os_create_task(KEY_TASK_ID);
	os_delete_task(0);
}








