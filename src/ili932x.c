/******************************************************************************
* �ļ����ƣ�ili932x.c
* ժ Ҫ��֧��ILI9320��ILI9325����IC���Ƶ�QVGA��ʾ����ʹ��16λ���д���
  ��ͷ�ļ������� ��Ļʹ�÷��������IC����
  ע�⣺16λ������ɫ�ʷֲ�>>  BGR(565)

* ��ǰ�汾��V1.3
* �޸�˵�����汾�޶�˵����
  1.�޸ķ�תģʽ�µ�ASCII�ַ�дBug
  2.���ӿ����ڷ�תģʽ�µ��Զ���д
  3.�Ż�ˢͼƬ ʹ����ˮ�߷�����Ч��
*��Ҫ˵����
��.h�ļ��У�#define Immediatelyʱ��������ʾ��ǰ����
�����#define Delay����ֻ����ִ����LCD_WR_REG(0x0007,0x0173);
֮��Ż���ʾ��ִ��һ��LCD_WR_REG(0x0007,0x0173)������д����
�ݶ�������ʾ��
#define Delayһ�����ڿ����������ʾ����ֹ��ʾ��ȫ��ͼ���ˢ��
����
******************************************************************************/
#include "stm32f10x.h"  
#include "ili932x.h"
#include "ili9320_font.h"
#include "spi_flash.h"


#define LCD_DATA_PORT GPIOE

static  vu16 TextColor = 0x0000, BackColor = 0xFFFF;

#define START_BYTE  0x70
#define SET_INDEX   0x00
#define READ_STATUS 0x01
#define WRITE_REG   0x02
#define READ_REG    0x03

#ifdef ILI9325
	u16 DeviceCode = 0x9325;    //2.8TFT
#elif defined ILI9320
	u16 DeviceCode = 0x9320;    //3.2TFT
#elif defined SSD2119      		//3.5TFT
	u16 DeviceCode = 0x9919;
#elif defined SSD1298      		//3.5TFT
	u16 DeviceCode = 0x8999;
#elif defined ILI9328	   		//2.8TFT
	u16 DeviceCode = 0x9328;
#elif defined ILI9331	   		//2.8TFT
	u16 DeviceCode = 0x9331;
#elif defined ST7781
	u16 DeviceCode = 0x7783;
#elif defined LGDP4531
	u16 DeviceCode = 0x4531;
#endif

