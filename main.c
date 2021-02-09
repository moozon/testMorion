/**
  *************************************************************************************************************************************
  * Тестовое задание: testMorion.
  *************************************************************************************************************************************
  *Описание:
  * - МК: STM32F103C8
  * - Светодиод подключен к пину: PB6
  * - USRAT1: TX=PA9, RX=PA10 
  * - Ввод/вывод интерфейса USART реализован через канал DMA 
  *************************************************************************************************************************************
  * Работа программы: При первоначальном запуске программы светодиод мигает в режиме 500мс включен, 500мс выключен. 
  * Можно изменять режим свечения светодиода нижеописанными командами.
  *************************************************************************************************************************************
  * Реализовано:
  * 1. Команда "Blink X(ms) Y(ms)" - Задает режим свечения светодиода с заданным временем включения/выключения(X/Y). 
  * 2. Команда "Fade X(ms)"        - Задает режим плавного зажегания и затухания светодиода, используя Шим на Timer4.
  * 3. Команда "Stop"              - Вводит МК в режим зависания и через 10 сек. срабатывает таймер Watchdog(IWDG) и МК перезагружается.
  * 4. Приглашение пользователк в консоле к выполнению команд с примерами. 
  * 5. Оформлены комментарии к коду в формате Doxyge и сгенерирована соответствующая документация.
  * 6. Создан git репозиторий и выгружен в удаленный репозиторий на GitHub().
  *************************************************************************************************************************************
  */

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



#define BAUDRATE                115200
#define RX_BUFFER_SIZE          32
#define TX_BUFFER_SIZE          128
#define PERIOD                  1000


// User Vars
GPIO_InitTypeDef portA;
GPIO_InitTypeDef portB;
GPIO_InitTypeDef portC;
USART_InitTypeDef usart;
NVIC_InitTypeDef nvic;
TIM_TimeBaseInitTypeDef timer;
TIM_OCInitTypeDef timerPWM;


uint8_t inputData;
uint8_t outputData;
DMA_InitTypeDef dma;
uint8_t txBuffer[128] = "Hello, please enter Blink or PWM time in ms.:\nExamples:\n         \"blink 500 500\"\n         \"fade 500\"\n";
char rxBuffer[RX_BUFFER_SIZE];
char tmprxBuffer[RX_BUFFER_SIZE];
uint32_t time1 = 5000000;
uint32_t time2 = 5000000;
uint32_t fadeTime = 10000;
char _time1[4];
char _time2[4];
uint32_t index = 0;
uint8_t fadeFlag = 0;
uint8_t rx_counter = 0;
char tmp1[32] = {0};
char tmp2[32] = {0};
char tmp3[32] = {0};
char tmp4[32] = {0};
char tmp5[32] = {0};
char tmp6[32] = {0};
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
uint8_t flagBuf = 0;
uint8_t itFlag = 0;
uint8_t rxFlag = 0;
int TIM_Pulse = 0;


// Methods definition
void initAll();
void blink(uint32_t timeOn, uint32_t timeOff);
void usartDmaReceive();
void parseBuffer(char *buffer);
void fade(uint32_t time);
void loop();

int main()
{
   __enable_irq();
    initAll();
    
   /* Wait until USART1 TX DMA1 Channel Transfer Complete */
  while (DMA_GetFlagStatus(USART1_Tx_DMA_FLAG) == RESET)
  {
  }
      
  while (1)
  {   
    if (rxFlag)
    {
      parseBuffer(rxBuffer);
    }
    if (!fadeFlag)
    {
     blink(time1, time2);
    }
    else
    {
     fade(fadeTime); 
    }
    IWDG_ReloadCounter();
  }
}

