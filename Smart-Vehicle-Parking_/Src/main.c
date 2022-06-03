#if !defined(__SOFT_FP__) && defined(__ARM_FP)
#warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif


/*===============Learn in depth================*/
/*===Second term project Smart Car Barking=====*/
/*Auther :Mostafa Mahmoud Elshiekh*/

#include "STM32_F103C6.h"
#include "STM32F103C6_GPIO_DRIVERS.h"
#include "LCD_ME.h"
#include "STM32F103C6_USART_DRIVERS.h"
#include "Servo_Motor.h"


/*=================Global Variables===============*/
uint16_t RFID_CARD_Enter;
uint16_t RFID_CARD_EXIT;

uint8_t pass[3] ={0};

unsigned char Available_Slots='3';
uint8_t Search_Done = 0;



void RFID_CARD_Enter_CallBack(void);
void RFID_CARD_EXIT_CallBack(void);

void LCD_Welcome(void);
void LCD_Display_Empty_Slots(void);
void LEDs_Init(void);
void PIRs_Iint(void);
void Store_IDs_From_Admin(void);



s_LCD_Config_t LCD={
		{ GPIOB,GPIO_PIN_11,GPIO_PIN_10},
		{ GPIOB, GPIO_PIN_12 },
		NOSHIFT_INCREMENT,
		ONDISP_BLINKCURSOR,
		FOURBIT_2LINE_7DOT,0
};

s_LCD_Config_t LCD2={
		{ GPIOA,GPIO_PIN_5,GPIO_PIN_6},
		{ GPIOA, GPIO_PIN_12 },
		NOSHIFT_INCREMENT,
		ONDISP_BLINKCURSOR,
		FOURBIT_2LINE_7DOT,0
};


