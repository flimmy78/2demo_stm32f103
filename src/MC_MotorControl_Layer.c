/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : MC_MotorControl_Layer.c
* Author             : IMS Systems Lab  
* Date First Issued  : 21/11/07
* Description        : This file contains the function implementing the motor 
*                      control layer 
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
#include "stm32f10x_MClib.h"
// #include "stm32f10x_type.h"
#include "MC_Globals.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BRK_GPIO GPIOE
#define BRK_PIN GPIO_Pin_15

//����PE14�Ƿ������������BELL
//�޸������ҲҪ�޸�MCL_Fan_Init������ʹ��ʱ�ӡ�
#define FAN_GPIO_PORT GPIOA
#define FAN_GPIO_PIN  GPIO_Pin_8

//����PE14�Ƿ������������BELL
//�޸������ҲҪ�޸�MCL_Bell_Init������ʹ��ʱ�ӡ�
#define BELL_GPIO_PORT GPIOE
#define BELL_GPIO_PIN  GPIO_Pin_14

//FAULT_RESET�������Ŷ���
#define FAULT_RESET_GPIO_PORT  GPIOA
#define FAULT_RESET_GPIO_PIN   GPIO_Pin_3


#define FAULT_STATE_MIN_PERMANENCY 600 //0.5msec unit  ��300MS�ĵȴ�ʱ��󣬿������FAULT״̬��

#define BUS_AV_ARRAY_SIZE  (u8)32  //number of averaged acquisitions
#define T_AV_ARRAY_SIZE  (u16)2048  //number of averaged acquisitions

#define BUSV_CONVERSION (u16) (3.32/(BUS_ADC_CONV_RATIO)) 
#define TEMP_CONVERSION (u8)  195

#define VOLT_ARRAY_INIT (u16)(UNDERVOLTAGE_THRESHOLD+ OVERVOLTAGE_THRESHOLD)/2
#define TEMP_ARRAY_INIT (u16)0

#define BRAKE_GPIO_PORT       GPIOD
#define BRAKE_GPIO_PIN        GPIO_Pin_15  //ɲ���������

#define NTC_THRESHOLD (u16) ((32768*(NTC_THRESHOLD_C - 14))/TEMP_CONVERSION)
#define NTC_HYSTERIS  (u16) ((32768*(NTC_THRESHOLD_C - NTC_HYSTERIS_C - 14))\
                                                               /TEMP_CONVERSION)

/* Private macro -------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void MCL_Reset_PID_IntegralTerms(void);
/* Private variables ---------------------------------------------------------*/

static s16 h_BusV_Average;
static u32 w_Temp_Average;

u16 h_ADCBusvolt;
u16 h_ADCTemp;
  
/*******************************************************************************
* Function Name  : MCL_Init
* Description    : This function implements the motor control initialization to 
*                  be performed at each motor start-up 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Init(void)
{
// reset PID's integral values
    MCL_Reset_PID_IntegralTerms();
    FOC_Init();
	    
#ifdef ENCODER
    ENC_Clear_Speed_Buffer();
   #ifdef OBSERVER_GAIN_TUNING
      STO_Init();
   #endif
#elif defined HALL_SENSORS
    HALL_InitHallMeasure();
    HALL_Init_Electrical_Angle();
   #ifdef OBSERVER_GAIN_TUNING
      STO_Init();
   #endif
#elif defined NO_SPEED_SENSORS
    STO_Init();
   #ifdef VIEW_ENCODER_FEEDBACK
      ENC_Clear_Speed_Buffer();
   #elif defined VIEW_HALL_FEEDBACK
      HALL_InitHallMeasure();
      HALL_Init_Electrical_Angle();
   #endif
#endif    
        
#ifdef THREE_SHUNT                    
    SVPWM_3ShuntCurrentReadingCalibration();
#elif defined ICS_SENSORS
    SVPWM_IcsCurrentReadingCalibration();
#elif defined SINGLE_SHUNT
    SVPWM_1ShuntCurrentReadingCalibration(); //�趨�����ƫ����
#endif  
      
    Stat_Volt_alfa_beta.qV_Component1 = 0;
    Stat_Volt_alfa_beta.qV_Component2 = 0;             
    CALC_SVPWM(Stat_Volt_alfa_beta);  //��IR2103S�Ծٵ��ݳ�硣
    hTorque_Reference = PID_TORQUE_REFERENCE;   
 
    //It generates for 2 msec a 50% duty cycle on the three phases to load Boot 
    //capacitance of high side drivers
    TB_Set_StartUp_Timeout(4); 
    
    /* Main PWM Output Enable */
    TIM_CtrlPWMOutputs(TIM1,ENABLE);
  
    while(!TB_StartUp_Timeout_IsElapsed())
    {
    }  
