/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : readme.txt
* Author             : MCD Application Team
* Version            : V2.0
* Date               : 05/23/2008
* Description        : This sub-directory contains all the user-modifiable files 
*                      needed to create a new project linked with the STM32F10x  
*                      Firmware Library and working with RealView Microcontroller
*                      Development Kit(RVMDK) software toolchain (Version 3.21 and later).
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
* FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED 
* IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
*******************************************************************************/

Directory contents
===================
- Project.Uv2/.Opt: A pre-configured project file with the provided library structure
                    that produces an executable image with RVMDK.
              
- stm32f10x_vector.s: This file contains the vector table for STM32F10x, Stack
                      and Heap definition. User can also enable the use of external
                      SRAM mounted on STM3210E-EVAL board as data memory.                
                      
- cortexm3_macro.s: Instruction wrappers for special Cortex-M3 instructions. 

- note.txt        : Steps to follow when using the default startup file provided 
                    by RVMDK when creating new projects.
      
How to use it
=============
- Open the Project.Uv2 project
- In the build toolbar select the project config:
    - STM3210B-EVAL: to configure the project for STM32 Medium-density devices
    - STM3210E-EVAL: to configure the project for STM32 High-density devices
- Rebuild all files: Project->Rebuild all target files
- Load project image: Debug->Start/Stop Debug Session
- Run program: Debug->Run (F5)

NOTE:
 - Medium-density devices are STM32F101xx and STM32F103xx microcontrollers where
   the Flash memory density ranges between 32 and 128 Kbytes.
 - High-density devices are STM32F101xx and STM32F103xx microcontrollers where
   the Flash memory density ranges between 256 and 512 Kbytes.
    
******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE******

******************* (C) COPYRIGHT 2009 MiNiSoft Lab. ***************************

�Ѿ������I/O�ܽţ�

1��	MCDAC��� stm32f10x_MCdac.c  ������TIM3�� PB0,PB1ΪDAC����š�

2��	TIMER2ȱʡ����Ϊ ENCODER�Ĳɼ����ܽŶ���ο�void ENC_Init(void)������	ͬʱ��Ӧ��
    �жϴ�����Ҳ�������ˡ�   stm32f10x_encoder.c�ļ��ʹ��TIM2->PA0,PA1,ʹ��TIM3->
	PA6,PA7,ʹ��TIM4->PB6,PB7��

3��	TIMER2ȱʡ����Ϊ hall�Ĳɼ����ܽŶ���ο�void HALL_HallTimerInit(void)������ͬʱ
    �ж϶�λΪinput capture and Update (overflow) events generate interrupt��
	stm32f10x_hall.c�ļ��ʹ��TIM2->PA0,PA1,PA2��ʹ��TIM3->PA6,PA7,PB0��ʹ��TIM4->PB6
	PB7��PB8���жϴ���������ENCODERʹ�����Ͳ�ͬ��

4�� MC_pwm_1shunt_prm.h��MC_pwm_3shunt_prm.h�ļ�����������������ADCͨ���Լ��¶Ⱥ�
    ֱ��ĸ�ߵ�ѹ���ADCͨ���� ����ʹ�õ����ߵ�����⣬�������ʹ��ͨ��Ϊ��PC0��PC1��PC2��
	ֱ��ĸ�ߵ�ѹ���->PA3,�¶�ʹ��->PC0����λ��������PC4����10K��λ����

7�� stm32f10x_Timebase.c�ļ��ﶨ��������ϵͳ��ʱ�ӣ�������������void SysTickHandler(void)
    �жϺ��������жϺ����ڲ��ṩ��Ť�غʹ�ͨ��PID���ơ�����MOTOR���ٶȸı䡣

8�� STM32x_svpwm_1shunt.c��STM32x_svpwm_3shunt.c�Լ�STM32x_svpwm_ics.c����ֱ�����
    ������������6��PWMͨ������Լ�ʹ��DMA��ʽ�Ķ��壬ADC�Ĺ�����ʽ��TIM1��4��ͨ����
	������ʽ�Լ��ж�������ʹ�ܡ�����ο�void SVPWM_1ShuntInit(void)��void SVPWM_3ShuntInit(void)

9�� stm32f10x_it.c�ļ��ﶨ����void ADC1_2_IRQHandler(void)�ж��������������������״̬��
    ����ʹ��HALLʱ��ֱ�Ӱ�״̬��ΪRUN������2���и�����������һ��������ʽ������������������
	2���ж�������������void TIM1_BRK_IRQHandler(void)ɲ���жϺ�void TIM1_UP_IRQHandler(void)
	TIM1 overflow and update interrupt��

10�� stm32f10x_lcd.c�ļ��ﶨ����LCD�����Ĺܽš�оƬ�ͺ��Լ����ݶ�д��������Ҫ����غ���ת��
    ��ʹ��FSMC���������ù����Ѿ���ɡ���

11��PC0-PC4->A/Dת����PC4Ϊ��λ���������롣 PA3Ϊֱ��ĸ�ߵ�ѹ��⡣
    PA0-PA2ȱʡΪTIM2�������룬ENCODER���롣PE15ΪTIM1��ɲ����������ˣ�����ͣ����ť��
	PD13Ϊ������ɲ�����裬�����������PD10ΪNTC������·�̵���������������ƣ�����PFC������
	PB0��PB1ΪTIM3��2��PWMͨ�����������D/Aת����PE8-PE13ΪTIM1��6������PWMͨ��������6��
	�űۣ�����PE9��PE11��PE13���ű۵����Ŷ�����������ֹ�����ܷɣ��յ�����ģ�顣PE14ΪTIM1
	�ĵ�4ͨ�����ṩA/D�ɼ��жϴ������ڡ�PE8-PE14ΪREMAP���Ź��ܡ�

12�� �������壺��-PD8����-PD14����-PE1����-PE0��SEL-PD12��USER-PB9��  
13�� ʹ��FSMCʱ������ʹ��TIM4_CH2ͨ����USART2��CK�źų�ͻ��
=================================================================================================
14�� ��ţ��4���������壺PA0��PA8��PC13��PD3��

15�� Ϊ��ʹ��TIM2��CH1-CH3ͨ������Ҫȡ��ST3232оƬ��ʹ�ùܽŶ���Ϊ��PA0-PA2��
     PA1��ΪTFTģ���LED���⡣

16�� ȡ��ST3232��ʹ��PA8-PA11������ʹ��USB���ˣ�ʹ��TIM1��CH1-CH4��
     
17�� ʹ��PB0��PB1����ΪTIM3��CH3��CH4����Ϊ����D/Aת�������

18�� PC0-PC6����7��A/Dת��ͨ������ʹ�ã�PC3�Ѿ�����Ϊ��λ����

19�� ȡ��3.2TFT��Ļ�ϵ�ADS7843��������PB12-PB15����ΪTIM1��BKIN��CH1N-CH3N��

20�� ���ȡ��AT45DB161D������ʹ��PA6��PA7��PB0��PB1��ΪTIM1��BKIN��CH1N-CH3N��

21�� ����֮�⣬����Ҫ8-10��������������6��������2��NTC��·�̵����Ͷ�����ɲ������
     �̵�����
=========================================================
ʹ��MINISTM32F103RBT6�İ��Ӳ���PMSM��������Ľ���˳��

1��TIM1BKIN--------PB12      TIM1CH1-----------PA8
   TIM1CHIN--------PB13      TIM1CH2-----------PA9
   TIM1CH2N--------PB14      TIM1CH3-----------PA10
   TIM1CH3N--------PB15      TIM1CH4-----------PA11
   (REMAP���ܣ�

2��TIM2������ͨ������ = PA0��PA1��PA2 (û��FT,ע����DIODE����)
   TIM3�Ķ���ͨ����� = PB4��PB5 ����ΪDAת����PWMԴ ��REMAP���ܣ�
   TIM4�Ķ���ͨ������ = PB8��PB9  ---------CH3��CH4  
   ע��:���ǿ���ʹ��A/D�Ŀ���,������FT��.
3��PA3��PA4��PA5Ϊ����A/Dת��������2�������ĸ�ߵ�ѹ, PA6Ϊɢ��
   �¶�,PA7Ϊ�ⲿģ���ѹ����,PB0,PB1Ϊ����A/D����.

4��������Ϊͨ��I/O�Ŀ���Ϊ��PB10��PB11��PA13��PA14��PA15��PB3����Ϊ
   �������������״ָ̬ʾ��PD2ΪLED,PC13ΪTFT���⡣
   UP----PB11             DOWN------PB10
   LEFT----PA14           RIGHT-----PA13
   SEL-----PA15           USER------PB3
   
5������ͨѶ�ڶ��壺PA11��PA12������ΪUSB/CAN���ߣ�PD2ΪLEDָʾ��PB4,PB5Ϊ
   DA0,DA1��PB6,PB7Ϊ485�ڣ�PB8,PB9ΪTIM4��3��4���벶׽ͨ����

5����Ҫ�����ģ�鹦�ܣ�
   MAX3232��RX1��TX1�ӵ���PA9��PA10��
   ����LED-D2��D3ΪPA0��PA1��PA2Ϊ��λ��������

6��������߿��Ե�������TFT��������ΪSPI����4�߷�ʽ������
   ȡ��LCD������74HC595�������������ʾ�����������и���
   ���ܵĿ��ߣ�����A/D����ͨѶ�ڵ����š�

=================================================================
DISPLAY_LCDʹ�õĺ���Ϊ:
 LCD_DisplayStringLine(Line0, ptr);
 LCD_ClearLine(Line2);
 LCD_SetTextColor(Blue);
 LCD_DisplayChar(Line4, (u16)(320 -(16*(18-i))),'-')
 LCD_DrawRect(161,97,1,2);
 ���ĺ���: LCD_DrawChar(Line, Column, &ASCII_Table[Ascii * 24]);
========================================================================================
// When using Id = 0, NOMINAL_CURRENT is utilized to saturate the output of the 
// PID for speed regulation (i.e. reference torque). 
// Whit MB459 board, the value must be calculated accordingly with formula:
// NOMINAL_CURRENT = (Nominal phase current (A, 0-to-peak)*32767* Rshunt) /0.64
// ʹ��MB459ʱ��  ÿA������Ӧ��A/D����ֵ��S16��=  Rshunt * Aop(�������� = 2.57��* 65536 / 3.3(V)
											 // =  Rshunt * 32767 / 0.64	  
											 //���� 0.64 = 1.65 / 2.57
// ͬ�����ʹ��ICS����ô��������ΪAV =62.5��mv/A), 	��ô ÿA������Ӧ��A/D����ֵ��S16��= 
//						Av * 65536 / 3.3 = Av * 32767 / 1.65 = 1241 (s16���ͣ�
//						��õ���ICS���Բ�����������Ϊ = 32767 / 1241 = 26.4��A)
// ���ʹ���������͵ĵ�������������ô��������Ľ���Ϊ��ÿ�仯1����ĵ���������仯�ĵ�ѹֵ��
============================================================================================