int main(void)
{
	RCC_GPIOA_CLOCK_EN();
	RCC_GPIOB_CLOCK_EN();
	RCC_AFIO_CLOCK_EN();
	keypad_Init();
	Timer2_init();
	Servo1_Entry_Gate_Init();
	Servo2_Exit_Gate_Init();
	//==========================UART1 INIT=============================
	/* PA9  TX
	 * PA10 RX
	 * PA11 CTC
	 * PA12 RTS
	 */
	USART_Config uartCFG;

	uartCFG.BaudRate = USART_BaudRate_115200;
	uartCFG.HW_FlowCtrl = USART_HW_FlowCtrl_NONE;
	uartCFG.IRQ_Enable = USART_IRQ_Enable_RXNE;
	uartCFG.P_IRQ_Call_Back = RFID_CARD_Enter_CallBack;
	uartCFG.Parity = USART_Parity_NONE;
	uartCFG.PayLoad_Length = USART_PayLoad_Length_8B;
	uartCFG.StopBits = USART_StopBits_1;
	uartCFG.USART_MODE = USART_MODE_RX_AND_TX;
	MCAL_UART_INIT(USART1, &uartCFG);
	MCAL_UART_GPIO_Set_Pins(USART1);


	//==========================UART2 INIT=============================
	/* PA2  ->TX
	 * PA3 ->RX
	 * PA0 ->CTS
	 * PA1 ->RTS
	 */
	USART_Config uart2CFG;

	uart2CFG.BaudRate = USART_BaudRate_115200;
	uart2CFG.HW_FlowCtrl = USART_HW_FlowCtrl_NONE;
	uart2CFG.IRQ_Enable = USART_IRQ_Enable_RXNE;
	uart2CFG.P_IRQ_Call_Back = RFID_CARD_EXIT_CallBack;
	uart2CFG.Parity = USART_Parity_NONE;
	uart2CFG.PayLoad_Length = USART_PayLoad_Length_8B;
	uart2CFG.StopBits = USART_StopBits_1;
	uart2CFG.USART_MODE = USART_MODE_RX_AND_TX;
	MCAL_UART_INIT(USART2, &uart2CFG);
	MCAL_UART_GPIO_Set_Pins(USART2);

	LCD_init(&LCD);
	LCD_init(&LCD2);
	LEDs_Init();
	PIRs_Iint();

	/*==========Enter IDs To Store In Array Pass==========*/
	 Store_IDs_From_Admin();

	/*Dash Bord LCD*/
	LCD_Welcome();
	dms(200);
	LCD_Display_Empty_Slots();


	int i=0;
	while(1)
	{
		if(Available_Slots>'0')
		{
			LCD_gotoxy(&LCD2, -4, 2);
			LCD_sendstring(&LCD2,"Put Your Id In");
			LCD_gotoxy(&LCD2,-4,3 );
			LCD_sendstring(&LCD2,"Card Reader");
			while(RFID_CARD_Enter ==0 && RFID_CARD_EXIT==0); //Wait for Enter any id
		}
		/*======Parking is full so not allow enetr cars=======*/
		else
		{
			LCD_sendchar(&LCD2, 0x0c, COMMAND);
			LCD_sendchar(&LCD2, 0x01, COMMAND);
			LCD_gotoxy(&LCD2, 0, 0);
			LCD_sendstring(&LCD2,"Parking Is Full");
			LCD_gotoxy(&LCD2, 0, 1);
			LCD_sendstring(&LCD2,"Entry Not Allowed");
		}

		//Search about ID in array of Pass
		for(i=0;i<3;i++)
		{
			if(RFID_CARD_Enter == pass[i] || RFID_CARD_EXIT == pass[i] )
			{
				Search_Done=1;
			}

		}
		/*=====================Enter gate=====================*/
		if(Search_Done == 1 && RFID_CARD_Enter !=0 && Available_Slots>'0')
		{
			LCD_sendchar(&LCD2, 0x0c, COMMAND);
			LCD_sendchar(&LCD2, 0x01, COMMAND);
			LCD_gotoxy(&LCD2, 0, 0);
			LCD_sendstring(&LCD2,"ID Is Right");
			LCD_gotoxy(&LCD2, 0, 1);
			LCD_sendstring(&LCD2,"Entry gate Opens");

			//Green led*/
			MCAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 0);
			Servo1_Entry_Gate(UP);
			Available_Slots--;

			RFID_CARD_Enter =0;
			Search_Done =0;

			dms(300);
			//wait for car fully enter
			while(MCAL_GPIO_Read_Pin(GPIOA, GPIO_PIN_7)==1) //Read the PIR1 state
			{
				dms(200);
			}

			//Green led of
			MCAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 1);
			Servo1_Entry_Gate(Down);

			/*Dash Bord LCD*/
			LCD_Welcome();
			dms(200);
			LCD_Display_Empty_Slots();

		}
		/*===================Exit gate====================*/
		if(Search_Done == 1 && RFID_CARD_EXIT !=0 && Available_Slots<'3')
		{
			LCD_sendchar(&LCD2, 0x0c, COMMAND);
			LCD_sendchar(&LCD2, 0x01, COMMAND);
			LCD_gotoxy(&LCD2, 0, 0);
			LCD_sendstring(&LCD2,"ID Is Right");
			LCD_gotoxy(&LCD2, 0, 1);
			LCD_sendstring(&LCD2,"Exit Gate Opens");

			//Green led*/
			MCAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 0);
			Servo2_Exit_Gate(UP);
			Available_Slots++;
			dms(300);
			//wait for car fully enter
			while(MCAL_GPIO_Read_Pin(GPIOA, GPIO_PIN_1)==1) //Read the PIR2 state
			{
				dms(200);
			}
			//Green led of
			MCAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 1);
			Servo2_Exit_Gate(Down);

			RFID_CARD_EXIT =0;
			Search_Done =0;

			/*Dash Bord LCD*/
			LCD_Welcome();
			dms(200);
			LCD_Display_Empty_Slots();
		}

		/*==================Wrong Id====================*/
		if(Search_Done == 0 && (RFID_CARD_Enter >0 || RFID_CARD_EXIT>0) )
		{
			LCD_sendchar(&LCD2, 0x0c, COMMAND);
			LCD_sendchar(&LCD2, 0x01, COMMAND);
			LCD_gotoxy(&LCD2, 0, 0);
			LCD_sendstring(&LCD2,"Wrong ID");
			LCD_gotoxy(&LCD2, 0, 1);
			LCD_sendstring(&LCD2,"Try Again");

			for(i=0;i<3;i++)
			{
				//Red led ON as alarm
				MCAL_GPIO_Toggle_pin(GPIOA, GPIO_PIN_0);
				dms(150);
			}
			MCAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, 1); //to ensure Red Led off after toggle

			/*Dash Bord LCD*/
			LCD_Welcome();
			dms(150);
			LCD_Display_Empty_Slots();
			RFID_CARD_EXIT =0;
			RFID_CARD_Enter =0;
		}


	}

	return 0;
}


/*Call Back For UART*/
void RFID_CARD_Enter_CallBack(void)
{
	MCAL_UART_RECEIVE_DATA(USART1, &RFID_CARD_Enter, disable);
	MCAL_UART_SEND_DATA(USART1, &RFID_CARD_Enter, enable);
}

void RFID_CARD_EXIT_CallBack(void)
{
	MCAL_UART_RECEIVE_DATA(USART2, &RFID_CARD_EXIT, disable);
	MCAL_UART_SEND_DATA(USART2, &RFID_CARD_EXIT, enable);
}