void initAll()
{    
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  /* Initialize Leds mounted on STM32 board */  
  
  DMA_StructInit(&dma);
  
   /* USART DMA TX init */
  DMA_DeInit(DMA1_Channel4);
  dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
  dma.DMA_MemoryBaseAddr = (uint32_t)txBuffer;
  dma.DMA_BufferSize = TX_BUFFER_SIZE;
  dma.DMA_DIR = DMA_DIR_PeripheralDST;  
  dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  //dma.DMA_Mode = DMA_Mode_Circular;
  dma.DMA_Mode = DMA_Mode_Normal;
  dma.DMA_M2M = DMA_M2M_Disable;
  dma.DMA_Priority = DMA_Priority_High;  
  DMA_Init(DMA1_Channel4, &dma);
  
  
  /* USART DMA RX init */
  DMA_DeInit(DMA1_Channel5);
  dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
  dma.DMA_MemoryBaseAddr = (uint32_t)rxBuffer;
  dma.DMA_BufferSize = RX_BUFFER_SIZE;
  dma.DMA_DIR = DMA_DIR_PeripheralSRC;  
  //dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  //dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
  //dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  //dma.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  dma.DMA_Mode = DMA_Mode_Circular;
  //dma.DMA_Mode = DMA_Mode_Normal;
  //dma.DMA_M2M = DMA_M2M_Disable;
  //dma.DMA_Priority = DMA_Priority_High;  
  DMA_Init(DMA1_Channel5, &dma);  
  
  
  
    
  
  /* Initialize LED which connected to PC13, Enable the Clock*/
  
  /* Configure the GPIO_BUILTIN_LED pin */
//  GPIO_StructInit(&portC);
//  portC.GPIO_Pin = GPIO_Pin_13;
//  portC.GPIO_Mode = GPIO_Mode_Out_PP;
//  portC.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_Init(GPIOC, &portC);
  
  GPIO_StructInit(&portB);  
  portB.GPIO_Mode = GPIO_Mode_Out_PP;
  //portB.GPIO_Mode = GPIO_Mode_AF_PP;
  portB.GPIO_Pin = GPIO_Pin_6;
  portB.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &portB);
  
  portA.GPIO_Pin = USART1_TxPin;
  portA.GPIO_Speed = GPIO_Speed_50MHz;
  portA.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(USART1_GPIO, &portA);
  
  portA.GPIO_Pin = USART1_RxPin;
  portA.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(USART1_GPIO, &portA);
  
  
  /* NVIC */
  /* Enable the USARTz Interrupt */
  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  nvic.NVIC_IRQChannel = USART1_IRQn;
  nvic.NVIC_IRQChannelPreemptionPriority = 0;
  nvic.NVIC_IRQChannelSubPriority = 0;
  nvic.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvic);
  //NVIC_EnableIRQ(DMA1_Channel5_IRQn); //Dma interrupts Enable
  
  
  /* USART Interrupt */
  USART_StructInit(&usart);
  usart.USART_BaudRate = BAUDRATE;
  usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &usart);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //Usart RXNE interrupts Enable
  
  /* Timer  */
  
  TIM_TimeBaseStructInit(&timer);
  timer.TIM_Prescaler = 720;
  timer.TIM_Period = PERIOD;
  timer.TIM_ClockDivision = 0;
  timer.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &timer);

  TIM_OCStructInit(&timerPWM);
  timerPWM.TIM_OCMode = TIM_OCMode_PWM1;
  timerPWM.TIM_OutputState = TIM_OutputState_Enable;
  timerPWM.TIM_Pulse = 10;
  timerPWM.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OC1Init(TIM4, &timerPWM);

  
  RCC_LSICmd(ENABLE);
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  IWDG_SetReload(1562);
  IWDG_ReloadCounter();
  IWDG_Enable();  
  
  
  USART_DMACmd(USART1, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);  
    DMA_Cmd(DMA1_Channel4, ENABLE);
    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel5, ENABLE);    
    USART_Cmd(USART1, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
  
}