/*******************************************************************************
* Function Name  : LCD_Init
* Description    : Initializes the LCD.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_Init(void)
{
  /* Setups the LCD */
  Lcd_Configuration();
  Lcd_Initialize();
 }

  /****************************************************************
��������Lcd���ú���
���ܣ��������к�Lcd��ص�GPIO��ʱ��
���ŷ���Ϊ��
PB8--PB15����16Bit�������ߵ�8λ
PC0--PC7 ����16Bit�������߸�8λ
PC8 ����Lcd_cs
PC9 ����Lcd_rs*
PC10����Lcd_wr
PC11����Lcd_rd*
PC12����Lcd_rst
PC13����Lcd_blaklight ���⿿��ЧӦ����������ģ��
*****************************************************************/
void Lcd_Configuration(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	/*������Ӧʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD, ENABLE);  
	/*����Lcd��������Ϊ�������*/
#ifdef LCD_16B_IO 
	/*16λ���ݵ�8λ*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
#endif
	/*16λ���ݸ�8λ*/
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	/*���ƽ�*/
        Set_Cs;
        Set_Rs;
        Set_nRd;
        Set_nWr;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*������ƺ�LCD_RS����*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
#ifdef LCD_8B_IO 
	/*74HC573��ALE������ƶ�*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
        Clr_LE;	   //��573��ALE��
#endif
					   
}

/****************************************************************************
* ��    �ƣ�u16 CheckController(void)
* ��    �ܣ����ؿ���������
* ��ڲ�������
* ���ڲ������������ͺ�
* ˵    �������ú󷵻ؼ����ͺŵĿ������ͺ�
* ���÷�����code=CheckController();
****************************************************************************/
u16 CheckController(void)
{
  	u16 tmp=0,tmp1=0,tmp2=0; 
	GPIO_InitTypeDef GPIO_InitStructure;

 	DataToWrite(0xffff);//������ȫ��
//	Set_Rst;
	Set_nWr;
	Set_Cs;
	Set_Rs;
	Set_nRd;
//	Set_Rst;
	Delay_nms(1);
//	Clr_Rst;
	Delay_nms(1);
//	Set_Rst;
	Delay_nms(1);
	LCD_WR_REG(0x0000,0x0001);  //start oscillation
	Delay_nms(1);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*16λ���ݸ�8λ*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  	GPIO_ResetBits(GPIOC,GPIO_Pin_8);
  
  	GPIO_SetBits(GPIOC,GPIO_Pin_9);
  
  	GPIO_ResetBits(GPIOC,GPIO_Pin_11);

  	tmp1 = GPIO_ReadInputData(GPIOB);
	tmp2 = GPIO_ReadInputData(GPIOC);

	tmp = (tmp1>>8) | (tmp2<<8);
  
  	GPIO_SetBits(GPIOC,GPIO_Pin_11);
  
  	GPIO_SetBits(GPIOC,GPIO_Pin_8);

	/*16λ���ݵ�8λ*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	/*16λ���ݸ�8λ*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  	return tmp;
}


/**********************************************
��������Lcd��ʼ������
���ܣ���ʼ��Lcd
��ڲ�������
����ֵ����
***********************************************/
void Lcd_Initialize(void)
{
  	u16 i;
//	Lcd_Light_ON;
	DataToWrite(0xffff);//������ȫ��
//	Set_Rst;
	Set_nWr;
	Set_Cs;
	Set_Rs;
	Set_nRd;
//	Set_Rst;
	Delay_nms(1);
//	Clr_Rst;
	Delay_nms(1);
//	Set_Rst;
	Delay_nms(1); 

//    DeviceCode = ili9320_ReadRegister(0x0000);
//    DeviceCode = 0x9325;
	if(DeviceCode==0x9325||DeviceCode==0x9328)	  //2.8 TFT
	{
  	ili9320_WriteRegister(0x00e7,0x0010);      
        ili9320_WriteRegister(0x0000,0x0001);  			//start internal osc
        ili9320_WriteRegister(0x0001,0x0100);     
        ili9320_WriteRegister(0x0002,0x0700); 				//power on sequence                     
        //ILI9320��ILI9325����������
        //ili9320_WriteRegister(0x0003,(1<<12)|(1<<5)|(1<<4) ); 	//ILI9325������ 
        ili9320_WriteRegister(0x0003,(1<<12)|(1<<3)|(1<<4) ); 	//ILI9325���� 
        ili9320_WriteRegister(0x0004,0x0000);                                   
        ili9320_WriteRegister(0x0008,0x0207);	           
        ili9320_WriteRegister(0x0009,0x0000);         
        ili9320_WriteRegister(0x000a,0x0000); 				//display setting         
        ili9320_WriteRegister(0x000c,0x0001);				//display setting          
        ili9320_WriteRegister(0x000d,0x0000); 				//0f3c          
        ili9320_WriteRegister(0x000f,0x0000);
//Power On sequence //
        ili9320_WriteRegister(0x0010,0x0000);   
        ili9320_WriteRegister(0x0011,0x0007);
        ili9320_WriteRegister(0x0012,0x0000);                                                                 
        ili9320_WriteRegister(0x0013,0x0000);                 
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
        ili9320_WriteRegister(0x0010,0x1590);   
        ili9320_WriteRegister(0x0011,0x0227);
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
        ili9320_WriteRegister(0x0012,0x009c);                  
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
        ili9320_WriteRegister(0x0013,0x1900);   
        ili9320_WriteRegister(0x0029,0x0023);
        ili9320_WriteRegister(0x002b,0x000e);
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
//        ili9320_WriteRegister(0x0020,0x0000);                                                            
//        ili9320_WriteRegister(0x0021,0x0000);           
///////////////////////////////////////////////////////      
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
        ili9320_WriteRegister(0x0030,0x0007); 
        ili9320_WriteRegister(0x0031,0x0707);   
        ili9320_WriteRegister(0x0032,0x0006);
        ili9320_WriteRegister(0x0035,0x0704);
        ili9320_WriteRegister(0x0036,0x1f04); 
        ili9320_WriteRegister(0x0037,0x0004);
        ili9320_WriteRegister(0x0038,0x0000);        
        ili9320_WriteRegister(0x0039,0x0706);     
        ili9320_WriteRegister(0x003c,0x0701);
        ili9320_WriteRegister(0x003d,0x000f);
        for(i=50000;i>0;i--);
		for(i=50000;i>0;i--);
        ili9320_WriteRegister(0x0050,0x0000);        
        ili9320_WriteRegister(0x0051,0x00ef);   
        ili9320_WriteRegister(0x0052,0x0000);     
        ili9320_WriteRegister(0x0053,0x013f);
        ili9320_WriteRegister(0x0060,0xa700);        
        ili9320_WriteRegister(0x0061,0x0001); 
        ili9320_WriteRegister(0x006a,0x0000);
        ili9320_WriteRegister(0x0080,0x0000);
        ili9320_WriteRegister(0x0081,0x0000);
        ili9320_WriteRegister(0x0082,0x0000);
        ili9320_WriteRegister(0x0083,0x0000);
        ili9320_WriteRegister(0x0084,0x0000);
        ili9320_WriteRegister(0x0085,0x0000);
      
        ili9320_WriteRegister(0x0090,0x0010);     
        ili9320_WriteRegister(0x0092,0x0000);  
        ili9320_WriteRegister(0x0093,0x0003);
        ili9320_WriteRegister(0x0095,0x0110);
        ili9320_WriteRegister(0x0097,0x0000);        
        ili9320_WriteRegister(0x0098,0x0000);  
         //display on sequence     
        ili9320_WriteRegister(0x0007,0x0133);
    
        ili9320_WriteRegister(0x0020,0x0000);                                                            
        ili9320_WriteRegister(0x0021,0x0000);
	}
	else if(DeviceCode==0x9320 || DeviceCode==0x9300)	  //3.2 TFT
	{
		ili9320_WriteRegister(0x00,0x0000);
		ili9320_WriteRegister(0x01,0x0100);	//Driver Output Contral.
		ili9320_WriteRegister(0x02,0x0700);	//LCD Driver Waveform Contral.
//		ili9320_WriteRegister(0x03,0x1030);	//Entry Mode Set.
		ili9320_WriteRegister(0x03,0x1018);	//Entry Mode Set.
	
		ili9320_WriteRegister(0x04,0x0000);	//Scalling Contral.
		ili9320_WriteRegister(0x08,0x0202);	//Display Contral 2.(0x0207)
		ili9320_WriteRegister(0x09,0x0000);	//Display Contral 3.(0x0000)
		ili9320_WriteRegister(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
		ili9320_WriteRegister(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
		ili9320_WriteRegister(0x0d,0x0000);	//Frame Maker Position.
		ili9320_WriteRegister(0x0f,0x0000);	//Extern Display Interface Contral 2.
	
        	Delay_nms(1);
                
		ili9320_WriteRegister(0x07,0x0101);	//Display Contral.
		Delay_nms(1);

		ili9320_WriteRegister(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1.(0x16b0)
		ili9320_WriteRegister(0x11,0x0007);								//Power Control 2.(0x0001)
		ili9320_WriteRegister(0x12,(1<<8)|(1<<4)|(0<<0));					//Power Control 3.(0x0138)
		ili9320_WriteRegister(0x13,0x0b00);								//Power Control 4.
		ili9320_WriteRegister(0x29,0x0000);								//Power Control 7.
	
		ili9320_WriteRegister(0x2b,(1<<14)|(1<<4));
		
		ili9320_WriteRegister(0x50,0);		//Set X Start.
		ili9320_WriteRegister(0x51,239);	//Set X End.
		ili9320_WriteRegister(0x52,0);		//Set Y Start.
		ili9320_WriteRegister(0x53,319);	//Set Y End.
	
		ili9320_WriteRegister(0x60,0x2700);	//Driver Output Control.
		ili9320_WriteRegister(0x61,0x0001);	//Driver Output Control.
		ili9320_WriteRegister(0x6a,0x0000);	//Vertical Srcoll Control.
	
		ili9320_WriteRegister(0x80,0x0000);	//Display Position? Partial Display 1.
		ili9320_WriteRegister(0x81,0x0000);	//RAM Address Start? Partial Display 1.
		ili9320_WriteRegister(0x82,0x0000);	//RAM Address End-Partial Display 1.
		ili9320_WriteRegister(0x83,0x0000);	//Displsy Position? Partial Display 2.
		ili9320_WriteRegister(0x84,0x0000);	//RAM Address Start? Partial Display 2.
		ili9320_WriteRegister(0x85,0x0000);	//RAM Address End? Partial Display 2.
	
		ili9320_WriteRegister(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
		ili9320_WriteRegister(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
		ili9320_WriteRegister(0x93,0x0001);	//Panel Interface Contral 3.
		ili9320_WriteRegister(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
		ili9320_WriteRegister(0x97,(0<<8));	//
		ili9320_WriteRegister(0x98,0x0000);	//Frame Cycle Contral.

	
		ili9320_WriteRegister(0x07,0x0173);	//(0x0173)
        	Delay_nms(1);
                
	}
	else if(DeviceCode==0x9919)
	{
		//*********POWER ON &RESET DISPLAY OFF
		 LCD_WriteReg(0x28,0x0006);
		
		 LCD_WriteReg(0x00,0x0001);
		
		 LCD_WriteReg(0x10,0x0000);
		
		 LCD_WriteReg(0x01,0x72ef);

		 LCD_WriteReg(0x02,0x0600);

		 LCD_WriteReg(0x03,0x6a38);
		
		 LCD_WriteReg(0x11,0x6058);//70
		
		 
		     //  RAM WRITE DATA MASK
		 LCD_WriteReg(0x0f,0x0000); 
		    //  RAM WRITE DATA MASK
		 LCD_WriteReg(0x0b,0x5308); 
		
		 LCD_WriteReg(0x0c,0x0003);
		
		 LCD_WriteReg(0x0d,0x000a);
		
		 LCD_WriteReg(0x0e,0x2e00);  //0030
		
		 LCD_WriteReg(0x1e,0x00be);
		
		 LCD_WriteReg(0x25,0x8000);
		
		 LCD_WriteReg(0x26,0x7800);
		
		 LCD_WriteReg(0x27,0x0078);
		
		 LCD_WriteReg(0x4e,0x0000);
		
		 LCD_WriteReg(0x4f,0x0000);
		
		 LCD_WriteReg(0x12,0x08d9);
		
		 // -----------------Adjust the Gamma Curve----//
		 LCD_WriteReg(0x30,0x0000);	 //0007
		
		 LCD_WriteReg(0x31,0x0104);	   //0203
		
		 LCD_WriteReg(0x32,0x0100);		//0001

		 LCD_WriteReg(0x33,0x0305);	  //0007

		 LCD_WriteReg(0x34,0x0505);	  //0007
		
		 LCD_WriteReg(0x35,0x0305);		 //0407
		
		 LCD_WriteReg(0x36,0x0707);		 //0407
		
		 LCD_WriteReg(0x37,0x0300);		  //0607
		
		 LCD_WriteReg(0x3a,0x1200);		 //0106
		
		 LCD_WriteReg(0x3b,0x0800);		 

		 LCD_WriteReg(0x07,0x0033);
	}
	else if(DeviceCode==0x9331)		//2.8 TFT
	{
	 	/*********POWER ON &RESET DISPLAY OFF*/
		/************* Start Initial Sequence **********/
		ili9320_WriteRegister(0x00E7, 0x1014);
		ili9320_WriteRegister(0x0001, 0x0100); // set SS and SM bit   0x0100
		ili9320_WriteRegister(0x0002, 0x0200); // set 1 line inversion
		ili9320_WriteRegister(0x0003, 0x1030); // set GRAM write direction and BGR=1.     0x1030
		ili9320_WriteRegister(0x0008, 0x0202); // set the back porch and front porch
		ili9320_WriteRegister(0x0009, 0x0000); // set non-display area refresh cycle ISC[3:0]
		ili9320_WriteRegister(0x000A, 0x0000); // FMARK function
		ili9320_WriteRegister(0x000C, 0x0000); // RGB interface setting
		ili9320_WriteRegister(0x000D, 0x0000); // Frame marker Position
		ili9320_WriteRegister(0x000F, 0x0000); // RGB interface polarity
		//*************Power On sequence ****************//
		ili9320_WriteRegister(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
		ili9320_WriteRegister(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
		ili9320_WriteRegister(0x0012, 0x0000); // VREG1OUT voltage
		ili9320_WriteRegister(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
		ili9320_Delay(200); // Dis-charge capacitor power voltage
		ili9320_WriteRegister(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
		ili9320_WriteRegister(0x0011, 0x0227); // DC1[2:0], DC0[2:0], VC[2:0]
		ili9320_Delay(50); // Delay 50ms
		ili9320_WriteRegister(0x0012, 0x000C); // Internal reference voltage= Vci;
		ili9320_Delay(50); // Delay 50ms
		ili9320_WriteRegister(0x0013, 0x0800); // Set VDV[4:0] for VCOM amplitude
		ili9320_WriteRegister(0x0029, 0x0011); // Set VCM[5:0] for VCOMH
		ili9320_WriteRegister(0x002B, 0x000B); // Set Frame Rate
		ili9320_Delay(50); // Delay 50ms
		ili9320_WriteRegister(0x0020, 0x0000); // GRAM horizontal Address
		ili9320_WriteRegister(0x0021, 0x0000); // GRAM Vertical Address
		// ----------- Adjust the Gamma Curve ----------//
		ili9320_WriteRegister(0x0030, 0x0000);
		ili9320_WriteRegister(0x0031, 0x0106);
		ili9320_WriteRegister(0x0032, 0x0000);
		ili9320_WriteRegister(0x0035, 0x0204);
		ili9320_WriteRegister(0x0036, 0x160A);
		ili9320_WriteRegister(0x0037, 0x0707);
		ili9320_WriteRegister(0x0038, 0x0106);
		ili9320_WriteRegister(0x0039, 0x0707);
		ili9320_WriteRegister(0x003C, 0x0402);
		ili9320_WriteRegister(0x003D, 0x0C0F);
		//------------------ Set GRAM area ---------------//
		ili9320_WriteRegister(0x0050, 0x0000); // Horizontal GRAM Start Address
		ili9320_WriteRegister(0x0051, 0x00EF); // Horizontal GRAM End Address
		ili9320_WriteRegister(0x0052, 0x0000); // Vertical GRAM Start Address
		ili9320_WriteRegister(0x0053, 0x013F); // Vertical GRAM Start Address
		ili9320_WriteRegister(0x0060, 0x2700); // Gate Scan Line
		ili9320_WriteRegister(0x0061, 0x0001); // NDL,VLE, REV
		ili9320_WriteRegister(0x006A, 0x0000); // set scrolling line
		//-------------- Partial Display Control ---------//
		ili9320_WriteRegister(0x0080, 0x0000);
		ili9320_WriteRegister(0x0081, 0x0000);
		ili9320_WriteRegister(0x0082, 0x0000);
		ili9320_WriteRegister(0x0083, 0x0000);
		ili9320_WriteRegister(0x0084, 0x0000);
		ili9320_WriteRegister(0x0085, 0x0000);
		//-------------- Panel Control -------------------//
		ili9320_WriteRegister(0x0090, 0x0010);
		ili9320_WriteRegister(0x0092, 0x0600);
		ili9320_WriteRegister(0x0007,0x0021);		
		ili9320_Delay(50);
		ili9320_WriteRegister(0x0007,0x0061);
		ili9320_Delay(50);
		ili9320_WriteRegister(0x0007,0x0133);  // 262K color and display ON
		ili9320_Delay(50);
	}  
	else if(DeviceCode==0x7783)		   //2.8 TFT
	{
		// Start Initial Sequence
		ili9320_WriteRegister(0x00FF,0x0001);
		ili9320_WriteRegister(0x00F3,0x0008);
		ili9320_WriteRegister(0x0001,0x0100);
		ili9320_WriteRegister(0x0002,0x0700);
		ili9320_WriteRegister(0x0003,0x1030);  //0x1030
		ili9320_WriteRegister(0x0008,0x0302);
		ili9320_WriteRegister(0x0008,0x0207);
		ili9320_WriteRegister(0x0009,0x0000);
		ili9320_WriteRegister(0x000A,0x0000);
		ili9320_WriteRegister(0x0010,0x0000);  //0x0790
		ili9320_WriteRegister(0x0011,0x0005);
		ili9320_WriteRegister(0x0012,0x0000);
		ili9320_WriteRegister(0x0013,0x0000);
		ili9320_Delay(50);
		ili9320_WriteRegister(0x0010,0x12B0);
		ili9320_Delay(50);
		ili9320_WriteRegister(0x0011,0x0007);
		ili9320_Delay(50);
		ili9320_WriteRegister(0x0012,0x008B);
		ili9320_Delay(50);
		ili9320_WriteRegister(0x0013,0x1700);
		ili9320_Delay(50);
		ili9320_WriteRegister(0x0029,0x0022);
		
		//################# void Gamma_Set(void) ####################//
		ili9320_WriteRegister(0x0030,0x0000);
		ili9320_WriteRegister(0x0031,0x0707);
		ili9320_WriteRegister(0x0032,0x0505);
		ili9320_WriteRegister(0x0035,0x0107);
		ili9320_WriteRegister(0x0036,0x0008);
		ili9320_WriteRegister(0x0037,0x0000);
		ili9320_WriteRegister(0x0038,0x0202);
		ili9320_WriteRegister(0x0039,0x0106);
		ili9320_WriteRegister(0x003C,0x0202);
		ili9320_WriteRegister(0x003D,0x0408);
		ili9320_Delay(50);
		
		
		ili9320_WriteRegister(0x0050,0x0000);		
		ili9320_WriteRegister(0x0051,0x00EF);		
		ili9320_WriteRegister(0x0052,0x0000);		
		ili9320_WriteRegister(0x0053,0x013F);		
		ili9320_WriteRegister(0x0060,0xA700);		
		ili9320_WriteRegister(0x0061,0x0001);
		ili9320_WriteRegister(0x0090,0x0033);				
		ili9320_WriteRegister(0x002B,0x000B);		
		ili9320_WriteRegister(0x0007,0x0133);
		ili9320_Delay(50);
	}  

	else if(DeviceCode==0x8999)			//SSD1298 3.2 TFT
	{
		LCD_WriteReg(0x0000,0x0001);    ili9320_Delay(50);  //�򿪾���
    	LCD_WriteReg(0x0003,0xA8A4);    ili9320_Delay(50);   //0xA8A4
    	LCD_WriteReg(0x000C,0x0000);    ili9320_Delay(50);   
    	LCD_WriteReg(0x000D,0x080C);    ili9320_Delay(50);   
    	LCD_WriteReg(0x000E,0x2B00);    ili9320_Delay(50);   
    	LCD_WriteReg(0x001E,0x00B0);    ili9320_Delay(50);   
    	LCD_WriteReg(0x0001,0x3B3F);    ili9320_Delay(50);   //�����������320*240  0x6B3F
    	LCD_WriteReg(0x0002,0x0600);    ili9320_Delay(50);
    	LCD_WriteReg(0x0010,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x0011,0x6058);    ili9320_Delay(50);        //0x4030           //�������ݸ�ʽ  16λɫ 		���� 0x6058
    	LCD_WriteReg(0x0005,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x0006,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x0016,0xEF1C);    ili9320_Delay(50);
    	LCD_WriteReg(0x0017,0x0003);    ili9320_Delay(50);
    	LCD_WriteReg(0x0007,0x0233);    ili9320_Delay(50);        //0x0233       
    	LCD_WriteReg(0x000B,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x000F,0x0000);    ili9320_Delay(50);        //ɨ�迪ʼ��ַ
    	LCD_WriteReg(0x0041,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x0042,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x0048,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x0049,0x013F);    ili9320_Delay(50);
    	LCD_WriteReg(0x004A,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x004B,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x0044,0xEF00);    ili9320_Delay(50);
    	LCD_WriteReg(0x0045,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x0046,0x013F);    ili9320_Delay(50);
    	LCD_WriteReg(0x0030,0x0707);    ili9320_Delay(50);
    	LCD_WriteReg(0x0031,0x0204);    ili9320_Delay(50);
    	LCD_WriteReg(0x0032,0x0204);    ili9320_Delay(50);
    	LCD_WriteReg(0x0033,0x0502);    ili9320_Delay(50);
    	LCD_WriteReg(0x0034,0x0507);    ili9320_Delay(50);
    	LCD_WriteReg(0x0035,0x0204);    ili9320_Delay(50);
    	LCD_WriteReg(0x0036,0x0204);    ili9320_Delay(50);
    	LCD_WriteReg(0x0037,0x0502);    ili9320_Delay(50);
    	LCD_WriteReg(0x003A,0x0302);    ili9320_Delay(50);
    	LCD_WriteReg(0x003B,0x0302);    ili9320_Delay(50);
    	LCD_WriteReg(0x0023,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x0024,0x0000);    ili9320_Delay(50);
    	LCD_WriteReg(0x0025,0x8000);    ili9320_Delay(50);
    	LCD_WriteReg(0x004f,0);        //����ַ0
    	LCD_WriteReg(0x004e,0);        //����ַ0
	} 
	else if(DeviceCode==0x4531)		 //2.8 TFT
	{
		ili9320_WriteRegister(0x0010,0x0628);    
    	ili9320_WriteRegister(0x0012,0x0006);     
    	ili9320_WriteRegister(0x0013,0x0A32);       
    	ili9320_WriteRegister(0x0011,0x0040);       
    	ili9320_WriteRegister(0x0015,0x0050);      
    	ili9320_WriteRegister(0x0012,0x0016);    
		
		ili9320_Delay(15);   
    	ili9320_WriteRegister(0x0010,0x5660);    
		ili9320_Delay(15);
    	
		ili9320_WriteRegister(0x0013,0x2A4E);    
    	ili9320_WriteRegister(0x0001,0x0100);    
    	ili9320_WriteRegister(0x0002,0x0300);    
    	ili9320_WriteRegister(0x0003,0x1018);    //�п�����ʾ������
    	ili9320_WriteRegister(0x0008,0x0202);    
    	ili9320_WriteRegister(0x000A,0x0000); 
		
		   
    	ili9320_WriteRegister(0x0030,0x0000);    
    	ili9320_WriteRegister(0x0031,0x0402);           //0x0233       
    	ili9320_WriteRegister(0x0032,0x0106);    
    	ili9320_WriteRegister(0x0033,0x0700);          //ɨ�迪ʼ��ַ
    	ili9320_WriteRegister(0x0034,0x0104);    
    	ili9320_WriteRegister(0x0035,0x0301);    
    	ili9320_WriteRegister(0x0036,0x0707);    
    	ili9320_WriteRegister(0x0037,0x0305);    
    	ili9320_WriteRegister(0x0038,0x0208);    
    	ili9320_WriteRegister(0x0039,0x0F0B); 
		ili9320_Delay(15);

	  
    	ili9320_WriteRegister(0x0041,0x0002);   
    	ili9320_WriteRegister(0x0060,0x2700);    
    	ili9320_WriteRegister(0x0061,0x0001);    
    	ili9320_WriteRegister(0x0090,0x0119);    
    	ili9320_WriteRegister(0x0092,0x010A);  
    	ili9320_WriteRegister(0x0093,0x0004);   
    	ili9320_WriteRegister(0x00A0,0x0100);    
    	ili9320_WriteRegister(0x0007,0x0001);    
		
		ili9320_Delay(15);
		ili9320_WriteRegister(0x0007,0x0021);
		ili9320_Delay(15);
		    
    	ili9320_WriteRegister(0x0007,0x0023);
		ili9320_Delay(15);
    	
		ili9320_WriteRegister(0x0007,0x0033);
		ili9320_Delay(15);
		   
    	ili9320_WriteRegister(0x0007,0x0133);
		ili9320_Delay(50000);
		  
    	ili9320_WriteRegister(0x00A0,0x0000); 
		ili9320_Delay(20);
   } 
	else if(DeviceCode==0x1505)
	{
// second release on 3/5  ,luminance is acceptable,water wave appear during camera preview
        LCD_WriteReg(0x0007,0x0000);
        ili9320_Delay(5);
        LCD_WriteReg(0x0012,0x011C);//0x011A   why need to set several times?
        LCD_WriteReg(0x00A4,0x0001);//NVM
    //
        LCD_WriteReg(0x0008,0x000F);
        LCD_WriteReg(0x000A,0x0008);
        LCD_WriteReg(0x000D,0x0008);
       
  //GAMMA CONTROL/
        LCD_WriteReg(0x0030,0x0707);
        LCD_WriteReg(0x0031,0x0007); //0x0707
        LCD_WriteReg(0x0032,0x0603); 
        LCD_WriteReg(0x0033,0x0700); 
        LCD_WriteReg(0x0034,0x0202); 
        LCD_WriteReg(0x0035,0x0002); //?0x0606
        LCD_WriteReg(0x0036,0x1F0F);
        LCD_WriteReg(0x0037,0x0707); //0x0f0f  0x0105
        LCD_WriteReg(0x0038,0x0000); 
        LCD_WriteReg(0x0039,0x0000); 
        LCD_WriteReg(0x003A,0x0707); 
        LCD_WriteReg(0x003B,0x0000); //0x0303
        LCD_WriteReg(0x003C,0x0007); //?0x0707
        LCD_WriteReg(0x003D,0x0000); //0x1313//0x1f08
        ili9320_Delay(5);
        LCD_WriteReg(0x0007,0x0001);
        LCD_WriteReg(0x0017,0x0001);   //Power supply startup enable
        ili9320_Delay(5);
  
  //power control//
        LCD_WriteReg(0x0010,0x17A0); 
        LCD_WriteReg(0x0011,0x0217); //reference voltage VC[2:0]   Vciout = 1.00*Vcivl
        LCD_WriteReg(0x0012,0x011E);//0x011c  //Vreg1out = Vcilvl*1.80   is it the same as Vgama1out ?
        LCD_WriteReg(0x0013,0x0F00); //VDV[4:0]-->VCOM Amplitude VcomL = VcomH - Vcom Ampl
        LCD_WriteReg(0x002A,0x0000);  
        LCD_WriteReg(0x0029,0x000A); //0x0001F  Vcomh = VCM1[4:0]*Vreg1out    gate source voltage??
        LCD_WriteReg(0x0012,0x013E); // 0x013C  power supply on
           //Coordinates Control//
        LCD_WriteReg(0x0050,0x0000);//0x0e00
        LCD_WriteReg(0x0051,0x00EF); 
        LCD_WriteReg(0x0052,0x0000); 
        LCD_WriteReg(0x0053,0x013F); 
    //Pannel Image Control//
        LCD_WriteReg(0x0060,0x2700); 
        LCD_WriteReg(0x0061,0x0001); 
        LCD_WriteReg(0x006A,0x0000); 
        LCD_WriteReg(0x0080,0x0000); 
    //Partial Image Control//
        LCD_WriteReg(0x0081,0x0000); 
        LCD_WriteReg(0x0082,0x0000); 
        LCD_WriteReg(0x0083,0x0000); 
        LCD_WriteReg(0x0084,0x0000); 
        LCD_WriteReg(0x0085,0x0000); 
  //Panel Interface Control//
        LCD_WriteReg(0x0090,0x0013); //0x0010 frenqucy
        LCD_WriteReg(0x0092,0x0300); 
        LCD_WriteReg(0x0093,0x0005); 
        LCD_WriteReg(0x0095,0x0000); 
        LCD_WriteReg(0x0097,0x0000); 
        LCD_WriteReg(0x0098,0x0000); 
  
        LCD_WriteReg(0x0001,0x0100); 
        LCD_WriteReg(0x0002,0x0700); 
        LCD_WriteReg(0x0003,0x1030); 
        LCD_WriteReg(0x0004,0x0000); 
        LCD_WriteReg(0x000C,0x0000); 
        LCD_WriteReg(0x000F,0x0000); 
        LCD_WriteReg(0x0020,0x0000); 
        LCD_WriteReg(0x0021,0x0000); 
        LCD_WriteReg(0x0007,0x0021); 
        ili9320_Delay(20);
        LCD_WriteReg(0x0007,0x0061); 
        ili9320_Delay(20);
        LCD_WriteReg(0x0007,0x0173); 
        ili9320_Delay(20);
	}							 

  Delay_nms(1);
  Lcd_Clear(Blue2);
  Lcd_Clear(White);

}

/*******************************************************************************
* Function Name  : LCD_SetTextColor
* Description    : Sets the Text color.
* Input          : - Color: specifies the Text color code RGB(5-6-5).
* Output         : - TextColor: Text color global variable used by LCD_DrawChar
*                  and LCD_DrawPicture functions.
* Return         : None
*******************************************************************************/
void LCD_SetTextColor(vu16 Color)
{
  TextColor = Color;
}

/*******************************************************************************
* Function Name  : LCD_SetBackColor
* Description    : Sets the Background color.
* Input          : - Color: specifies the Background color code RGB(5-6-5).
* Output         : - BackColor: Background color global variable used by 
*                  LCD_DrawChar and LCD_DrawPicture functions.
* Return         : None
*******************************************************************************/
void LCD_SetBackColor(vu16 Color)
{
  BackColor = Color;
}

/*******************************************************************************
* Function Name  : LCD_ClearLine
* Description    : Clears the selected line.
* Input          : - Line: the Line to be cleared.
*                    This parameter can be one of the following values:
*                       - Linex: where x can be 0..9
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_ClearLine(u8 Line)
{
  LCD_DisplayStringLine(Line, "                    ");
}

/*******************************************************************************
* Function Name  : LCD_Clear
* Description    : Clears the hole LCD.
* Input          : Color: the color of the background.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_Clear(u16 Color)		
{
  Lcd_Clear(Color);
}

/*******************************************************************************
* Function Name  : LCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position. 
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
	if(DeviceCode==0x8999||DeviceCode==0x8989)
	{
	 	LCD_WR_REG(0x004e,Xpos);        //��
    	LCD_WR_REG(0x004f,Ypos);  //��
	}
 	if(DeviceCode==0x9919)
	{
		LCD_WR_REG(0x004e,Xpos); // ��
  		LCD_WR_REG(0x004f,Ypos); // ��	
	}
	else
	{
  		LCD_WR_REG(0x0020,Xpos); // ��
  		LCD_WR_REG(0x0021,Ypos); // ��
	}
}
/*******************************************************************************
* Function Name  : LCD_DrawChar
* Description    : Draws a character on LCD.
* Input          : - Xpos: the Line where to display the character shape.
*                    This parameter can be one of the following values:
*                       - Linex: where x can be 0..9
*                  - Ypos: start column address.
*                  - c: pointer to the character data.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_DrawChar(u8 Xpos, u16 Ypos, uc16 *c)
{
  u32 index = 0, i = 0;
  u8  Xaddress = 0;
   
  Xaddress = Xpos;
  
  LCD_SetCursor(Xaddress, Ypos);
  
  for(index = 0; index < 24; index++)	  //ÿ��ASCII��16X24������24��16λ���ݡ�
  {
    Lcd_WR_Start(); /* Prepare to write GRAM */
    for(i = 0; i < 16; i++)	  //ÿ��16λ�������
    {
      if((c[index] & (1 << i)) == 0x00)
      {
        DataToWrite(BackColor);	 //��Ҫ�ĳ�8λ
		Clr_nWr;
		Set_nWr;

      }
      else
      {
        DataToWrite(TextColor);
		Clr_nWr;
    	Set_nWr;

      }
    }
    Xaddress++;
    LCD_SetCursor(Xaddress, Ypos);
  }
}




/*******************************************************************************
* Function Name  : LCD_DisplayChar
* Description    : Displays one character (16dots width, 24dots height).
* Input          : - Line: the Line where to display the character shape .
*                    This parameter can be one of the following values:
*                       - Linex: where x can be 0..9
*                  - Column: start column address.
*                  - Ascii: character ascii code, must be between 0x20 and 0x7E.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_DisplayChar(u8 Line, u16 Column, u8 Ascii)
{ 
  u8 temp;
  temp = Ascii - 32;
  LCD_DrawChar(Line, Column, &ASCII_Table[temp * 24]);
}


/*******************************************************************************
* Function Name  : LCD_DisplayStringLine
* Description    : Displays a maximum of 20 char on the LCD.
* Input          : - Line: the Line where to display the character shape .
*                    This parameter can be one of the following values:
*                       - Linex: where x can be 0..9
*                  - *ptr: pointer to string to display on LCD.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_DisplayStringLine(u8 Line, u8 *ptr)
{
  u32 i = 0;
  u16 refcolumn = 319;

  /* Send the string character by character on lCD */
  while ((*ptr != 0) & (i < 20))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, *ptr);
    /* Decrement the column position by 16 */
    refcolumn -= 16;
    /* Point on the next character */
    ptr++;
    /* Increment the character counter */
    i++;
  }
}
/*******************************************************************************
* Function Name  : LCD_SetDisplayWindow
* Description    : Sets a display window
* Input          : - Xpos: specifies the X buttom left position.
*                  - Ypos: specifies the Y buttom left position.
*                  - Height: display window height.
*                  - Width: display window width.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_SetDisplayWindow(u8 Xpos, u16 Ypos, u8 Height, u16 Width)
{
  /* Horizontal GRAM Start Address */
  if(Xpos >= Height)
  {
    LCD_WR_REG(R80, (Xpos - Height + 1));
  }
  else
  {
    LCD_WR_REG(R80, 0);
  }
  /* Horizontal GRAM End Address */
  LCD_WR_REG(R81, Xpos);
  /* Vertical GRAM Start Address */
  if(Ypos >= Width)
  {
    LCD_WR_REG(R82, (Ypos - Width + 1));
  }  
  else
  {
    LCD_WR_REG(R82, 0);
  }
  /* Vertical GRAM End Address */
  LCD_WR_REG(R83, Ypos);

  LCD_SetCursor(Xpos, Ypos);
}


/*******************************************************************************
* Function Name  : LCD_WindowModeDisable
* Description    : Disables LCD Window mode.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_WindowModeDisable(void)
{
  LCD_SetDisplayWindow(239, 0x13F, 240, 320);
  LCD_WR_REG(R3, 0x1018);    
}

/*******************************************************************************
* Function Name  : LCD_DrawLine
* Description    : Displays a line.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position.
*                  - Length: line length.
*                  - Direction: line direction.
*                    This parameter can be one of the following values: Vertical 
*                    or Horizontal.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_DrawLine(u8 Xpos, u16 Ypos, u16 Length, u8 Direction)
{
  u32 i = 0;
  
  LCD_SetCursor(Xpos, Ypos);

  if(Direction == Horizontal)
  {
    Lcd_WR_Start(); /* Prepare to write GRAM */
    for(i = 0; i < Length; i++)
    {
      LCD_WriteRAM(TextColor);
  	  Clr_nWr;
      Set_nWr;

    }
  }
  else
  {
    for(i = 0; i < Length; i++)
    {
      LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
      LCD_WriteRAM(TextColor);
 	  Clr_nWr;
      Set_nWr;

      Xpos++;
      LCD_SetCursor(Xpos, Ypos);
    }
  }
}

/*******************************************************************************
* Function Name  : LCD_DrawRect
* Description    : Displays a rectangle.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position.
*                  - Height: display rectangle height.
*                  - Width: display rectangle width.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_DrawRect(u8 Xpos, u16 Ypos, u8 Height, u16 Width)
{
  LCD_DrawLine(Xpos, Ypos, Width, Horizontal);
  LCD_DrawLine((Xpos + Height), Ypos, Width, Horizontal);
  
  LCD_DrawLine(Xpos, Ypos, Height, Vertical);
  LCD_DrawLine(Xpos, (Ypos - Width + 1), Height, Vertical);
}

/*******************************************************************************
* Function Name  : LCD_DrawCircle
* Description    : Displays a circle.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position.
*                  - Height: display rectangle height.
*                  - Width: display rectangle width.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_DrawCircle(u8 Xpos, u16 Ypos, u16 Radius)
{
  s32 D;/* Decision Variable */ 
  u32 CurX;/* Current X Value */
  u32 CurY;/* Current Y Value */ 
  
  D = 3 - (Radius << 1);
  CurX = 0;
  CurY = Radius;
  
  while (CurX <= CurY)
  {
    LCD_SetCursor(Xpos + CurX, Ypos + CurY);
    Lcd_WR_Start(); /* Prepare to write GRAM */
    LCD_WriteRAM(TextColor);
	Clr_nWr;
   	Set_nWr;


    LCD_SetCursor(Xpos + CurX, Ypos - CurY);
    Lcd_WR_Start(); /* Prepare to write GRAM */
    LCD_WriteRAM(TextColor);
	Clr_nWr;
   	Set_nWr;

    LCD_SetCursor(Xpos - CurX, Ypos + CurY);
    Lcd_WR_Start(); /* Prepare to write GRAM */
    LCD_WriteRAM(TextColor);
	Clr_nWr;
   	Set_nWr;

    LCD_SetCursor(Xpos - CurX, Ypos - CurY);
    Lcd_WR_Start(); /* Prepare to write GRAM */
    LCD_WriteRAM(TextColor);
	Clr_nWr;
   	Set_nWr;

    LCD_SetCursor(Xpos + CurY, Ypos + CurX);
    Lcd_WR_Start(); /* Prepare to write GRAM */
    LCD_WriteRAM(TextColor);
	Clr_nWr;
   	Set_nWr;

    LCD_SetCursor(Xpos + CurY, Ypos - CurX);
    Lcd_WR_Start(); /* Prepare to write GRAM */
    LCD_WriteRAM(TextColor);
	Clr_nWr;
   	Set_nWr;

    LCD_SetCursor(Xpos - CurY, Ypos + CurX);
    Lcd_WR_Start(); /* Prepare to write GRAM */
    LCD_WriteRAM(TextColor);
	Clr_nWr;
   	Set_nWr;

    LCD_SetCursor(Xpos - CurY, Ypos - CurX);
    LCD_WriteRAM_Prepare(); /* Prepare to write GRAM */
    LCD_WriteRAM(TextColor);
	Clr_nWr;
   	Set_nWr;

    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
}


/*******************************************************************************
* Function Name  : LCD_DrawMonoPict
* Description    : Displays a monocolor picture.
* Input          : - Pict: pointer to the picture array.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_DrawMonoPict(uc32 *Pict)
{
  u32 index = 0, i = 0;

  LCD_SetCursor(0, 319); 

  Lcd_WR_Start(); /* Prepare to write GRAM */
  for(index = 0; index < 2400; index++)
  {
    for(i = 0; i < 32; i++)
    {
      if((Pict[index] & (1 << i)) == 0x00)
      {
        LCD_WriteRAM(BackColor);
  	    Clr_nWr;
   	    Set_nWr;

      }
      else
      {
        LCD_WriteRAM(TextColor);
  	    Clr_nWr;
   	    Set_nWr;

      }
    }
  }
}

/*******************************************************************************
* Function Name  : LCD_WriteBMP
* Description    : Displays a bitmap picture loaded in the internal Flash.
* Input          : - BmpAddress: Bmp picture address in the internal Flash.
* Output         : None
* Return         : None
*******************************************************************************/
void LCD_WriteBMP(u32 BmpAddress)
{
  u32 index = 0, size = 0;

  /* Read bitmap size */
  size = *(vu16 *) (BmpAddress + 2);
  size |= (*(vu16 *) (BmpAddress + 4)) << 16;

  /* Get bitmap data address offset */
  index = *(vu16 *) (BmpAddress + 10);
  index |= (*(vu16 *) (BmpAddress + 12)) << 16;

  size = (size - index)/2;

  BmpAddress += index;

  /* Set GRAM write direction and BGR = 1 */
  /* I/D=00 (Horizontal : decrement, Vertical : decrement) */
  /* AM=1 (address is updated in vertical writing direction) */
  LCD_WriteReg(R3, 0x1008);
 
  Lcd_WR_Start();
 
  for(index = 0; index < size; index++)
  {
    LCD_WriteRAM(*(vu16 *)BmpAddress);
    Clr_nWr;
    Set_nWr;

    BmpAddress += 2;
  }
 
  /* Set GRAM write direction and BGR = 1 */
  /* I/D = 01 (Horizontal : increment, Vertical : decrement) */
  /* AM = 1 (address is updated in vertical writing direction) */
  LCD_WriteReg(R3, 0x1018);
}


/*************************************************
��������Lcd�����㶨λ����
���ܣ�ָ��320240Һ���ϵ�һ����Ϊд���ݵ���ʼ��
��ڲ�����x ���� 0~239
          y ���� 0~319
����ֵ����
*************************************************/
void Lcd_SetCursor(u16 Xpos,u16 Ypos)
{ 
	if(DeviceCode==0x8999||DeviceCode==0x8989)
	{
	 	LCD_WR_REG(0x004e,Xpos);        //��
    	LCD_WR_REG(0x004f,Ypos);  //��
	}
 	if(DeviceCode==0x9919)
	{
		LCD_WR_REG(0x004e,Xpos); // ��
  		LCD_WR_REG(0x004f,Ypos); // ��	
	}
	else
	{
  		LCD_WR_REG(0x0020,Xpos); // ��
  		LCD_WR_REG(0x0021,Ypos); // ��
	}
}


/**********************************************
��������Lcdȫ����������
���ܣ���Lcd������Ϊָ����ɫ
��ڲ�����color ָ��Lcdȫ����ɫ RGB(5-6-5)
����ֵ����
***********************************************/
void Lcd_Clear(u16 Color)
{
	u32 temp;
  
	Lcd_SetCursor(0x00, 0x0000);
	LCD_WR_REG(0x0050,0x00);//ˮƽ GRAM��ʼλ��
	LCD_WR_REG(0x0051,239);//ˮƽGRAM��ֹλ��
	LCD_WR_REG(0x0052,0x00);//��ֱGRAM��ʼλ��
	LCD_WR_REG(0x0053,319);//��ֱGRAM��ֹλ��   
	Lcd_WR_Start();
	Set_Rs;
  
	for (temp = 0; temp < 76800; temp++)
	{
		DataToWrite(Color);
		Clr_nWr;
		Set_nWr;
	}
  
	Set_Cs;
}

void Lcd_SetBoxRestore(void)
{

	Lcd_SetCursor(0x00, 0x0000);
	LCD_WR_REG(0x0050,0x00);//ˮƽ GRAM��ʼλ��
	LCD_WR_REG(0x0051,239);//ˮƽGRAM��ֹλ��
	LCD_WR_REG(0x0052,0x00);//��ֱGRAM��ʼλ��
	LCD_WR_REG(0x0053,319);//��ֱGRAM��ֹλ��   
	Lcd_WR_Start();
	Set_Rs;
 	Set_Cs;
}


/**********************************************
��������Lcd��ѡ����
���ܣ�ѡ��Lcd��ָ���ľ�������

ע�⣺xStart�� yStart������Ļ����ת���ı䣬λ���Ǿ��ο���ĸ���

��ڲ�����xStart x�������ʼ��
          ySrart y�������ֹ��
          xLong Ҫѡ�����ε�x���򳤶�
          yLong  Ҫѡ�����ε�y���򳤶�
����ֵ����
***********************************************/
void Lcd_SetBox(u8 xStart,u16 yStart,u8 xLong,u16 yLong,u16 x_offset,u16 y_offset)
{
  
#if ID_AM==000    
	Lcd_SetCursor(xStart+xLong-1+x_offset,yStart+yLong-1+y_offset);

#elif ID_AM==001
	Lcd_SetCursor(xStart+xLong-1+x_offset,yStart+yLong-1+y_offset);
     
#elif ID_AM==010
	Lcd_SetCursor(xStart+x_offset,yStart+yLong-1+y_offset);
     
#elif ID_AM==011 
	Lcd_SetCursor(xStart+x_offset,yStart+yLong-1+y_offset);
     
#elif ID_AM==100
	Lcd_SetCursor(xStart+xLong-1+x_offset,yStart+y_offset);     
     
#elif ID_AM==101
	Lcd_SetCursor(xStart+xLong-1+x_offset,yStart+y_offset);     
     
#elif ID_AM==110
	Lcd_SetCursor(xStart+x_offset,yStart+y_offset); 
     
#elif ID_AM==111
	Lcd_SetCursor(xStart+x_offset,yStart+y_offset);  
     
#endif
     
	LCD_WR_REG(0x0050,xStart+x_offset);//ˮƽ GRAM��ʼλ��
	LCD_WR_REG(0x0051,xStart+xLong-1+x_offset);//ˮƽGRAM��ֹλ��
	LCD_WR_REG(0x0052,yStart+y_offset);//��ֱGRAM��ʼλ��
	LCD_WR_REG(0x0053,yStart+yLong-1+y_offset);//��ֱGRAM��ֹλ�� 
}


void Lcd_ColorBox(u8 xStart,u16 yStart,u8 xLong,u16 yLong,u16 Color)
{
	u32 temp;
  
	Lcd_SetBox(xStart,yStart,xLong,yLong,0,0);
	Lcd_WR_Start();
	Set_Rs;
  
	for (temp=0; temp<xLong*yLong; temp++)
	{
		DataToWrite(Color);
		Clr_nWr;
		Set_nWr;
	}

	Set_Cs;
}


void Lcd_ClearCharBox(u8 x,u16 y,u16 Color)
{
	u32 temp;
  
	Lcd_SetBox(x*8,y*16,8,16,0,0); 
	Lcd_WR_Start();
	Set_Rs;
  
	for (temp=0; temp < 128; temp++)
	{
		DataToWrite(Color); 
		Clr_nWr;
		//Delay_nus(22);
		Set_nWr; 
	}
	
	Set_Cs;
}


/****************************************************************
��������Lcdд1��ASCII�ַ�����
��ڲ�����x,�������꣬�����ҷֱ���0~29 
          y,�������꣬���ϵ��·ֱ�Ϊ0~19
          CharColaor,�ַ�����ɫ 
          CharBackColor,�ַ�������ɫ 
         ASCIICode,��Ӧ�ַ���ASCII��
Ҳ����˵��320240�ֱ��ʵ���ʾ������������ʾ30��ASCII�ַ�����������ʾ20��
����ֵ����

ע�⣡���������������ʹ�ô˺�����Ӧ�ü���Lcd_Rs_H()��Set_Cs;Ϊ���Ż�ϵͳʡȥ��
���ָ�����˺���ִ�е���һ�������д�����RS_L�������д�뽫����
����ΪILI9320��Ϊ��RS_Lʱд���������
*****************************************************************/
void Lcd_WriteASCII(u8 x,u8 y,u16 x_offset,u16 y_offset,u16 CharColor,u16 CharBackColor,u8 ASCIICode)
{
	u8 RowCounter,BitCounter;
	u8 *ASCIIPointer;
	u8 ASCIIBuffer[16];
  
#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)
	Lcd_SetBox(x*8,y*16,8,16,x_offset,y_offset);    
#else
	Lcd_SetBox(x*16,y*8,16,8,x_offset,y_offset);    
#endif 
        
	Lcd_WR_Start();
	GetASCIICode(ASCIIBuffer,ASCIICode,ASCII_Offset);//ȡ����ַ�����ʾ����
	ASCIIPointer=ASCIIBuffer;
  
	for (RowCounter=0; RowCounter<16; RowCounter++)
	{ 
		for (BitCounter=0; BitCounter<8; BitCounter++)
		{
			if ((*ASCIIPointer&(0x80 >> BitCounter)) == 0x00)
			{
				//Set_Rs;
				DataToWrite(CharBackColor);
				Clr_nWr;
				Set_nWr;
			}
			else
			{
				//Set_Rs;
				DataToWrite(CharColor);
				Clr_nWr;
				Set_nWr; 
			}
		}
		ASCIIPointer++;
	}
	// Set_Cs;
}


void Lcd_WriteASCIIClarity(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,u8 ASCIICode)
{
  
  GPIO_InitTypeDef GPIO_InitStructure;
  u8 RowCounter,BitCounter;
  u8 *ASCIIPointer;
  u8 ASCIIBuffer[16];
  u16 Temp;
        
#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)
  Lcd_SetBox(x*8,y*16,8,16,x_offset,y_offset);    
#else
  Lcd_SetBox(x*16,y*8,16,8,x_offset,y_offset);    
#endif
    
  Lcd_WR_Start();
  GetASCIICode(ASCIIBuffer,ASCIICode,ASCII_Offset);//ȡ����ַ�����ʾ����
  ASCIIPointer=ASCIIBuffer;
  
  for (RowCounter=0; RowCounter<16; RowCounter++)
  { 
    for(BitCounter=0;BitCounter<8;BitCounter++)
    {
      if((*ASCIIPointer  & (0x80 >> BitCounter)) == 0x00)
      {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr;
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }
    }
    ASCIIPointer++;
  }
  // Set_Cs;
  
}
/**************************************************************
��Һ����д��32*16��ASCII�ַ�
���������
x������   x :(0~14)
y������   y :(0~9)
x��ƫ���� x_offset:(��������0~239)
y��ƫ���� y_offset:(��������0~319)  ע�⣺ƫ�������ܹ��󣬵�x���򳬳�16��y���򳬹�32
                                          Ҳ����32*16�ַ���Сʱǿ�ҽ���ʹ��x��y������
                                          ��Ȼƫ��������ʵ�ִ�Χƫ�Ƶ��ǿɶ��Ժ�ά
                                          ���Զ��ϲ�
�ַ���ɫ CharColor:�ַ���ɫ
������ɫ CharBackColor��������ɫ
***************************************************************/
void Lcd_Write32X16ASCII(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,u16 CharBackColor,u8 ASCIICode)
{

  u8 RowCounter,BitCounter;
  u8 *ASCIIPointer;
  u8 ASCIIBuffer[16];
  
#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)
  Lcd_SetBox(x*16,y*32,16,32,x_offset,y_offset);    
#else
  Lcd_SetBox(x*32,y*16,32,16,x_offset,y_offset);    
#endif
    
  Lcd_WR_Start();
  GetASCIICode(ASCIIBuffer,ASCIICode,ASCII_Offset);//ȡ����ַ�����ʾ����
  ASCIIPointer=ASCIIBuffer;
  
  for(RowCounter=0; RowCounter<16; RowCounter++)
  { 
    for(BitCounter=0; BitCounter<8; BitCounter++)
    {
      if((*ASCIIPointer  & (0x80 >> BitCounter)) == 0x00)
      {
        //Set_Rs;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr;
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }      
    }     
    for(BitCounter=0; BitCounter<8; BitCounter++)
    {
      if((*ASCIIPointer  & (0x80 >> BitCounter)) == 0x00)
      {         
        //Set_Rs;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr;
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }
    }
    ASCIIPointer++;
  }
  // Set_Cs;
}
//���Ե���ʹ�õ�дһ��ASCII�ַ�����
void Lcd_Write32X16ASCIIWrite(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,u16 CharBackColor,u8 ASCIICode)
{
        u8 RowCounter,BitCounter;
        u8 *ASCIIPointer;
        
        //����ASCII�ַ�λ��
#if   (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110) 
        
        Lcd_SetBox(x*16,y*32,16,32,x_offset,y_offset);
        
#else
        
        Lcd_SetBox(x*32,y*16,32,16,x_offset,y_offset);
        
#endif
        
        Lcd_WR_Start();
//        CatchASCII(ASCIICode,ASCII_Offset);//ȡ����ַ�����ʾ����
//        ASCIIPointer=CharBuffer;
        for(RowCounter=0;RowCounter<16;RowCounter++)
        { 
          for(BitCounter=0;BitCounter<8;BitCounter++)
          {
            if((*ASCIIPointer  & (0x80 >> BitCounter)) == 0x00)
            {
               
               Set_Rs;
               DataToWrite(CharBackColor);
               Clr_nWr;
               Set_nWr;
               DataToWrite(CharBackColor);
               Clr_nWr;
               Set_nWr;

            }
            else
            {
               Set_Rs;
               DataToWrite(CharColor);
               Clr_nWr;
               Set_nWr; 
               DataToWrite(CharColor);
               Clr_nWr;
               Set_nWr; 
            }
            
          }
          
        for(BitCounter=0;BitCounter<8;BitCounter++)
          {
            if((*ASCIIPointer  & (0x80 >> BitCounter)) == 0x00)
            {
               
               Set_Rs;
               DataToWrite(CharBackColor);
               Clr_nWr;
               Set_nWr;
               DataToWrite(CharBackColor);
               Clr_nWr;
               Set_nWr;

            }
            else
            {
               Set_Rs;
               DataToWrite(CharColor);
               Clr_nWr;
               Set_nWr; 
               DataToWrite(CharColor);
               Clr_nWr;
               Set_nWr; 
            }
            
          }
           ASCIIPointer++;
        }
         Set_Cs;
}
void Lcd_Write32X16ASCIIClarity(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,u8 ASCIICode)
{

  GPIO_InitTypeDef GPIO_InitStructure;
  u8 RowCounter,BitCounter;
  u8 *ASCIIPointer;
  u8 ASCIIBuffer[16];
  u16 Temp;

#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)
  Lcd_SetBox(x*16,y*32,16,32,x_offset,y_offset);    
#else
  Lcd_SetBox(x*32,y*16,32,16,x_offset,y_offset);    
#endif
    
  Lcd_WR_Start();
  GetASCIICode(ASCIIBuffer,ASCIICode,ASCII_Offset);//ȡ����ַ�����ʾ����
  ASCIIPointer=ASCIIBuffer;
  
  for(RowCounter=0; RowCounter<16; RowCounter++)
  { 
    for(BitCounter=0; BitCounter<8; BitCounter++)
    {
      if((*ASCIIPointer  & (0x80 >> BitCounter)) == 0x00)
      {
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr;
                     
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr;
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }    
    }
          
    for(BitCounter=0; BitCounter<8; BitCounter++)
    {
      if((*ASCIIPointer  & (0x80 >> BitCounter)) == 0x00)
      {
        //Set_Rs;
              
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;       
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;          
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;     
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr;
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr;
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }            
    }
    ASCIIPointer++;
  }
  // Set_Cs;
}
/************************************************************
��������Lcdд�ַ�������
���ܣ���ָ��λ��д��һ�������ַ������������Զ����й���
��ڲ�����x,�������꣬�����ҷֱ���0~29 
          y,�������꣬���ϵ��·ֱ�Ϊ0~19
          CharColaor,�ַ�����ɫ 
          CharBackColor,�ַ�������ɫ 
          *s ָ��Ҫд���ַ���
����ֵ����
*************************************************************/
void Lcd_WriteString(u8 x,u8 y,u16 x_offset,u16 y_offset,u16 CharColor,u16 CharBackColor,char *s)
{

  u8 databuff;
  Set_Rs;

  do
  {
    databuff=*s++;  
    Lcd_WriteASCII(x,y,x_offset,y_offset,CharColor,CharBackColor,databuff);
    
#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)  
    if (x<29)
    {
      x++;
    } 
    else if (y<19)
    {
      x=0;
      y++;
    }   
    else
    {
      x=0;
      y=0;
    }
       
#else
    if (y<39)
    {
      y++;
    }
    else if (x<14)
    {
      y=0;
      x++;
    }
    else
    {
      x=0;
      y=0;
    }
#endif 
       
  }
  while (*s!=0);
  
  Set_Cs;
  
}
void Lcd_WriteStringClarity(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,char *s)
{

  u8 databuff;
  Set_Rs;

  do
  {
    databuff=*s++;  
    Lcd_WriteASCIIClarity(x,y,x_offset,y_offset,CharColor,databuff);
    
#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)  
    if (x<29)
    {
      x++;
    } 
    else if (y<19)
    {
      x=0;
      y++;
    }   
    else
    {
      x=0;
      y=0;
    }       
#else
    if (y<39)
    {
      y++;
    }
    else if (x<14)
    {
      y=0;
      x++;
    }
    else
    {
      x=0;
      y=0;
    }
#endif
    
  }
  while(*s!=0);
  
  Set_Cs;
  
}
/****************************************
д��32X16��ASCII�ַ���
*****************************************/
void Lcd_Write32X16String(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,u16 CharBackColor,char *s)
{

  u8 databuff;
  Set_Rs;
  
  do
  {
    databuff=*s++;  
    Lcd_Write32X16ASCII(x,y,x_offset,y_offset,CharColor,CharBackColor,databuff);
    
#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)  
    if (x<14)
    {
      x++;
    } 
    else if (y<9)
    {
      x=0;
      y++;
    }   
    else
    {
      x=0;
      y=0;
    }   
#else
    if (y<6)
    {
      y++;
    }
    else if (x<19)
    {
      y=0;
      x++;
    }
    else
    {
      x=0;
      y=0;
    }   
#endif
      
  }
  while (*s!=0);
    
  Set_Cs;
  
}
void Lcd_Write32X16StringClarity(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,char *s)
{

  u8 databuff;
  Set_Rs;
  
  do
  {
    databuff=*s++;  
    Lcd_Write32X16ASCIIClarity(x,y,x_offset,y_offset,CharColor,databuff);

#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)  
    if (x<14)
    {
      x++;
    } 
    else if (y<9)
    {
      x=0;
      y++;
    }   
    else
    {
      x=0;
      y=0;
    }     
#else
    if (y<6)
    {
      y++;
    }
    else if (x<19)
    {
      y=0;
      x++;
    }
    else
    {
      x=0;
      y=0;
    }
#endif
    
  }
  while(*s!=0);
  
  Set_Cs;
  
}
/****************************************************************
��������Lcdд1�����ĺ���
��ڲ�����x,�������꣬�����ҷֱ���0~14
          y,�������꣬���ϵ��·ֱ�Ϊ0~19
          CharColaor,�ַ�����ɫ 
          CharBackColor,�ַ�������ɫ 
         ASCIICode,��Ӧ���ĵı���
Ҳ����˵��320240�ֱ��ʵ���ʾ������������ʾ15�����ַ�����������ʾ20��
����ֵ����

ע�⣡���������������ʹ�ô˺�����Ӧ�ü���Lcd_Rs_H()��Set_Cs;Ϊ���Ż�ϵͳʡȥ��
���ָ�����˺���ִ�е���һ�������д�����RS_L�������д�뽫����
����ΪILI9320��Ϊ��RS_Lʱд���������
*****************************************************************/
void Lcd_WriteChinese(u8 x,u8 y,u16 x_offset,u16 y_offset,u16 CharColor,u16 CharBackColor,u16 ChineseCode)
{

  u8 ByteCounter,BitCounter;
  u8 *ChinesePointer;
  u8 ChineseBuffer[32];
  u8 temp;      
  Lcd_SetBox(x*16,y*16,16,16,x_offset,y_offset);         
  Lcd_WR_Start();
  GetSpiChineseCode(ChineseBuffer,ChineseCode,Chinese_Offset);
  ChinesePointer=ChineseBuffer;
  for(ByteCounter=0; ByteCounter<32; ByteCounter++)
  { 
    for(BitCounter=0;BitCounter<8;BitCounter++)
    {
      if((*ChinesePointer & (0x01 << BitCounter)) == 0x00)
      {
//        Set_Rs;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr; 
      }
      else
      {
//        Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }     
    }
    ChinesePointer++;
  }
  Set_Cs;
  
}
void Lcd_WriteChineseClarity(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,u16 ChineseCode)
{

  GPIO_InitTypeDef GPIO_InitStructure;
  u16 Temp;
  u8 ByteCounter,BitCounter;
  u8 *ChinesePointer;
  u8 ChineseBuffer[32];
  
  Lcd_SetBox(x*16,y*16,16,16,x_offset,y_offset);         
  Lcd_WR_Start();
  GetSpiChineseCode(ChineseBuffer,ChineseCode,Chinese_Offset);
  ChinesePointer=ChineseBuffer;
  
  for(ByteCounter=0; ByteCounter<32; ByteCounter++)
  { 
    for(BitCounter=0; BitCounter<8; BitCounter++)
    {
      if((*ChinesePointer & (0x80 >> BitCounter)) == 0x00)
      {
        //Set_Rs;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr;
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }     
    }
  ChinesePointer++;
  }
  //Set_Cs;
  
}

/****************************************************************
��������Lcdд1�����ĺ���
��ڲ�����x,�������꣬�����ҷֱ���0~10
          y,�������꣬���ϵ��·ֱ�Ϊ0~13
          CharColaor,�ַ�����ɫ 
          CharBackColor,�ַ�������ɫ 
         ASCIICode,��Ӧ���ĵı���
Ҳ����˵��320240�ֱ��ʵ���ʾ������������ʾ10�����ַ�����������ʾ13��
����ֵ����

ע�⣡���������������ʹ�ô˺�����Ӧ�ü���Lcd_Rs_H()��Set_Cs;Ϊ���Ż�ϵͳʡȥ��
���ָ�����˺���ִ�е���һ�������д�����RS_L�������д�뽫����
����ΪILI9320��Ϊ��RS_Lʱд���������
*****************************************************************/
void Lcd_Write24X24Chinese(u8 x,u8 y,u16 x_offset,u16 y_offset,u16 CharColor,u16 CharBackColor,u16 ChineseCode)
{

  u8 ByteCounter,BitCounter;
  u8 *ChinesePointer;
  u8 ChineseBuffer[72];
  u8 temp;  
  Lcd_SetBoxRestore();
  Lcd_SetBox(x*24,319-y*24,24,24,x_offset,y_offset);         
  Lcd_WR_Start();
  GetSpiChineseCode(ChineseBuffer,ChineseCode,Chinese_Offset);
//  GetChineseCode(ChineseBuffer,ChineseCode,Chinese_Offset);
  ChinesePointer=ChineseBuffer;
  for(ByteCounter=0; ByteCounter<72; ByteCounter++)
  { 
    for(BitCounter=0;BitCounter<8;BitCounter++)
    {
      if((*ChinesePointer & (0x01 << BitCounter)) == 0x00)
      {
//        Set_Rs;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr; 
      }
      else
      {
//        Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }     
    }
    ChinesePointer++;
  }
  Lcd_SetBoxRestore();
  Set_Cs;
  
}

void Lcd_Write24X24ChineseString(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,u16 CharBackColor,char *s)
{
   
  u8 databuffer;
  u16 ChineseCode;
   
  Set_Rs;
  do
  {
    databuffer=*s++;
    ChineseCode=databuffer<<8;
    ChineseCode=ChineseCode|*s++;
    Lcd_Write24X24Chinese(x,y,x_offset,y_offset,CharColor,CharBackColor,ChineseCode);
       
#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)  
    if (x<9)
    {
      x++;
    }
    else if (y<12)
    {
      x=0;
      y++;
    }
    else
    {
      x=0;
      y=0;
    }     
#else
    if (y<12)
    {
      y++;
    }
    else if (x<9)
    {
      y=0;
      x++;
    }
    else
    {
      x=0;
      y=0;
    }  
#endif
     
  }
  while(*s!=0);

  Set_Cs;

}

/****************************************************************
��������Lcdд1��32X32���ĺ���
��ڲ�����x,�������꣬�����ҷֱ���0~7
          y,�������꣬���ϵ��·ֱ�Ϊ0~9
          CharColaor,�ַ�����ɫ 
          CharBackColor,�ַ�������ɫ 
         ASCIICode,��Ӧ���ĵı���
Ҳ����˵��320240�ֱ��ʵ���ʾ������������ʾ7�����ַ�����������ʾ10��
����ֵ����

ע�⣡���������������ʹ�ô˺�����Ӧ�ü���Lcd_Rs_H()��Set_Cs;Ϊ���Ż�ϵͳʡȥ��
���ָ�����˺���ִ�е���һ�������д�����RS_L�������д�뽫����
����ΪILI9320��Ϊ��RS_Lʱд���������
*****************************************************************/
void Lcd_Write32X32Chinese(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,u16 CharBackColor,u16 ChineseCode)
{
        
  u8 ByteCounter,BitCounter;
  u8 *ChinesePointer;
  u8 ChineseBuffer[128];	//ÿ������ռ��128���ֽ�
  
  Lcd_SetBox(x*32,y*32,32,32,x_offset,y_offset);         
  Lcd_WR_Start();
  GetSpiChineseCode(ChineseBuffer,ChineseCode,Chinese_Offset);
  ChinesePointer=ChineseBuffer;
  
  for(ByteCounter=0; ByteCounter<16; ByteCounter++)
  { 
    for(BitCounter=0; BitCounter<8; BitCounter++)
    {
      if (((*ChinesePointer)&(0x80 >> BitCounter)) == 0x00)
      {
        //Set_Rs;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr; 
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }
    }


    for (BitCounter=0; BitCounter<8; BitCounter++)
    {
      if (((*(ChinesePointer+1)) & (0x80 >> BitCounter)) == 0x00)
      {
        //Set_Rs;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr; 
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }
    }          
    for (BitCounter=0; BitCounter<8; BitCounter++)
    {
      if (((*ChinesePointer) & (0x80 >> BitCounter)) == 0x00)
      {
        //Set_Rs;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr; 
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }
    }        
    for (BitCounter=0; BitCounter<8; BitCounter++)
    {
      if (((*(ChinesePointer+1)) & (0x80 >> BitCounter)) == 0x00)
      {
        //Set_Rs;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharBackColor);
        Clr_nWr;
        Set_nWr; 
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }
    }
	  
    ChinesePointer+=2;
  }
  //Set_Cs;
 }
void Lcd_Write32X32ChineseClarity(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,u16 ChineseCode)
{
        
  GPIO_InitTypeDef GPIO_InitStructure;
  u8 ByteCounter,BitCounter;
  u8 *ChinesePointer;
  u8 ChineseBuffer[32];
  u16 Temp;

  Lcd_SetBox(x*32,y*32,32,32,x_offset,y_offset);         
  Lcd_WR_Start();
  GetSpiChineseCode(ChineseBuffer,ChineseCode,Chinese_Offset);
  ChinesePointer=ChineseBuffer;       
  
  for(ByteCounter=0; ByteCounter<16; ByteCounter++)
  { 
    for(BitCounter=0; BitCounter<8; BitCounter++)
    {
      if (((*ChinesePointer)&(0x80 >> BitCounter)) == 0x00)
      {
        //Set_Rs;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr;
                     
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr; 
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }
    }
          
    for(BitCounter=0; BitCounter<8; BitCounter++)
    {
      if (((*(ChinesePointer+1))&(0x80 >> BitCounter)) == 0x00)
      {
        //Set_Rs;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr;
                     
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr; 
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }
    }

          
          
    for(BitCounter=0; BitCounter<8; BitCounter++)
    {
      if (((*ChinesePointer)&(0x80 >> BitCounter)) == 0x00)
      {
        //Set_Rs;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr;
                     
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr; 
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }
    }
          
    for(BitCounter=0; BitCounter<8; BitCounter++)
    {
      if (((*(ChinesePointer+1))&(0x80 >> BitCounter)) == 0x00)
      {
        //Set_Rs;
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr;
                     
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        Clr_nRd;
        Set_nRd;
               
        Temp=GPIO_ReadInputData(GPIOE);
               
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
               
        DataToWrite(Temp);
        Clr_nWr;
        Set_nWr; 
      }
      else
      {
        //Set_Rs;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr;
        DataToWrite(CharColor);
        Clr_nWr;
        Set_nWr; 
      }
    }
      ChinesePointer+=2;
  }
  //Set_Cs;
  
}
void Lcd_WriteChineseString(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,u16 CharBackColor,char *s)
{
   
  u8 databuffer;
  u16 ChineseCode;
  Set_Rs;
  
  do
  {
    databuffer=*s++;
    ChineseCode=databuffer<<8;
    ChineseCode=ChineseCode|*s++;
    Lcd_WriteChinese(x,y,x_offset,y_offset,CharColor,CharBackColor,ChineseCode);
    
#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)  
    if (x<14)
    {
      x++;
    }
    else if (y<19)
    {
      x=0;
      y++;
    }
    else
    {
      x=0;
      y=0;
    }     
#else
    if (y<19)
    {
      y++;
    }
    else if (x<14)
    {
      y=0;
      x++;
    }
    else
    {
      x=0;
      y=0;
    }  
#endif
      
  }
  while(*s!=0);
    
  Set_Cs;

}
void Lcd_WriteChineseStringClarity(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,char *s)
{
   u8 databuffer;
   u16 ChineseCode;
   Set_Rs;
    do
   {
       databuffer=*s++;
       ChineseCode=databuffer<<8;
       ChineseCode=ChineseCode|*s++;
       Lcd_WriteChineseClarity(x,y,x_offset,y_offset,CharColor,ChineseCode);
       
#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)  
       if (x<14)
       {
         x++;
       }
       else if (y<19)
       {
         x=0;
         y++;
       }
       else
       {
         x=0;
         y=0;
       }     
#else
       if (y<19)
       {
         y++;
       }
       else if (x<14)
       {
         y=0;
         x++;
       }
       else
       {
         x=0;
         y=0;
       }  
#endif
       
   }
     while(*s!=0);
    Set_Cs;
}
void Lcd_Write32X32ChineseString(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,u16 CharBackColor,char *s)
{
   
  u8 databuffer;
  u16 ChineseCode;
   
  Set_Rs;
  do
  {
    databuffer=*s++;
    ChineseCode=databuffer<<8;
    ChineseCode=ChineseCode|*s++;
    Lcd_Write32X32Chinese(x,y,x_offset,y_offset,CharColor,CharBackColor,ChineseCode);
       
#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)  
    if (x<6)
    {
      x++;
    }
    else if (y<9)
    {
      x=0;
      y++;
    }
    else
    {
      x=0;
      y=0;
    }     
#else
    if (y<9)
    {
      y++;
    }
    else if (x<6)
    {
      y=0;
      x++;
    }
    else
    {
      x=0;
      y=0;
    }  
#endif
     
  }
  while(*s!=0);

  Set_Cs;

}
void Lcd_Write32X32ChineseStringClarity(u8 x,u8 y,u8 x_offset,u8 y_offset,u16 CharColor,char *s)
{
		u8 databuffer;
		u16 ChineseCode;
		Set_Rs;

		do
		{
			databuffer=*s++;
			ChineseCode=databuffer<<8;
			ChineseCode=ChineseCode|*s++;
			Lcd_Write32X32ChineseClarity(x,y,x_offset,y_offset,CharColor,ChineseCode);
    
#if (ID_AM==000)|(ID_AM==010)|(ID_AM==100)|(ID_AM==110)  

		if (x<6)
		{
			x++;
		}
		else if (y<9)
		{
			x=0;
			y++;
		}
		else
		{
			x=0;
			y=0;
		}     
#else
		if (y<9)
		{
			y++;
		}
		else if (x<6)
		{
			y=0;
			x++;
		}
			else
		{
			x=0;
			y=0;
		}  
#endif
    
	}
	while (*s!=0);
	
	Set_Cs;
}
/*********************************************************
���ǻ�ͼ����
**********************************************************/
void LcdWritePictureFromSPI(u8 xStart,u16 yStart,u8 xLong,u16 yLong,u32 BaseAddr)
{
	ColorTypeDef color;
	u32 pixels;
	
	Lcd_SetBox(xStart,yStart,xLong,yLong,0,0);
	Lcd_WR_Start();
	Set_Rs;
	
//	SPI_FLASH_CS_LOW();
//	SPI_FLASH_SendByte(0x0B);//FSTREAD
//	SPI_FLASH_SendByte((BaseAddr & 0xFF0000) >> 16);
//	SPI_FLASH_SendByte((BaseAddr& 0xFF00) >> 8);
//	SPI_FLASH_SendByte(BaseAddr & 0xFF);
//	SPI_FLASH_SendByte(0);//Dummy_Byte
	
	SPI2->DR = 0;//Dummy_Byte
	while((SPI2->SR & SPI_I2S_FLAG_RXNE) == (u16) RESET);
	color.U8[1] = SPI2->DR;	
	
	SPI2->DR = 0;//Dummy_Byte
	
	for (pixels=0; pixels<(xLong*yLong); pixels++)
	{

		while((SPI2->SR & SPI_I2S_FLAG_RXNE) == (u16) RESET);
		color.U8[0] = SPI2->DR;
		
		SPI2->DR = 0;//Dummy_Byte
		
		DataToWrite(color.U16);
		Clr_nWr;
		Set_nWr;
		
		while((SPI2->SR & SPI_I2S_FLAG_RXNE) == (u16) RESET);
		color.U8[1] = SPI2->DR;
		
		SPI2->DR = 0;//Dummy_Byte
	}

//	SPI_FLASH_CS_HIGH();
	Set_Cs;  
}
/*********************************************************
��������SPIȡASCII���ӳ���
���������u8 ASCII �����ASCII�룬��'A'
          BaseAddr ��ַ ��ASCII��ʾ������FLASH�е���ʾλ��
����ֵ����
˵��������һ��ASCII�룬ȡ������SPI FLASH�е�16Byte��ʾ����
�������ŵ�һ��16byte��ASCII��ʾ����CharBuffer[]��
**********************************************************/
void GetASCIICode(u8* pBuffer,u8 ASCII,u32 BaseAddr)
{  u8 i;
   for(i=0;i<16;i++)
    *(pBuffer+i)=*(ASCII_Table + (ASCII - 32)*16 + i);//    OffSet = (*pAscii - 32)*16;

 // SPI_FLASH_BufferRead(pBuffer,BaseAddr+16*ASCII,16);

}
/*********************************************************
��������SPI������ʾ���ӳ���
���������u16 ASCII ��������ģ���"��"
          BaseAddr ���� ����ʾ������FLASH�е���ʼλ��
����ֵ����
˵��������һ�����ģ�ȡ������SPI FLASH�е�32Byte��ʾ����
�������ŵ�һ��32byte����ʾ����ChineseBuffer[]
**********************************************************/
/*
void GetChineseCode(u8* pBuffer,u16 ChineseCode,u32 BaseAddr)
{  

//BaseAddr�Ǻ��ֿ���FLASH�еľ��Ե�ַ��ʹ��SD���ϵ��ļ�ʱΪ0
  u8 High8bit,Low8bit,i;
  u16 temp;
  u32 abs_addr,len;
  
  temp = ChineseCode - HzkBaseAddr;//���������λ�����ʼ����,���ֿ��ļ��й�
//  temp=ChineseCode;
  High8bit=(temp>>8);
  Low8bit=(temp&0x00FF);
  abs_addr = (u32)(BaseAddr + BufferSize*((High8bit)*94 + Low8bit)); //94��ÿ�������������ĺ�����Ŀ����������ǹ̶��ġ�
  f_lseek(&fil, abs_addr); 
//  TurnToSD();
  res = f_read(&fil, pBuffer, BufferSize, &len);  //BufferSizeΪÿ������ռ�õ��ֽ���
												  //��ILI932X.H�ļ��ﶨ��
//  for(i=0;i<32;i++)
//     	*(pBuffer+i)= *(HzLib + 32*((High8bit-0xb0)*94+Low8bit-0xa1) + i);
//  SPI_FLASH_BufferRead(pBuffer,BaseAddr+32*((High8bit-1)*94+Low8bit-1),32);

}

*/
/*********************************************************
��������SPI������ʾ���ӳ���
���������u16 ASCII ��������ģ���"��"
          BaseAddr ���� ����ʾ������FLASH�е���ʼλ��
����ֵ����
˵��������һ�����ģ�ȡ������SPI FLASH�е�32Byte��ʾ����
�������ŵ�һ��32byte����ʾ����ChineseBuffer[]
**********************************************************/
void GetSpiChineseCode(u8* pBuffer,u16 ChineseCode,u32 BaseAddr)
{  

//BaseAddr�Ǻ��ֿ���FLASH�еľ��Ե�ַ��ʹ��SD���ϵ��ļ�ʱΪ0
  u8 High8bit,Low8bit,i;
  u16 temp;
  u32 abs_addr,len;
  
  temp = ChineseCode - HzkBaseAddr;//���������λ�����ʼ����,���ֿ��ļ��й�
//  temp=ChineseCode;
  High8bit=(temp>>8);
  Low8bit=(temp&0x00FF);
  abs_addr = (u32)(BaseAddr + BufferSize*((High8bit)*94 + Low8bit)); //94��ÿ�������������ĺ�����Ŀ����������ǹ̶��ġ�

  SPI_FLASH_ABS_READByte(pBuffer, abs_addr, BufferSize);

}

void Get320240PictureCode(u8* pBuffer,u32 BufferCounter,u32 BaseAddr)
{
 //   SPI_FLASH_BufferRead(pBuffer,BaseAddr+BufferCounter*32,32);
}


void DrawPixel(u8 x, u8 y, int Color)
{
	Lcd_SetCursor(x,y);
    Lcd_WR_Start(); 
	Set_Rs;
    DataToWrite(Color);
	Clr_nWr;
	Set_nWr;
	Set_Cs;
}
void DispPic240_320(const unsigned char *str)
{

	u32 temp;
	ColorTypeDef color;
	Lcd_SetCursor(0x00, 0x0000);
	LCD_WR_REG(0x0050,0x00);//ˮƽ GRAM��ʼλ��
	LCD_WR_REG(0x0051,239);//ˮƽGRAM��ֹλ��
	LCD_WR_REG(0x0052,0);//��ֱGRAM��ʼλ��
	LCD_WR_REG(0x0053,319);//��ֱGRAM��ֹλ��   
	Lcd_WR_Start();
	Set_Rs;
  
	for (temp = 0; temp < 240*320; temp++)
	{  
		color.U8[1] =*(unsigned short *)(&str[ 2 * temp]);
		color.U8[0]=*(unsigned short *)(&str[ 2 * temp+1]);
		//DataToWrite(i);
	
		DataToWrite(color.U16);
		Clr_nWr;
		Set_nWr;
	} 
}


//==============================  
void Test_Color(void){
  u8  R_data,G_data,B_data,i,j;

	LCD_SetCursor(0x00, 0x0000);
	LCD_WR_REG(0x0050,0x00);//ˮƽ GRAM��ʼλ��
	LCD_WR_REG(0x0051,239);//ˮƽGRAM��ֹλ��
	LCD_WR_REG(0x0052,0);//��ֱGRAM��ʼλ��
	LCD_WR_REG(0x0053,319);//��ֱGRAM��ֹλ��   
	Lcd_WR_Start();
	Set_Rs;
    R_data=0;G_data=0;B_data=0;     
    for(j=0;j<50;j++)//��ɫ��ǿ��
    {
        for(i=0;i<240;i++)
            {R_data=i/8;DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;Set_nWr;}
    }
    R_data=0x1f;G_data=0x3f;B_data=0x1f;
    for(j=0;j<50;j++)
    {
        for(i=0;i<240;i++)
            {
            G_data=0x3f-(i/4);
            B_data=0x1f-(i/8);
            DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;
			Set_nWr;
			}
    }
//----------------------------------
    R_data=0;G_data=0;B_data=0;
    for(j=0;j<50;j++)//��ɫ��ǿ��
    {
        for(i=0;i<240;i++)
            {G_data=i/4;
			DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;
			Set_nWr;}
    }

    R_data=0x1f;G_data=0x3f;B_data=0x1f;
    for(j=0;j<50;j++)
    {
        for(i=0;i<240;i++)
            {
            R_data=0x1f-(i/8);
            B_data=0x1f-(i/8);
            DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;
			Set_nWr;
		}
    }
//----------------------------------
 
    R_data=0;G_data=0;B_data=0;
    for(j=0;j<60;j++)//��ɫ��ǿ��
    {
        for(i=0;i<240;i++)
            {B_data=i/8;DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;
			Set_nWr;}
    } 

    B_data=0; 
    R_data=0x1f;G_data=0x3f;B_data=0x1f;

    for(j=0;j<60;j++)
    {
        for(i=0;i<240;i++)
            {
            G_data=0x3f-(i/4);
            R_data=0x1f-(i/8);
            DataToWrite(R_data<<11|G_data<<5|B_data);
			Clr_nWr;
			Set_nWr;
		}
    }	  
	Set_Cs;
}

//====================================================================================
/*
void DrawSingleAscii(u16 x, u16 y, u8 *pAscii, u16 LineColor,u16 FillColor, u8 Mod)
{
    u8 i, j;
    u8 str;
    u16 OffSet;

	Lcd_SetCursor(0x00, 0x0000);
	LCD_WR_REG(0x0050,0x00);//ˮƽ GRAM��ʼλ��
	LCD_WR_REG(0x0051,239);//ˮƽGRAM��ֹλ��
	LCD_WR_REG(0x0052,0);//��ֱGRAM��ʼλ��
	LCD_WR_REG(0x0053,319);//��ֱGRAM��ֹλ�� 
	Lcd_WR_Start();
	Set_Rs;

    OffSet = (*pAscii - 32)*16;
    for (i=0;i<16;i++)
    {
        Lcd_SetCursor(x,y+i);
        Lcd_WR_Start();
		Set_Rs;

        str = *(AsciiLib + OffSet + i);  
        for (j=0;j<8;j++)
        {
            if ( str & (0x80>>j) )     //0x80>>j
            {
                DataToWrite((u16)(LineColor&0xffff));
				Clr_nWr;
				Set_nWr;
            }
            else
            {
                if (NORMAL == Mod) 
                    DataToWrite((u16)(FillColor&0xffff));
					Clr_nWr;
					Set_nWr;
                else
                {
                    Lcd_SetCursor(x+j+1,y+i);
                    Lcd_WR_Start();
					Set_Rs; 
                }
            }               
        } 
    }
}
*/



/****************************************************************************
* ��    �ƣ�void DataToWrite(u16 data)
* ��    �ܣ�����������ĺ�����ʹ��һ��ͨ�ÿڵĵ�8λ��
* ��ڲ�������
* ���ڲ�������
* ˵    �������ú������ֱ���͵�TFT��GRAM�Դ档
* ���÷�������
****************************************************************************/
#ifdef LCD_16B_IO 
void DataToWrite(u16 data) 	   //���ĺ�����16λͨѶ��ʽ
{
	u16 temp;
	temp = GPIO_ReadOutputData(GPIOD);
	GPIO_Write(GPIOD, (data&0x00ff)|(temp&0xff00));	  //��8λ����
	temp = GPIO_ReadOutputData(GPIOE);
	GPIO_Write(GPIOE, (data>>8)|(temp&0xff00));	  //��8λ����
}
#elif defined LCD_8B_IO		   //ע�ⲻҪ��PB8-PB15������TFT�ĸ�8λ�������ͻ
void DataToWrite(u16 data) 	   //���ĺ�����8λͨѶ��ʽ
{
	u16 temp;
	u16 half1 = data & 0x00ff;	 //��8λ����
	u16 half2 = data >>8; 		 //��8λ����
	temp = GPIO_ReadOutputData(LCD_DATA_PORT);
	Set_LE;		  //LE = 1,��λ������

//#ifdef LGDP4531
//	GPIO_Write(LCD_DATA_PORT, half2 |(temp&0xff00));   //���͸߰�λ
//	Clr_nWr;
//	Set_nWr;
									          
//#else
	GPIO_Write(LCD_DATA_PORT, half1 |(temp&0xff00));  // ���͵�8λ
	Clr_LE;		                              //	LE = 0����λ����
//#endif
//#ifdef LGDP4531
//	GPIO_Write(LCD_DATA_PORT, half1 |(temp&0xff00));   //���͵�8λ
											           //��������	
//#else
	GPIO_Write(LCD_DATA_PORT, half2 |(temp&0xff00));   //���͸߰�λ
//#endif
   
}
#endif


/****************************************************************************
* ��    �ƣ�DataToWrite(u16 data)
* ��    �ܣ�����������ĺ�����ʹ��һ��ͨ�ÿڵĸ�8λ��
* ��ڲ�������
* ���ڲ�������
* ˵    �������ú������ֱ���͵�TFT��GRAM�Դ档
* ���÷�������
****************************************************************************/
/*
#ifdef LCD_16B_IO 
void DataToWrite(u16 data) 	   //���ĺ�����16λͨѶ��ʽ
{
  	GPIOE->ODR = data;
}
#elif defined LCD_8B_IO		   //ע�ⲻҪ��PB8-PB15������TFT�ĸ�8λ�������ͻ
void DataToWrite(u16 data) 	   //���ĺ�����8λͨѶ��ʽ
{
	u16 temp;
	u16 half2 = data & 0xff00;	 //��8λ����
	u16 half1 = data << 8; 		 //��8λ����
	temp = GPIO_ReadOutputData(GPIOE);
	Set_LE;		  //LE = 1,��λ������
#ifdef LGDP4531
	GPIOE->ODR = half2 |(temp&0x00ff);   //���͸߰�λ
	Clr_nWr;
	Set_nWr;
									          
#else
	GPIOE->ODR = half1 |(temp&0x00ff);  // ���͵�8λ
	Clr_LE;		                              //	LE = 0����λ����
#endif
#ifdef LGDP4531
	GPIOE->ODR = half1 |(temp&0x00ff);   //���͵�8λ
											   //��������	
#else
	GPIOE->ODR = half2 |(temp&0x00ff);   //���͸߰�λ

#endif
   
}
#endif
*/


/******************************************
��������Lcdд�����
���ܣ���Lcdָ��λ��д��Ӧ�����������
��ڲ�����Index ҪѰַ�ļĴ�����ַ
          ConfigTemp д������ݻ�����ֵ
����ֵ����
******************************************/
void LCD_WR_REG(u16 Index,u16 CongfigTemp)
{
	Clr_Cs;
	Clr_Rs;
	Set_nRd;
	DataToWrite(Index);
	Clr_nWr;
	Set_nWr;
	Set_Rs;       
	DataToWrite(CongfigTemp);       
	Clr_nWr;
	Set_nWr;
	Set_Cs;
}


/************************************************
��������Lcdд��ʼ����
���ܣ�����Lcd�������� ִ��д���� ���൱��LCD_WriteRAM_Prepare()
��ڲ�������
����ֵ����
************************************************/
void Lcd_WR_Start(void)
{
	Clr_Cs;
	Clr_Rs;
	Set_nRd;
	DataToWrite(0x0022);
	Clr_nWr;
	Set_nWr;
	Set_Rs;
}

/****************************************************************************
* ��    �ƣ�void ili9320_Test()
* ��    �ܣ�����Һ����
* ��ڲ�������
* ���ڲ�������
* ˵    ������ʾ����������Һ�����Ƿ���������
* ���÷�����ili9320_Test();
****************************************************************************/
void ili9320_Test(void)
{
  u8  R_data,G_data,B_data,i,j;

	LCD_SetCursor(0x00, 0x0000);
	ili9320_WriteRegister(0x0050,0x00);//ˮƽ GRAM��ʼλ��
	ili9320_WriteRegister(0x0051,239);//ˮƽGRAM��ֹλ��
	ili9320_WriteRegister(0x0052,0);//��ֱGRAM��ʼλ��
	ili9320_WriteRegister(0x0053,319);//��ֱGRAM��ֹλ��  
	Clr_Cs; 
	ili9320_WriteIndex(0x0022);
    R_data=0;G_data=0;B_data=0;     
    for(j=0;j<50;j++)//��ɫ��ǿ��
    {
        for(i=0;i<240;i++)
            {R_data=i/8;ili9320_WriteData(R_data<<11|G_data<<5|B_data);
		}
    }
    R_data=0x1f;G_data=0x3f;B_data=0x1f;
    for(j=0;j<50;j++)
    {
        for(i=0;i<240;i++)
            {
            G_data=0x3f-(i/4);
            B_data=0x1f-(i/8);
            ili9320_WriteData(R_data<<11|G_data<<5|B_data);
			}
    }
//----------------------------------
    R_data=0;G_data=0;B_data=0;
    for(j=0;j<50;j++)//��ɫ��ǿ��
    {
        for(i=0;i<240;i++)
            {G_data=i/4;
			ili9320_WriteData(R_data<<11|G_data<<5|B_data);
}
    }

    R_data=0x1f;G_data=0x3f;B_data=0x1f;
    for(j=0;j<50;j++)
    {
        for(i=0;i<240;i++)
            {
            R_data=0x1f-(i/8);
            B_data=0x1f-(i/8);
            ili9320_WriteData(R_data<<11|G_data<<5|B_data);
		}
    }
//----------------------------------
 
    R_data=0;G_data=0;B_data=0;
    for(j=0;j<60;j++)//��ɫ��ǿ��
    {
        for(i=0;i<240;i++)
            {B_data=i/8;ili9320_WriteData(R_data<<11|G_data<<5|B_data);
			}
    } 

    B_data=0; 
    R_data=0x1f;G_data=0x3f;B_data=0x1f;

    for(j=0;j<60;j++)
    {
        for(i=0;i<240;i++)
            {
            G_data=0x3f-(i/4);
            R_data=0x1f-(i/8);
            ili9320_WriteData(R_data<<11|G_data<<5|B_data);
		}
    }	  
	Set_Cs;
}

/****************************************************************************
* ��    �ƣ�u16 ili9320_BGR2RGB(u16 c)
* ��    �ܣ�RRRRRGGGGGGBBBBB ��Ϊ BBBBBGGGGGGRRRRR ��ʽ
* ��ڲ�����c      BRG ��ɫֵ
* ���ڲ�����RGB ��ɫֵ
* ˵    �����ڲ���������
* ���÷�����
****************************************************************************/
u16 ili9320_BGR2RGB(u16 c)
{
  u16  r, g, b, rgb;

  b = (c>>0)  & 0x1f;
  g = (c>>5)  & 0x3f;
  r = (c>>11) & 0x1f;
  
  rgb =  (b<<11) + (g<<5) + (r<<0);

  return( rgb );
}

/****************************************************************************
* ��    �ƣ�void ili9320_WriteIndex(u16 idx)
* ��    �ܣ�д ili9320 �������Ĵ�����ַ
* ��ڲ�����idx   �Ĵ�����ַ
* ���ڲ�������
* ˵    ��������ǰ����ѡ�п��������ڲ�����
* ���÷�����ili9320_WriteIndex(0x0000);
****************************************************************************/
void ili9320_WriteIndex(u16 idx)
{
        Clr_Rs;
	Set_nRd;
	DataToWrite(idx);
	Clr_nWr;
	Set_nWr;
}

/****************************************************************************
* ��    �ƣ�void ili9320_WriteData(u16 dat)
* ��    �ܣ�д ili9320 �Ĵ�������
* ��ڲ�����dat     �Ĵ�������
* ���ڲ�������
* ˵    �����������ָ����ַд�����ݣ�����ǰ����д�Ĵ�����ַ���ڲ�����
* ���÷�����ili9320_WriteData(0x1030)
****************************************************************************/
void ili9320_WriteData(u16 data)
{
	Set_Rs;
	Set_nRd;
	DataToWrite(data);
	Clr_nWr;
	Set_nWr;
}

/****************************************************************************
* ��    �ƣ�u16 ili9320_ReadData(void)
* ��    �ܣ���ȡ����������
* ��ڲ�������
* ���ڲ��������ض�ȡ��������
* ˵    �����ڲ�����
* ���÷�����i=ili9320_ReadData();
****************************************************************************/
u16 ili9320_ReadData(void)
{
//========================================================================
// **                                                                    **
// ** nCS       ----\__________________________________________/-------  **
// ** RS        ------\____________/-----------------------------------  **
// ** nRD       -------------------------\_____/---------------------  **
// ** nWR       --------\_______/--------------------------------------  **
// ** DB[0:15]  ---------[index]----------[data]-----------------------  **
// **                                                                    **
//========================================================================
	unsigned short val = 0;
	Set_Rs;
	Set_nWr;
	Clr_nRd;
        GPIOE->CRH = 0x44444444;
	GPIOE->CRL = 0x44444444;
	val = GPIOE->IDR;
	val = GPIOE->IDR;
	GPIOE->CRH = 0x33333333;
	GPIOE->CRL = 0x33333333;
	Set_nRd;
	return val;
}

/****************************************************************************
* ��    �ƣ�u16 ili9320_ReadRegister(u16 index)
* ��    �ܣ���ȡָ����ַ�Ĵ�����ֵ
* ��ڲ�����index    �Ĵ�����ַ
* ���ڲ������Ĵ���ֵ
* ˵    �����ڲ�����
* ���÷�����i=ili9320_ReadRegister(0x0022);
****************************************************************************/
u16 ili9320_ReadRegister(u16 index)
{ 
  	Clr_Cs;
	ili9320_WriteIndex(index);     
	index = ili9320_ReadData();      	
	Set_Cs;
	return index;
}

/****************************************************************************
* ��    �ƣ�void ili9320_WriteRegister(u16 index,u16 dat)
* ��    �ܣ�дָ����ַ�Ĵ�����ֵ
* ��ڲ�����index    �Ĵ�����ַ
*         ��dat      �Ĵ���ֵ
* ���ڲ�������
* ˵    �����ڲ�����
* ���÷�����ili9320_WriteRegister(0x0000,0x0001);
****************************************************************************/
void ili9320_WriteRegister(u16 index,u16 dat)
{
 /************************************************************************
  **                                                                    **
  ** nCS       ----\__________________________________________/-------  **
  ** RS        ------\____________/-----------------------------------  **
  ** nRD       -------------------------------------------------------  **
  ** nWR       --------\_______/--------\_____/-----------------------  **
  ** DB[0:15]  ---------[index]----------[data]-----------------------  **
  **                                                                    **
  ************************************************************************/
        Clr_Cs;
	ili9320_WriteIndex(index);      
	ili9320_WriteData(dat);    
	Set_Cs; 
}


/****************************************************************************
* ��    �ƣ�void ili9320_Delay(vu32 nCount)
* ��    �ܣ���ʱ
* ��ڲ�����nCount   ��ʱֵ
* ���ڲ�������
* ˵    ����
* ���÷�����ili9320_Delay(10000);
****************************************************************************/
void ili9320_Delay(vu32 nCount)
{
  for(; nCount != 0; nCount--);
}

/****************************************************************************
* ��    �ƣ�void Delay_nms(vu32 n)
* ��    �ܣ���ʱ����MS
* ��ڲ�����n   ��ʱֵ
* ���ڲ�������
* ˵    ����
* ���÷�����Delay_nms(100);
****************************************************************************/

void Delay_nms(vu32 n)
{
  
  vu32 f=n,k;
  for (; f!=0; f--)
  {
    for(k=0xFFF; k!=0; k--);
  }
}


