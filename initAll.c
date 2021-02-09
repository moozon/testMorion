//#include "initAll.h"
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_iwdg.h"

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