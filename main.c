/**
  *************************************************************************************************************************************
  * �������� �������: testMorion.
  * \version 1.0.0
  *************************************************************************************************************************************
  * ��������:
  * - ��: STM32F103C8
  * - ��������� ��������� � ����: PB6
  * - USRAT1: TX=PA9, RX=PA10 
  * - ����/����� ���������� USART ���������� ����� ����� DMA 
  *************************************************************************************************************************************
  * ������ ���������: ��� �������������� ������� ��������� ��������� ������ � ������ 500�� �������, 500�� ��������. 
  * ����� �������� ����� �������� ���������� �������������� ���������.
  *************************************************************************************************************************************
  * �����������:
  * 1. ������� "Blink X(ms) Y(ms)" - ������ ����� �������� ���������� � �������� �������� ���������/����������(X/Y). 
  * 2. ������� "Fade X(ms)"        - ������ ����� �������� ��������� � ��������� ����������, ��������� ��� �� Timer4.
  * 3. ������� "Stop"              - ������ �� � ����� ��������� � ����� 10 ���. ����������� ������ Watchdog(IWDG) � �� ���������������.
  * 4. ����������� ������������ � ������� � ���������� ������ � ���������. 
  * 5. ��������� ����������� � ���� � ������� Doxygen � ������������� ��������������� ������������.
  * 6. ������ git ����������� � �������� � ��������� ����������� �� GitHub(https://github.com/moozon/testMorion).
  *************************************************************************************************************************************
  */
/* Includes -------------------------------------------------------------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_iwdg.h"
#include "platform_config.h"
#include <string.h>
#include <stdlib.h>
#include "misc.h"

/* Private define --------------------------------------------------------------------------------------------------------------------*/
/// �������� USART
#define BAUDRATE                115200
/// ������ ��������� ������ USART
#define RX_BUFFER_SIZE          32
/// ������ ���������� ������ USART
#define TX_BUFFER_SIZE          128
/// ������������ �������� ���
#define PERIOD                  1000


/* Private variables -----------------------------------------------------------------------------------------------------------------*/
GPIO_InitTypeDef portA;
GPIO_InitTypeDef portB;
USART_InitTypeDef usart;
DMA_InitTypeDef dma;
NVIC_InitTypeDef nvic;
TIM_TimeBaseInitTypeDef timer;
TIM_OCInitTypeDef timerPWM;

/// TX ����� USART 
uint8_t txBuffer[128] = "Hello, please enter Blink or PWM time in ms. or Stop:\nExamples:\n         \"blink 500 500\"\n         \"fade 500\"\n         \"stop\"\n";

/// RX ����� USART
char rxBuffer[RX_BUFFER_SIZE];

/// ������� �������� ����������
uint32_t time1 = 5000000;      

/// ����� �� �������� ����������
uint32_t time2 = 5000000;    

/// ����� �������� ��������� � ��������� ����������, ��������� ��� �� Timer4.
uint32_t fadeTime = 10000;                      

/// ���� ��� ������ Fade
uint8_t fadeFlag = 0;   

/// ��������������� ������� ��� �������� ��������� ��������� �������
char tmp1[RX_BUFFER_SIZE] = {0};                            
char tmp2[RX_BUFFER_SIZE] = {0};                            
char tmp3[RX_BUFFER_SIZE] = {0};                            
char tmp4[RX_BUFFER_SIZE] = {0};                                 
char tmp5[RX_BUFFER_SIZE] = {0};                            
char tmp6[RX_BUFFER_SIZE] = {0};  

///  ��������������� ���������� ��� �������� ��������� ��������� �������
uint8_t y1 = 0;                                 
uint8_t y2 = 0;                                 
uint8_t y3 = 0;                                 
uint8_t y4 = 0;                                 
uint8_t y5 = 0;                                 
uint8_t y6 = 0;                                 
int x1 = 0;                                     
int x2 = 0;                                     
int x3 = 0;                                     
int x4 = 0;                                     
int x5 = 0;                                     
int x6 = 0;  

/// ����, ��� ������ ������ �� USART
uint8_t rxFlag = 0;       

/// �������� ���
int TIM_Pulse = 0;                              


