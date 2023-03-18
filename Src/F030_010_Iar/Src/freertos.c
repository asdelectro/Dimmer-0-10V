/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "1306_32.h"
#include <string.h>
#include <stdio.h>
#include "flash_setings.h"
#include <stdlib.h>
#include <stdbool.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
osMessageQId RecQHandle;


#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t b1;
uint8_t b2;
char g_buffer[257];
uint8_t data_i = 0;

typedef struct {
	uint8_t usart;
	uint8_t byte;
} myMes;

portBASE_TYPE           xHigherPriorityTaskWoken = pdFALSE;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{ 
        myMes m;
        m.usart=1;
        m.byte=b1;
        xQueueSendFromISR( RecQHandle, &m, &xHigherPriorityTaskWoken ); 
        HAL_UART_Receive_IT(&huart1, &b1,1); 
				
}

bool boottonPress=false;
bool yesFlag=true;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint16_t capture=0;
uint8_t r202_out[8]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
extern ESettings Settings;
 
/* USER CODE END Variables */
osThreadId Encoder_changeHandle;
osThreadId Uart_changeHandle;
osThreadId MonitorHandle;
osThreadId AT_handlerHandle;
osThreadId KlikKlackHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
   
/* USER CODE END FunctionPrototypes */

void Task_1_Enc(void const * argument);
void Task_2_uart(void const * argument);
void Task_3_monitor(void const * argument);
void AT_HANDLER(void const * argument);
void KLIK_KLACK(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
   osMessageQDef(RecQ, 32, 2);
   RecQHandle = osMessageCreate(osMessageQ(RecQ), NULL);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Encoder_change */
  osThreadDef(Encoder_change, Task_1_Enc, osPriorityNormal, 0, 128);
  Encoder_changeHandle = osThreadCreate(osThread(Encoder_change), NULL);

  /* definition and creation of Uart_change */
  osThreadDef(Uart_change, Task_2_uart, osPriorityIdle, 0, 128);
  Uart_changeHandle = osThreadCreate(osThread(Uart_change), NULL);

  /* definition and creation of Monitor */
  osThreadDef(Monitor, Task_3_monitor, osPriorityIdle, 0, 128);
  MonitorHandle = osThreadCreate(osThread(Monitor), NULL);

  /* definition and creation of AT_handler */
  osThreadDef(AT_handler, AT_HANDLER, osPriorityIdle, 0, 128);
  AT_handlerHandle = osThreadCreate(osThread(AT_handler), NULL);

  /* definition and creation of KlikKlack */
  osThreadDef(KlikKlack, KLIK_KLACK, osPriorityIdle, 0, 128);
  KlikKlackHandle = osThreadCreate(osThread(KlikKlack), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_Task_1_Enc */
/**
  * @brief  Function implementing the Encoder_change thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_Task_1_Enc */
uint16_t captureOld=0;
uint16_t test_volt=0;
void Task_1_Enc(void const * argument)
{
  /* USER CODE BEGIN Task_1_Enc */
HAL_TIM_PWM_Start_IT(&htim14, TIM_CHANNEL_1);
HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
if(CheckEEprom()) restoreSettings();
TIM3->CNT=Settings.DefPrest.AdrData.data/Settings.Speed.AdrData.data;
  /* Infinite loop */
  for(;;)
  {
   capture=TIM3->CNT;
   
 if(captureOld!=capture)
 {
   if(capture>=2999)
   {
   TIM3->CNT=0;
   capture=0;
   }
   else if(capture* Settings.Speed.AdrData.data>Settings.Scale.AdrData.data)
   {
    TIM3->CNT=Settings.Scale.AdrData.data/Settings.Speed.AdrData.data;
   }
   else
   {
   capture=TIM3->CNT* Settings.Speed.AdrData.data;
   TIM14->CCR1= capture;
   test_volt=capture;
   }
 }  
   captureOld=TIM3->CNT;
 
  // if(capture>Settings.Scale.AdrData.data){TIM3->CNT=0;capture=0;}
   
   osDelay(1);
  }
  /* USER CODE END Task_1_Enc */
}

/* USER CODE BEGIN Header_Task_2_uart */
/**
* @brief Function implementing the Uart_change thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_2_uart */
void Task_2_uart(void const * argument)
{
  /* USER CODE BEGIN Task_2_uart */
  /* Infinite loop */

  for(;;)
  {
  if(HAL_UART_Receive_IT(&huart1, &b1,1)==HAL_OK)
  osDelay(1);
  }
  /* USER CODE END Task_2_uart */
}

/* USER CODE BEGIN Header_Task_3_monitor */
/**
* @brief Function implementing the Monitor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Task_3_monitor */
void Task_3_monitor(void const * argument)
{
  /* USER CODE BEGIN Task_3_monitor */
    LCD_Clear();
  /* Infinite loop */
  for(;;)
  {
        osDelay(1000/portTICK_RATE_MS);
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);
        LCD_Goto(0,0);
        OLED_string("Encoder:");
        LCD_Goto(72,0);
        OLED_num_to_str(capture,4);
        LCD_Goto(0,1);
        OLED_string("Voltage[mV]:");
        LCD_Goto(72,1);
        OLED_num_to_str((int)capture*3.32,4); 
      
        
  }
  /* USER CODE END Task_3_monitor */
}

/* USER CODE BEGIN Header_AT_HANDLER */

int parse_args(char* str, char* argv[])
{
    int i = 0;
    char* ch = str;

    while(*ch != '\0') {
        i++;
        /*Check if length exceeds*/
        if (i > 3) {
            return 0;
        }

        argv[i-1] = ch;
        		
        while(*ch != '=' && *ch != '\0' && *ch != '\r') {
                ch++;
        }
        if (*ch == '\r')
            break;
				
        if (*ch != '\0') {
            *ch = '\0';
			//printf("parm: i=%d, %s \r\n", i-1, argv[i-1]);
            ch++;
            while(*ch == '=') {
                ch++;
            }
        }
    }
    return i;
}

int chack_data(char* str)
{
LCD_Clear();
LCD_Goto(0,2);
OLED_string(str);
   int argc;
  	char *argv[2]={NULL};
	argc = parse_args(str, argv);
        
        
	if (argc == 2){		
		//printf("%s\r\n",argv[0]);
		//printf("%s\r\n",argv[1]);
		char *buffer = argv[1];
				
		if (0 == strcmp(argv[0],"at+preset1")){
			if (strlen(buffer) > 2) {
		          Settings.Preset1.AdrData.data=(uint16_t)atoi(buffer);
                          SaveSettings();
                          printf("Preset 1 is:%d\r\n",Settings.Preset1.AdrData.data);               
			} else {
				return 1;
			}
		} else if (0 == strcmp(argv[0],"at+preset2")){
			if (strlen(buffer) > 2) {
		          Settings.Preset2.AdrData.data=(uint16_t)atoi(buffer);
                          SaveSettings();
                          printf("Preset 2 is:%d\r\n",Settings.Preset2.AdrData.data);               
			} else {
				return 1;
			}
		} else if (0 == strcmp(argv[0],"at+preset3")){
			if (strlen(buffer) > 2) {
		          Settings.Preset3.AdrData.data=(uint16_t)atoi(buffer);
                          SaveSettings();
                          printf("Preset 3 is:%d\r\n",Settings.Preset3.AdrData.data);               
			} else  {
				return 1;
			}
		} else if (0 == strcmp(argv[0],"at+preset4")){
			if (strlen(buffer) > 2) {
		          Settings.Preset4.AdrData.data=(uint16_t)atoi(buffer);
                          SaveSettings();
                          printf("Preset 4 is:%d\r\n",Settings.Preset4.AdrData.data);               
			} else {
				return 1;
			}		
		} else if (0 == strcmp(argv[0],"at+preset5")){
			if (strlen(buffer) > 2) {
		          Settings.Preset5.AdrData.data=(uint16_t)atoi(buffer);
                          SaveSettings();
                          printf("Preset 5 is:%d\r\n",Settings.Preset5.AdrData.data);               
			} else {
				return 1;
			}
	} else if (0 == strcmp(argv[0],"at+defpreset")){
			if (strlen(buffer) > 2) {
		          Settings.DefPrest.AdrData.data=(uint16_t)atoi(buffer);
                          SaveSettings();
                          printf("DefPreset is:%d\r\n",Settings.DefPrest.AdrData.data);               
			} else { 
				return 1;
			}
		} else if (0 == strcmp(argv[0],"at+scale")){
			if (strlen(buffer) >2) {
		          Settings.Scale.AdrData.data=(uint16_t)atoi(buffer);
                          SaveSettings();
                          printf("Scale is:%d\r\n",Settings.Scale.AdrData.data);               
			} else {
				return 1;
			}
		}else if (0 == strcmp(argv[0],"at+speed")){
			if (strlen(buffer) == 1) {
		          Settings.Speed.AdrData.data=(uint16_t)atoi(buffer);
                          SaveSettings();
                          printf("Speed is:%d\r\n",Settings.Speed.AdrData.data);               
			} else {
				return 1;
			}
		}
         else if (0 == strcmp(argv[0],"at+manual")){
			if (strlen(buffer) > 2) {
		         TIM3->CNT=(uint16_t)atoi(buffer)/Settings.Speed.AdrData.data;
                          printf("Manual is:%d\r\n",TIM3->CNT*Settings.Speed.AdrData.data);               
			} else {
				return 1;
			}
		}else if (0 == strcmp(argv[0],"at+factory")){
			if (strlen(buffer) > 0) {
                        DeffSettins();
		        printf("Factory Reset!\r\n");               
			} else {
				return 1;
			}
		} else {
			return 1;
		}
		return 0;
	}
	return 1;
}
/**


* @brief Function implementing the AT_handler thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_AT_HANDLER */
void AT_HANDLER(void const * argument)
{
  /* USER CODE BEGIN AT_HANDLER */
  myMes w;
  uint8_t data = '\0';
  
  /* Infinite loop */
  for(;;)
  {
   xQueueReceive( RecQHandle, &w, portMAX_DELAY );
   data=w.byte;
   if ( data != '\0' && data != 0xFF)
	{
		g_buffer[data_i] = data;
        		
		if (data == '\r') {
        g_buffer[data_i] = '\0';
          		}
		data_i++;
	}

    if (data == '\n') {

       // printf("%s\r\n",g_buffer);
         data_i = 0;
        if (chack_data(g_buffer) == 0){
		
			printf("OK\r\n");
            
		} else {
			printf("ERROR\r\n");
           
               }

    
    }
   
   
    osDelay(1);
  }
  /* USER CODE END AT_HANDLER */
}

/* USER CODE BEGIN Header_KLIK_KLACK */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  
if(GPIO_Pin==  GPIO_PIN_2)
{
 
  if(boottonPress==false)
  {
  boottonPress=true;
  //printf("PRESS\r\n");
  }
}
}
/**
* @brief Function implementing the KlikKlack thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_KLIK_KLACK */
void KLIK_KLACK(void const * argument)
{
  /* USER CODE BEGIN KLIK_KLACK */
uint8_t swCount=0;
  /* Infinite loop */
  for(;;)
  {
  
   if(boottonPress)
   {
    boottonPress=false;
   swCount++;
    if( swCount>5)swCount=1;
     switch (swCount) {
        case 1:
        TIM3->CNT=Settings.Preset1.AdrData.data/Settings.Speed.AdrData.data;
        printf("Press 1. Activated is:%d \r\n",Settings.Preset1.AdrData.data);
        break;
        case 2:
        TIM3->CNT=Settings.Preset2.AdrData.data/Settings.Speed.AdrData.data;
        printf("Press 2. Activated is:%d \r\n",Settings.Preset2.AdrData.data);
        break;
        case 3:
        TIM3->CNT=Settings.Preset3.AdrData.data/Settings.Speed.AdrData.data;
        printf("Press 3. Activated is:%d \r\n",Settings.Preset3.AdrData.data/Settings.Speed.AdrData.data);
        break;
        case 4:
        TIM3->CNT=Settings.Preset4.AdrData.data/Settings.Speed.AdrData.data;
        printf("Press 4. Activated is:%d \r\n",Settings.Preset4.AdrData.data/Settings.Speed.AdrData.data);
        break;
        case 5:
        TIM3->CNT=Settings.Preset5.AdrData.data/Settings.Speed.AdrData.data;
        printf("Press 5. Activated is:%d \r\n",Settings.Preset5.AdrData.data/Settings.Speed.AdrData.data);
        break;

        default:

        break;
}
     
   }
   osDelay(500/portTICK_RATE_MS);
  }
  /* USER CODE END KLIK_KLACK */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART2 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