void parseBuffer(char *buffer)
{
  char cmd[5] = {0};  
  
  for (uint8_t i = 0; i < 5; i++)
  {
    cmd[i] = buffer[i];
    //for(int j = 0; j < 100; j++);
  }
  
  // Blink
  if (strncmp(cmd, "blink", 5) == 0)
  {   
    x3 = strcspn(buffer," ");
    for(int x = x3 + 1 ; x < 32; x++)
    {      
      tmp1[y1] = buffer[x];
      y1++;  
    }
    y1 = 0;
    
    x2 = strcspn(tmp1," ");
    //x2 = (int)strchr(tmp3, '%');
    for(int x = 0; x < x2; x++)
    {
      tmp2[y2] = tmp1[x];
      y2++;
    }    
    y2 = 0;
    
    for(int x = x2 + 1; x < 32; x++)
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
    
    if (tmp2[0] != 0){    
    time1 = (uint32_t)atoi(tmp2)*10000;
    }
    if (tmp4[0] != 0){ 
    time2 = (uint32_t)atoi(tmp4)*10000;
    }
    memset(tmp1, 0, sizeof(tmp1));
    memset(tmp2, 0, sizeof(tmp2));
    memset(tmp3, 0, sizeof(tmp3));
    memset(tmp4, 0, sizeof(tmp4));
   
    fadeFlag = 0; 
    
    //GPIO_StructInit(&portB);
    portB.GPIO_Mode = GPIO_Mode_Out_PP;
    //portB.GPIO_Mode = GPIO_Mode_AF_PP;
    //portB.GPIO_Pin = GPIO_Pin_6;
    //portB.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &portB);

  }
  // Fade
  if (strncmp(cmd,"fade", 4) == 0)
  {  
    x5 = strcspn(buffer," ");
    for(int x = x5 + 1 ; x < 32; x++)
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
    
    fadeTime = (uint32_t)atoi(tmp6)*10;
    
    memset(tmp5, 0, sizeof(tmp5));
    memset(tmp6, 0, sizeof(tmp6));
    
    
    fadeFlag = 1;
    
    //GPIO_StructInit(&portB);
    //portB.GPIO_Mode = GPIO_Mode_Out_PP;
    portB.GPIO_Mode = GPIO_Mode_AF_PP;
    //portB.GPIO_Pin = GPIO_Pin_6;
    //portB.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &portB);
  }
  // Stop
  if (strncmp(cmd,"stop", 4) == 0)
  {
    loop();
  }
  
  
  memset(rxBuffer, 0, sizeof(rxBuffer));
  rxFlag = 0;
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void USART1_IRQHandler(void)
{
 
//  if (USART_GetFlagStatus(USART1,USART_FLAG_ORE) != RESET)
//    {
//      parseBuffer(rxBuffer);
//    }
//  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
//    {
//      parseBuffer(rxBuffer);
//    }

  USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
  //parseBuffer(rxBuffer);
  rxFlag = 1;
  
  for(int j = 0; j < 100000; j++);
  
  DMA_Cmd(DMA1_Channel5, DISABLE);
  dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
  dma.DMA_MemoryBaseAddr = (uint32_t)rxBuffer;
  dma.DMA_BufferSize = TX_BUFFER_SIZE;
  DMA_Init(DMA1_Channel5, &dma);  
  DMA_Cmd(DMA1_Channel5, ENABLE);
  
  //USART_ReceiveData(USART1);
  
  //usartDmaReceive();
  //if ( USART_GetITStatus(USART1, USART_IT_RXNE) == RESET ) {
  //DMA_GetITStatus(DMA1_IT
//  if (USART1->SR & USART_SR_RXNE){
//  parseBuffer(rxBuffer);
//        USART_ClearFlag(USART1, USART_FLAG_RXNE);
//        USART_ClearFlag(USART1, USART_FLAG_ORE);
//        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//        USART_ClearITPendingBit(USART1, USART_IT_ORE);
//        //USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
//    };
//  
  //USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  //USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//  USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
  


}

void blink(uint32_t timeOn, uint32_t timeOff)
{
  uint32_t i;
  GPIOB->ODR |= GPIO_Pin_6;   
  for(i = 0; i < timeOn; i++);    
  GPIOB->ODR &= ~GPIO_Pin_6;
  for(i = 0; i < timeOff; i++);
}

void fade(uint32_t time)
{
  int i;
      for (i = 0; i < PERIOD; i++)
      {
        TIM4->CCR1 = i;
        for(int j = 0; j < time; j++);
      }
      for (i = PERIOD; i > 0; i--)
      {
        TIM4->CCR1 = i;
        for(int j = 0; j < time; j++);
      }
}
void loop()
{
  while(1);
}
