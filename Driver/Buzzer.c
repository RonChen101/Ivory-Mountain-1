#include "Buzzer.h"
#include "GPIO.h"
#include "NVIC.h"
#include "Switch.h" // 配置外设引脚
#include "STC8H_PWM.h"

#define BUZZER P34

//			 C	 D     E 	F	 G	  A	   B	 C`
static u16 code hz2[] = {523, 587, 659, 698, 784, 880, 988, 1047};

//			             C`	   D`     E`   F`	  G`	A`	  B`    C``
static u16 code hz[] = {1047, 1175, 1319, 1397, 1568, 1760, 1976, 2093};

static void GPIO_config(void) {
    GPIO_InitTypeDef	GPIO_InitStructure;		//结构定义
    GPIO_InitStructure.Pin  = GPIO_Pin_4;		//指定要初始化的IO,
    GPIO_InitStructure.Mode = GPIO_OUT_PP;	    //指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
    GPIO_Inilize(GPIO_P3, &GPIO_InitStructure);//初始化
}

void	PWM_config(u16 hz_output)
{
	PWMx_InitDefine		PWMx_InitStructure;
    
    u16 period = (MAIN_Fosc / hz_output);
	
	// 配置PWM8
	PWMx_InitStructure.PWM_Mode    		= CCMRn_PWM_MODE1;	//模式,		CCMRn_FREEZE,CCMRn_MATCH_VALID,CCMRn_MATCH_INVALID,CCMRn_ROLLOVER,CCMRn_FORCE_INVALID,CCMRn_FORCE_VALID,CCMRn_PWM_MODE1,CCMRn_PWM_MODE2
	PWMx_InitStructure.PWM_Duty   	 	= 0.0;	            //PWM占空比时间, 0~Period
	PWMx_InitStructure.PWM_EnoSelect    = ENO8P;			//输出通道选择,	ENO1P,ENO1N,ENO2P,ENO2N,ENO3P,ENO3N,ENO4P,ENO4N / ENO5P,ENO6P,ENO7P,ENO8P
	PWM_Configuration(PWM8, &PWMx_InitStructure);			//初始化PWM,  PWMA,PWMB

	// 配置PWMB
	PWMx_InitStructure.PWM_Period   = period - 1;			//周期时间,   0~65535
	PWMx_InitStructure.PWM_DeadTime = 0;					//死区发生器设置, 0~255
	PWMx_InitStructure.PWM_MainOutEnable= ENABLE;			//主输出使能, ENABLE,DISABLE
	PWMx_InitStructure.PWM_CEN_Enable   = ENABLE;			//使能计数器, ENABLE,DISABLE
	PWM_Configuration(PWMB, &PWMx_InitStructure);			//初始化PWM通用寄存器,  PWMA,PWMB

	// 切换PWM通道
	PWM8_SW(PWM8_SW_P34);					//PWM5_SW_P20,PWM5_SW_P17,PWM5_SW_P00,PWM5_SW_P74

	// 初始化PWMB的中断
	NVIC_PWM_Init(PWMB,DISABLE,Priority_0);
}


void Buzzer_init(void){
    EAXSFR(); /* 扩展寄存器使能 */
    GPIO_config();
    PWM_config(1000); // 不能省略
}

// 按照指定频率播放声音
void Buzzer_play(u16 hz_value){
//    PWM_config(hz_value);
    u16 period = (MAIN_Fosc / hz_value);
    u16 duty = period * 0.05f; // 这里的0.05可以决定音量大小
    
    // 设置PWMB输出Period (1/频率)
    PWMB_AutoReload(period - 1);	//周期设置
    // 设置PWM8的占空比
    PWMB_Duty8(duty);
    // 启用PWMB通道8的输出使能
    PWMB_CC8E_Enable();
}

// 按照指定音调播放声音
void Buzzer_beep(u16 tone){ // 1,2,3,4,5,6,7,8
    // tone -> idx -> hz
    Buzzer_play(hz[tone - 1]);
}

void Buzzer_stop(void){
    
//    PWMB_BrakeOutputEnable(DISABLE); // 关闭PWMB输出使能
    PWMB_CC8E_Disable();
}

//#include "Delay.h"
//void delay_X_ms(unsigned int ms){
//    while(ms--) delay_ms(1);
//}
void Buzzer_alarm() {  // 警告声
	// 659 * 2
	Buzzer_play(659 * 2);
	os_wait2(K_TMO, 20); 
	Buzzer_stop();
	os_wait2(K_TMO, 10); 
	
	// 587 * 2
	Buzzer_play(587 * 2);
	os_wait2(K_TMO, 20); 
	Buzzer_stop();
	os_wait2(K_TMO, 10); 
	
	// 523 * 2
	Buzzer_play(523 * 2);
	os_wait2(K_TMO, 20);
	Buzzer_stop();
	os_wait2(K_TMO, 10); 	
}