/* Private function prototypes ---------------------------------------------------------------------------------------------------------*/

/// ������������� ���� ���������
void initAll();

/// ������ ����� �������� ���������� � �������� �������� ���������/����������
void blink(uint32_t timeOn, uint32_t timeOff);

/// ������ ����� �������� ��������� � ��������� ����������
void fade(uint32_t time);

/// ������ �� � ����� ��������� � ����� 10 ���. ����������� ������ Watchdog(IWDG) � �� ���������������.
void loop();

/// ������� ��������� �� USART ������� ������
void parseBuffer(char *buffer);


/// ���� � ���������
int main()
{
  /* ��������� ���������� IRQ */
   __enable_irq();
   
  /* ������������� ���� ��������� */ 
    initAll();    
    
   /* ���� ���� ���������� �������� ������ �� USART */
  while (DMA_GetFlagStatus(USART1_Tx_DMA_FLAG) == RESET)
  {
  }
  
  /// �������� ����     
  while (1)
  {   
    /* ���� ���� ��������� �� USART ������, �� ������������ �� */
    if (rxFlag)
    {
      parseBuffer(rxBuffer);
    }
    /* ����� �������� ���������� */
    if (!fadeFlag)
    {
     blink(time1, time2);
    }
    else
    {
     fade(fadeTime); 
    }
    /* ����� ������� Watchdog*/
    IWDG_ReloadCounter();
  }
}