#ifdef THREE_SHUNT    
    // Enable the Adv Current Reading during Run state
    SVPWM_3ShuntAdvCurrentReading(ENABLE);
#endif  
#ifdef SINGLE_SHUNT    
    // Enable the Adv Current Reading during Run state
    SVPWM_1ShuntAdvCurrentReading(ENABLE);	  //�򿪵������
#endif
}


/*******************************************************************************
* Function Name  : MCL_Init_Arrays
* Description    : This function initializes array to avoid erroneous Fault 
*                  detection after a reswt
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Init_Arrays(void)
{   
    w_Temp_Average = TEMP_ARRAY_INIT;
    h_BusV_Average = VOLT_ARRAY_INIT;   
}


/*******************************************************************************
* Function Name  : MCL_ChkPowerStage
* Description    : This function check for power stage working conditions
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_ChkPowerStage(void) 
{
    //  check over temperature of power stage
    if (MCL_Chk_OverTemp() == TRUE) 
    {
      MCL_SetFault(OVERHEAT);
	  MCL_Set_Fan_On();
	  MCL_Set_Bell_On();
    }   
    //  check bus under voltage 
    if (MCL_Chk_BusVolt() == UNDER_VOLT) 
    {
      MCL_SetFault(UNDER_VOLTAGE);
    }
    // bus over voltage is detected by analog watchdog
}

/*******************************************************************************
* Function Name  : MCL_SetFault() 
* Description    : This function manage faults occurences
* Input          : Fault type
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_SetFault(u16 hFault_type)
{
  TB_Set_Delay_500us(FAULT_STATE_MIN_PERMANENCY); 
  /* Main PWM Output Enable */
  TIM_CtrlPWMOutputs(TIM1, DISABLE);
  wGlobal_Flags |= hFault_type;
  State = FAULT;
  bMenu_index = FAULT_MENU;  
  // It is required to disable AdvCurrentReading in IDLE to sample DC 
  // Bus Value
#ifdef THREE_SHUNT
  SVPWM_3ShuntAdvCurrentReading(DISABLE);
#endif
#ifdef SINGLE_SHUNT
  SVPWM_1ShuntAdvCurrentReading(DISABLE);
#endif
}

/*******************************************************************************
* Function Name  : MCL_ClearFault() 
* Description    : This function check if the fault source is over. In case it 
*                  is, it clears the related flag and return true. Otherwise it 
*                  returns FALSE
* Input          : Fault type
* Output         : None
* Return         : None
*******************************************************************************/
bool MCL_ClearFault(void)
{     
  if (TB_Delay_IsElapsed())
  {   
	 MCL_Fault_Reset_Off();       //ʹ��IGBT

    if ((wGlobal_Flags & OVERHEAT) == OVERHEAT)   
    {               
      if(MCL_Chk_OverTemp()== FALSE)
      {
        wGlobal_Flags &= ~OVERHEAT;
		MCL_Set_Fan_Off();
		MCL_Set_Bell_Off();
      }     
    }
    
    if ((wGlobal_Flags & OVER_VOLTAGE) == OVER_VOLTAGE)   
    {            
      if(MCL_Chk_BusVolt()== NO_FAULT)
      {
        wGlobal_Flags &= ~OVER_VOLTAGE;
      } 
    }
    
    if ((wGlobal_Flags & UNDER_VOLTAGE) == UNDER_VOLTAGE)   
    {            
      if(MCL_Chk_BusVolt()== NO_FAULT)
      {
        wGlobal_Flags &= ~UNDER_VOLTAGE;
      } 
    }
    
    if ((wGlobal_Flags & OVER_CURRENT) == OVER_CURRENT)
    {
      // high level detected on emergency pin?              
      //It checks for a low level on MCES before re-enable PWM 
      //peripheral
      if (GPIO_ReadInputDataBit(BRK_GPIO, BRK_PIN))
      {            
        wGlobal_Flags &= ~OVER_CURRENT;
      }
    }
  
    if ((wGlobal_Flags & START_UP_FAILURE) == START_UP_FAILURE )
    {
        wGlobal_Flags &= ~START_UP_FAILURE;
    } 
    
    if ((wGlobal_Flags & SPEED_FEEDBACK) == SPEED_FEEDBACK )
    {
        wGlobal_Flags &= ~SPEED_FEEDBACK;
    } 
  }
  
  if (KEYS_ExportbKey() == SEL)
  {
    if ( (wGlobal_Flags & (OVER_CURRENT | OVERHEAT | UNDER_VOLTAGE | 
                       SPEED_FEEDBACK | START_UP_FAILURE | OVER_VOLTAGE)) == 0 )       
    { 
      return(TRUE);
    } 
    else
    {
      return(FALSE);
    }
  }
  else 
  {
    return(FALSE);
  }
}

