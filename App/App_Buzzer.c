#include "App.h"
#include "Buzzer.h"


void Buzzer_Task() _task_ BUZZER_TASK_ID
{
    while(1){
        os_wait2(K_SIG,5);
        Buzzer_alarm();
    }
}