void initAll()
{    
  /* ��������� ������������ ���������*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  
  
  /* ������������� DMA */   
  DMA_StructInit(&dma);  
     
  /* ������������� DMA ��� USART TX */ 
  DMA_DeInit(DMA1_Channel4);                                    // ����� ������ Dma
  dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);         // ��������� ����� �������� ������ USART 
  dma.DMA_MemoryBaseAddr = (uint32_t)txBuffer;                  // ��������� �����, ���� ������ ������ �� ������     
  dma.DMA_BufferSize = TX_BUFFER_SIZE;                          // ������ ������
  dma.DMA_DIR = DMA_DIR_PeripheralDST;                          // ����������� �������� ������(�� ������ � USART)
  dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            // ����������������� ������ � USART ���������    
  dma.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // ����������������� ������ � ������ ��������
  dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;     // ������ ��������� ������ � USART
  dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;             // ������ ��������� ������ � ������
  dma.DMA_Mode = DMA_Mode_Normal;                               // ����� ������ DMA
  dma.DMA_M2M = DMA_M2M_Disable;                                // ����� M2M ��������
  dma.DMA_Priority = DMA_Priority_High;                         // ��������� �������
  DMA_Init(DMA1_Channel4, &dma);                                // ������������� DMA
  
  /* ������������� DMA ��� USART RX */  
  DMA_DeInit(DMA1_Channel5);                                    // ����� ������ Dma
  dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);         // ��������� ����� �������� ������ USART 
  dma.DMA_MemoryBaseAddr = (uint32_t)rxBuffer;                  // ��������� �����, ���� ������ ������ �� ������
  dma.DMA_BufferSize = RX_BUFFER_SIZE;                          // ������ ������
  dma.DMA_DIR = DMA_DIR_PeripheralSRC;                          // ����������� �������� ������(�� USART � ������)
  dma.DMA_Mode = DMA_Mode_Circular;                             // ����� ������ DMA
  DMA_Init(DMA1_Channel5, &dma);                                // ������������� DMA
  
  /* ������������� GPIO */  
  GPIO_StructInit(&portB);                                      
  portB.GPIO_Mode = GPIO_Mode_Out_PP;                           // ����� ������ ���� - push-pull
  portB.GPIO_Pin = GPIO_Pin_6;                                  // ����� ���� ��� ����������� ����������
  portB.GPIO_Speed = GPIO_Speed_50MHz;                          // ������� ������������ �����
  GPIO_Init(GPIOB, &portB);                                     //
  
  portA.GPIO_Pin = USART1_TxPin;                                // Tx ���
  portA.GPIO_Speed = GPIO_Speed_50MHz;                          // ������� ������������ �����     
  portA.GPIO_Mode = GPIO_Mode_AF_PP;                            // ����� ������ ���� - �������������� �������, push-pull
  GPIO_Init(USART1_GPIO, &portA);
  
  portA.GPIO_Pin = USART1_RxPin;                                // Rx ���
  portA.GPIO_Mode = GPIO_Mode_IN_FLOATING;                      // ����� ������ ���� - ��������� ���� 
  GPIO_Init(USART1_GPIO, &portA);    
  
  /* ������������� NVIC */  
  nvic.NVIC_IRQChannel = USART1_IRQn;                           // ����� ���������� USART1
  nvic.NVIC_IRQChannelPreemptionPriority = 0;                   // ��������� 1
  nvic.NVIC_IRQChannelSubPriority = 0;                          // ��������� 2
  nvic.NVIC_IRQChannelCmd = ENABLE;                             // ��������� ���������� �� ������
  NVIC_Init(&nvic);  
      
  /* ������������� USART1 */  
  USART_StructInit(&usart);
  usart.USART_BaudRate = BAUDRATE;                              // �������� �������� ������ USART
  usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;             // ����� ������ USART TX+RX
  USART_Init(USART1, &usart);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                // ��������� ���������� �� ������� RXNE (�������� ������� �� ������)  
  
  /* ������������� Timer4 */
  TIM_TimeBaseStructInit(&timer);                               
  timer.TIM_Prescaler = 720;                                    // ������������ �������
  timer.TIM_Period = PERIOD;                                    // ������ 
  timer.TIM_ClockDivision = 0;                                  // ������� ������� �� ������������
  timer.TIM_CounterMode = TIM_CounterMode_Up;                   // ����� ����� ������� - ������
  TIM_TimeBaseInit(TIM4, &timer);
  
  /* ������������� ��� */
  TIM_OCStructInit(&timerPWM);
  timerPWM.TIM_OCMode = TIM_OCMode_PWM1;                        // ����� ������ ���
  timerPWM.TIM_OutputState = TIM_OutputState_Enable;            // �������� ��������� ���
  timerPWM.TIM_Pulse = 10;                                      // ����� �������
  timerPWM.TIM_OCPolarity = TIM_OCPolarity_High;                // ���������� ������� - ������������
  TIM_OC1Init(TIM4, &timerPWM);

  /* ������������� Watchdog */  
  RCC_LSICmd(ENABLE);
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);                 // ��������� ������ � ������� ������
  IWDG_SetPrescaler(IWDG_Prescaler_256);                        // ��������� ������������ ������� Watchdog        
  IWDG_SetReload(1562);                                         // ��������� ������� Watchdog �� 10 ���. (40000/256*10)
  IWDG_ReloadCounter();                                         // ���������� ������ Watchdog
  IWDG_Enable();                                                // �������� Watchdog
  
  /* ��������� ��������� */  
  USART_DMACmd(USART1, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);  
  DMA_Cmd(DMA1_Channel4, ENABLE);
  DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
  DMA_Cmd(DMA1_Channel5, ENABLE);    
  USART_Cmd(USART1, ENABLE);
  TIM_Cmd(TIM4, ENABLE);
  
}