/*******************************************************************************
* Function Name  : MCL_Chk_OverTemp
* Description    : Return TRUE if the voltage on the thermal resistor connected 
*                  to channel AIN3 has reached the threshold level or if the           
*                  voltage has not yet reached back the threshold level minus  
*                  the hysteresis value after an overheat detection.
* Input          : None
* Output         : Boolean
* Return         : None
*******************************************************************************/
bool MCL_Chk_OverTemp(void)
{
  bool bStatus;
   
  w_Temp_Average = ((T_AV_ARRAY_SIZE-1)*w_Temp_Average + h_ADCTemp)
                                                              /T_AV_ARRAY_SIZE;
  
  if (w_Temp_Average >= NTC_THRESHOLD)    
  {
    bStatus = TRUE;
  }
  else if (w_Temp_Average >= (NTC_HYSTERIS) ) 
    {
    if ((wGlobal_Flags & OVERHEAT) == OVERHEAT)
      {
        bStatus = TRUE;       
      }
    else
      {
        bStatus = FALSE;
      }
    }
  else 
    {
      bStatus = FALSE;
    }

  return(bStatus);
}

/*******************************************************************************
* Function Name  : MCL_Calc_BusVolt
* Description    : It measures the Bus Voltage
* Input          : None
* Output         : Bus voltage
* Return         : None
*******************************************************************************/
void MCL_Calc_BusVolt(void)
{
 h_BusV_Average = ((BUS_AV_ARRAY_SIZE-1)*h_BusV_Average + h_ADCBusvolt)
                                                             /BUS_AV_ARRAY_SIZE;
}

/*******************************************************************************
* Function Name  : MCL_Chk_BusVolt 
* Description    : Check for Bus Over Voltage
* Input          : None
* Output         : Boolean
* Return         : None
*******************************************************************************/
BusV_t MCL_Chk_BusVolt(void)
{
  BusV_t baux;
  if (h_BusV_Average > OVERVOLTAGE_THRESHOLD)    
  {
    baux = OVER_VOLT;
  }
  else if (h_BusV_Average < UNDERVOLTAGE_THRESHOLD)    
  {
    baux = UNDER_VOLT;
  }
  else 
  {
    baux = NO_FAULT; 
  }
  return ((BusV_t)baux);
}

/*******************************************************************************
* Function Name  : MCL_Get_BusVolt
* Description    : Get bus voltage in s16
* Input          : None
* Output         : None
* Return         : Bus voltage in s16 unit
*******************************************************************************/
s16 MCL_Get_BusVolt(void)
{
  return (h_BusV_Average);
}

/*******************************************************************************
* Function Name  : MCL_Compute_BusVolt
* Description    : Compute bus voltage in volt
* Input          : None
* Output         : Bus voltage in Volt unit
* Return         : None
*******************************************************************************/
u16 MCL_Compute_BusVolt(void)
{
  return ((u16)((h_BusV_Average * BUSV_CONVERSION)/32768));
}

/*******************************************************************************
* Function Name  : MCL_Compute_Temp
* Description    : Compute temperature in Celsius degrees
* Input          : None
* Output         : temperature in Celsius degrees
* Return         : None
*******************************************************************************/
u8 MCL_Compute_Temp(void)
{
  return ((u8)((w_Temp_Average * TEMP_CONVERSION)/32768+14));
}      

/*******************************************************************************
* Function Name  : MCL_Reset_PID_IntegralTerms
* Description    : Resets flux, torque and speed PID Integral Terms
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Reset_PID_IntegralTerms(void)
{
  PID_Speed_InitStructure.wIntegral=0;
  PID_Torque_InitStructure.wIntegral=0;
  PID_Flux_InitStructure.wIntegral = 0;
}

#ifdef BRAKE_RESISTOR
/*******************************************************************************
* Function Name  : MCL_Brake_Init
* Description    : Initialize the GPIO driving the switch for resitive brake 
*                  implementation  
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Brake_Init(void)
{  
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOB clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
//  GPIO_DeInit(BRAKE_GPIO_PORT);
  GPIO_StructInit(&GPIO_InitStructure);
                  
  /* Configure PB.8 as Output push-pull for break feature */
  GPIO_InitStructure.GPIO_Pin = BRAKE_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(BRAKE_GPIO_PORT, &GPIO_InitStructure);     
}