void LCD_Welcome(void)
{
	/*Dash Bord LCD*/

	LCD_sendchar(&LCD2, 0x0c, COMMAND);
	LCD_sendchar(&LCD2, 0x01, COMMAND);
	LCD_gotoxy(&LCD2, 0, 0);
	LCD_sendstring(&LCD2,"Welcome in");
	LCD_gotoxy(&LCD2,0,1 );
	LCD_sendstring(&LCD2,"Mostafa Parking");
}

void LCD_Display_Empty_Slots(void)
{
	LCD_sendchar(&LCD2, 0x0c, COMMAND);
	LCD_sendchar(&LCD2, 0x01, COMMAND);
	LCD_gotoxy(&LCD2, 0, 0);
	LCD_sendstring(&LCD2,"Empty Slots=");
	LCD_gotoxy(&LCD2, 13, 0);
	LCD_sendchar(&LCD2,Available_Slots, DATA);
}

void LEDs_Init(void)
{
	//RED LED A0
	GPIO_Pin_Configure_t PinCinfg;
	PinCinfg.GPIO_Pin_Number=GPIO_PIN_0;
	PinCinfg.GPIO_MODE = GPIO_MODE_OUTPUT_PP;
	PinCinfg.GPIO_OUTPUT_SPEED =GPIO_SPEED_10M;
	MCAL_GPIO_Init(GPIOA, &PinCinfg);

	//GREEN LED A11
	PinCinfg.GPIO_Pin_Number=GPIO_PIN_11;
	PinCinfg.GPIO_MODE = GPIO_MODE_OUTPUT_PP;
	PinCinfg.GPIO_OUTPUT_SPEED =GPIO_SPEED_10M;
	MCAL_GPIO_Init(GPIOA, &PinCinfg);
}

void PIRs_Iint(void)
{
	//PIR1 A7
	GPIO_Pin_Configure_t PinCinfg;
	PinCinfg.GPIO_Pin_Number=GPIO_PIN_7;
	PinCinfg.GPIO_MODE = GPIO_MODE_INPUT_PU;
	MCAL_GPIO_Init(GPIOA, &PinCinfg);

	//PIR2 A1
	PinCinfg.GPIO_Pin_Number=GPIO_PIN_1;
	PinCinfg.GPIO_MODE = GPIO_MODE_INPUT_PU;
	MCAL_GPIO_Init(GPIOA, &PinCinfg);

	MCAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, 1); //make A0 ground
	MCAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 1); //make A11 ground

}

void Store_IDs_From_Admin(void)
{
	LCD_sendchar(&LCD, 0x0c, COMMAND);
		LCD_gotoxy(&LCD, 0, 0);
		LCD_sendstring(&LCD,"Enter Allowed ID");
		/*First ID*/
		LCD_gotoxy(&LCD, 4, 1);
		LCD_sendstring(&LCD,"First  ID:");
		while(keypad_Read()=='N');
		pass[0]=keypad_Read();
		while(keypad_Read()!='N');
		LCD_gotoxy(&LCD,14,1 );
		LCD_sendchar(&LCD, pass[0], DATA);
		LCD_sendchar(&LCD, 0x0c, COMMAND);

		/*Second ID*/
		LCD_gotoxy(&LCD, 0, 2);
		LCD_sendstring(&LCD,"Second ID:");
		while(keypad_Read()=='N');
		pass[1]=keypad_Read();
		while(keypad_Read()!='N');
		LCD_gotoxy(&LCD, 10,2);
		LCD_sendchar(&LCD, pass[1], DATA);
		LCD_sendchar(&LCD, 0x0c, COMMAND);

		/*Therd ID*/
		LCD_gotoxy(&LCD, 0, 3);
		LCD_sendstring(&LCD,"Therd  ID:");
		while(keypad_Read()=='N');
		pass[2]=keypad_Read();
		while(keypad_Read()!='N');
		LCD_gotoxy(&LCD, 10,3 );
		LCD_sendchar(&LCD, pass[2], DATA);
		LCD_sendchar(&LCD, 0x0c, COMMAND);

		/*Saved*/
		LCD_clearscreen(&LCD);
		LCD_gotoxy(&LCD, 0, 0);
		LCD_sendstring(&LCD,"IDs are Saved");
		LCD_sendchar(&LCD, 0x0c, COMMAND);

		LCD_gotoxy(&LCD, 0, 1);
		LCD_sendchar(&LCD, pass[0], DATA);
		LCD_gotoxy(&LCD, 2, 1);
		LCD_sendchar(&LCD, pass[1], DATA);
		LCD_gotoxy(&LCD, 4, 1);
		LCD_sendchar(&LCD, pass[2], DATA);
}
