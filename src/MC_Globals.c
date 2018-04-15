/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : MC_Globals.c
* Author             : IMS Systems Lab
* Date First Issued  : 21/11/07
* Description        : This file contains the declarations of the global 
*                      variables utilized by the motor control library
********************************************************************************
* History:
* 21/11/07 v1.0
* 29/05/08 v2.0
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOURCE CODE IS PROTECTED BY A LICENSE.
* FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
* IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_MCconf.h"
#include "MC_const.h"
#include "MC_type.h"
#include "MC_Globals.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Electrical, magnetic and mechanical variables*/

Curr_Components Stat_Curr_a_b;              /*Stator currents Ia,Ib*/ 

Curr_Components Stat_Curr_alfa_beta;        /*Ialpha & Ibeta, Clarke's  
                                            transformations of Ia & Ib */

Curr_Components Stat_Curr_q_d;              /*Iq & Id, Parke's transformations of 
                                            Ialpha & Ibeta, */

Volt_Components Stat_Volt_a_b;              /*Stator voltages Va, Vb*/ 

Volt_Components Stat_Volt_q_d;              /*Vq & Vd, voltages on a reference
                                            frame synchronous with the rotor flux*/

Volt_Components Stat_Volt_alfa_beta;        /*Valpha & Vbeta, RevPark transformations
                                             of Vq & Vd*/

/*Variable of convenience*/

#ifdef FLUX_TORQUE_PIDs_TUNING
volatile u32 wGlobal_Flags = FIRST_START;	 //LCD����ʼ�˵�
#else
volatile u32 wGlobal_Flags = FIRST_START | SPEED_CONTROL;
#endif

volatile SystStatus_t State;

PID_Struct_t PID_Flux_InitStructure;
volatile s16 hFlux_Reference;

PID_Struct_t PID_Torque_InitStructure;
volatile s16 hTorque_Reference;

PID_Struct_t   PID_Speed_InitStructure;
volatile s16 hSpeed_Reference;

//�����ȫ�ֱ���ϵͳ����ʱ��ʵʱADת������
volatile u16 hPOT1_Volt;  //��λ��
volatile u16 hAin0_Volt;  //ģ����1
volatile u16 hAin1_Volt;  //ģ����2

volatile s16 hBreak_Curr;  //ɲ������ƽ��ֵ
volatile s16 hBus_Curr;	   //ĸ�ߵ���ƽ��ֵ

#define BufferLenght 36

volatile u32  ADC_DualConvertedValueTab[BufferLenght];
volatile u16  ADC1_RegularConvertedValueTab[BufferLenght];
volatile u16  ADC2_RegularConvertedValueTab[BufferLenght];

//���������ݲɼ�����,������������Ϊ10S�Ĳɼ�
//�ɼ�������void FOC_TorqueCtrl(void)�����ﴦ��
s16 Curr_Iq_Data[2500];
s16 Curr_Id_Data[2500];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