/*******************************************************************************
* Function Name  : MCL_Set_Brake_On
* Description    : Switch on brake (set the related GPIO pin)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Set_Brake_On(void)
{  
 GPIO_SetBits(BRAKE_GPIO_PORT, BRAKE_GPIO_PIN);
}

/*******************************************************************************
* Function Name  : MCL_Set_Brake_Off
* Description    : Switch off brake (reset the related GPIO pin)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Set_Brake_Off(void)
{  
 GPIO_ResetBits(BRAKE_GPIO_PORT, BRAKE_GPIO_PIN);
}

#endif

/*******************************************************************************
* Function Name  : MCL_Fan_Init
* Description    : ��ʼ���������� (reset the related GPIO pin)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Fan_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOB clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  GPIO_StructInit(&GPIO_InitStructure);
                  
  /* Configure PB.8 as Output push-pull for break feature */
  GPIO_InitStructure.GPIO_Pin = FAN_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(FAN_GPIO_PORT, &GPIO_InitStructure);   
  GPIO_SetBits(FAN_GPIO_PORT, FAN_GPIO_PIN);
 
}
/*******************************************************************************
* Function Name  : MCL_Fan_On
* Description    : �򿪷������� (reset the related GPIO pin)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Set_Fan_On(void)
{
 GPIO_ResetBits(FAN_GPIO_PORT, FAN_GPIO_PIN);
}

/*******************************************************************************
* Function Name  : MCL_Fan_Off
* Description    : �رշ������� (reset the related GPIO pin)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Set_Fan_Off(void)
{  
 GPIO_SetBits(FAN_GPIO_PORT, FAN_GPIO_PIN);
}


/*******************************************************************************
* Function Name  : MCL_Bell_Init
* Description    : ��ʼ��BELL���� (reset the related GPIO pin)	PC9
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Bell_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOB clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
  GPIO_StructInit(&GPIO_InitStructure);
                  
  /* Configure PB.8 as Output push-pull for break feature */
  GPIO_ResetBits(BELL_GPIO_PORT, BELL_GPIO_PIN);	//�ر�BELL
  GPIO_InitStructure.GPIO_Pin = BELL_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(BELL_GPIO_PORT, &GPIO_InitStructure);   

}
/*******************************************************************************
* Function Name  : MCL_BELL_On
* Description    : ��BELL���� (reset the related GPIO pin)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Set_Bell_On(void)
{
 GPIO_SetBits(BELL_GPIO_PORT, BELL_GPIO_PIN);
}

/*******************************************************************************
* Function Name  : MCL_BELL_Off
* Description    : �ر�BELL���� (reset the related GPIO pin)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Set_Bell_Off(void)
{  
 GPIO_ResetBits(BELL_GPIO_PORT, BELL_GPIO_PIN);
}

/*******************************************************************************
* Function Name  : MCL_Fault_Reset_Init
* Description    : ��ʼ��FAULT_RESET���� (reset the related GPIO pin)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Fault_Reset_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOB clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  GPIO_StructInit(&GPIO_InitStructure);
                  
  /* Configure PB.8 as Output push-pull for break feature */
  GPIO_ResetBits(FAULT_RESET_GPIO_PORT, FAULT_RESET_GPIO_PIN);	//
  GPIO_SetBits(FAULT_RESET_GPIO_PORT, FAULT_RESET_GPIO_PIN);
  GPIO_InitStructure.GPIO_Pin = FAULT_RESET_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(FAULT_RESET_GPIO_PORT, &GPIO_InitStructure); 
  GPIO_ResetBits(FAULT_RESET_GPIO_PORT, FAULT_RESET_GPIO_PIN);	//
  TB_Wait(200); //��ʱ100ms
  GPIO_SetBits(FAULT_RESET_GPIO_PORT, FAULT_RESET_GPIO_PIN);
 }

/*******************************************************************************
* Function Name  : MCL_Fault_Reset_Off
* Description    : ��ʼ��FAULT_RESET���� (reset the related GPIO pin)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Fault_Reset_Off(void)
{  
 GPIO_SetBits(FAULT_RESET_GPIO_PORT, FAULT_RESET_GPIO_PIN);
}

/*******************************************************************************
* Function Name  : MCL_Fault_Reset_On
* Description    : ��ʼ��FAULT_RESET���� (reset the related GPIO pin)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MCL_Fault_Reset_On(void)
{  
  GPIO_ResetBits(FAULT_RESET_GPIO_PORT, FAULT_RESET_GPIO_PIN);	//
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