void parseBuffer(char *buffer)
{  
  /* ������� �������� ������� �� �������� ������ */
  char cmd[5] = {0};
  for (uint8_t i = 0; i < 5; i++)
  {
    cmd[i] = buffer[i];
  }
  
  // Blink
  if (strncmp(cmd, "blink", 5) == 0)
  {       
    /* ������� �������� ������� �������� ���������� �� �������� ������ � ���������� �� � ����������*/
    x3 = strcspn(buffer," ");
    for(int x = x3 + 1 ; x < RX_BUFFER_SIZE; x++)
    {      
      tmp1[y1] = buffer[x];
      y1++;  
    }
    y1 = 0;
    
    x2 = strcspn(tmp1," ");
    for(int x = 0; x < x2; x++)
    {
      tmp2[y2] = tmp1[x];
      y2++;
    }    
    y2 = 0;
    
    for(int x = x2 + 1; x < RX_BUFFER_SIZE; x++)
    {
      tmp3[y3] = tmp1[x];
      y3++;
    }    
    y3 = 0;
    
    x4 = strcspn(tmp3,"\0"); 
    for(int x = 0; x < x4; x++)
    {
      tmp4[y4] = tmp3[x];
      y4++;
    }   
    y4 = 0;
    
    /* �������� �� ������ ��������, ����� ������ ������ ���� �������� ������� "Blink"*/
    if (tmp2[0] != 0){    
    time1 = (uint32_t)atoi(tmp2)*10000;
    }
    if (tmp4[0] != 0){ 
    time2 = (uint32_t)atoi(tmp4)*10000;
    }    
    /* ������� ��������� �������*/
    memset(tmp1, 0, sizeof(tmp1));
    memset(tmp2, 0, sizeof(tmp2));
    memset(tmp3, 0, sizeof(tmp3));
    memset(tmp4, 0, sizeof(tmp4));
   
    /* �������, ��� ����� �������� ����� "Blink"*/
    fadeFlag = 0; 
    
    /* ������������ ������ ������ ������ ��� ������ "Blink"*/
    portB.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &portB);

  }
  // Fade
  if (strncmp(cmd,"fade", 4) == 0)
  {  
    /* ������� �������� ��� �� �������� ������ � ���������� ��� � ����������*/
    x5 = strcspn(buffer," ");
    for(int x = x5 + 1 ; x < RX_BUFFER_SIZE; x++)
    {      
      tmp5[y5] = buffer[x];
      y5++;  
    }
    y5 = 0;
    
    x6 = strcspn(tmp5," ");
    for(int x = 0; x < x6; x++)
    {
      tmp6[y6] = tmp5[x];
      y6++;
    }    
    y6 = 0;
    
    /* ������ �������� ��� � ����������*/
    fadeTime = (uint32_t)atoi(tmp6)*10;
    
    /* ������� ��������� �������*/
    memset(tmp5, 0, sizeof(tmp5));
    memset(tmp6, 0, sizeof(tmp6));
    
    /* �������, ��� ����� �������� ����� "Fade"*/
    fadeFlag = 1;
    
    /* ������������ ������ ������ ������ ��� ������ ���*/
    portB.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &portB);
  }
  // Stop
  if (strncmp(cmd,"stop", 4) == 0)
  {
    loop();
  }
  
  /* ������� ��������� ������ */
  memset(rxBuffer, 0, sizeof(rxBuffer));
  
  /* �������, ��� �������� ����� ���������*/
  rxFlag = 0;
  
  /* ��������� ����� ������������ ���������� USART*/
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}
/// ���������� ���������� USART
void USART1_IRQHandler(void)
{
  /* ���������� ���������� USART, ����� ��������� ������ ������������*/
  USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
  
  /* �������, ��� ������ ������ �� USART � �� ����� ����������*/
  rxFlag = 1;
  
  /* ��������, ����� ������������� ����� "�����" ������ */
  for(int j = 0; j < 100000; j++);
  
  /* ����������������� DMA, ��� ����, ����� ��� ��������� ������, ������ �� ������ ������������ � ������ ��������� ������*/  
  DMA_Cmd(DMA1_Channel5, DISABLE);
  dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
  dma.DMA_MemoryBaseAddr = (uint32_t)rxBuffer;
  dma.DMA_BufferSize = TX_BUFFER_SIZE;
  DMA_Init(DMA1_Channel5, &dma);  
  DMA_Cmd(DMA1_Channel5, ENABLE);
}

void blink(uint32_t timeOn, uint32_t timeOff)
{
  uint32_t i;
  
  /* ��������� �������*/
  GPIOB->ODR |= GPIO_Pin_6;   
  for(i = 0; i < timeOn; i++); 
  
  /* ��������� ��������*/
  GPIOB->ODR &= ~GPIO_Pin_6;
  for(i = 0; i < timeOff; i++);
}

void fade(uint32_t time)
{
  int i;
  
  /* ������� ��������� ����������*/
  for (i = 0; i < PERIOD; i++)
  {        
    TIM4->CCR1 = i;
    for(int j = 0; j < time; j++);
  }
  /* ������� ���������� ����������*/
  for (i = PERIOD; i > 0; i--)
  {
    TIM4->CCR1 = i;
    for(int j = 0; j < time; j++);
  }
}
void loop()
{
  /* ����������� ���� ��� ������������ ������� Watchdog*/
  while(1);
}
