/**
  *************************************************************************************************************************************
  * Тестовое задание: testMorion.
  * \version 1.0.0
  *************************************************************************************************************************************
  * Описание:
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
  * 2. Команда "Fade X(ms)"        - Задает режим плавного зажигания и затухания светодиода, используя Шим на Timer4.
  * 3. Команда "Stop"              - Вводит МК в режим зависания и через 10 сек. срабатывает таймер Watchdog(IWDG) и МК перезагружается.
  * 4. Приглашение пользователк в консоле к выполнению команд с примерами. 
  * 5. Оформлены комментарии к коду в формате Doxygen и сгенерирована соответствующая документация.
  * 6. Создан git репозиторий и выгружен в удаленный репозиторий на GitHub(https://github.com/moozon/testMorion).
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
/// Скорость USART
#define BAUDRATE                115200
/// Размер входящего буфера USART
#define RX_BUFFER_SIZE          32
/// Размер исходящего буфера USART
#define TX_BUFFER_SIZE          128
/// Максимальное значение ШИМ
#define PERIOD                  1000


/* Private variables -----------------------------------------------------------------------------------------------------------------*/
GPIO_InitTypeDef portA;
GPIO_InitTypeDef portB;
USART_InitTypeDef usart;
DMA_InitTypeDef dma;
NVIC_InitTypeDef nvic;
TIM_TimeBaseInitTypeDef timer;
TIM_OCInitTypeDef timerPWM;

/// TX буфер USART 
uint8_t txBuffer[128] = "Hello, please enter Blink or PWM time in ms. or Stop:\nExamples:\n         \"blink 500 500\"\n         \"fade 500\"\n         \"stop\"\n";

/// RX буфер USART
char rxBuffer[RX_BUFFER_SIZE];

/// Временя свечения светодиода
uint32_t time1 = 5000000;      

/// Время НЕ свечения светодиода
uint32_t time2 = 5000000;    

/// Время плавного зажигания и затухания светодиода, используя Шим на Timer4.
uint32_t fadeTime = 10000;                      

/// Флаг для режима Fade
uint8_t fadeFlag = 0;   

/// Вспомогательные массивы для парсинга основного входящего массива
char tmp1[RX_BUFFER_SIZE] = {0};                            
char tmp2[RX_BUFFER_SIZE] = {0};                            
char tmp3[RX_BUFFER_SIZE] = {0};                            
char tmp4[RX_BUFFER_SIZE] = {0};                                 
char tmp5[RX_BUFFER_SIZE] = {0};                            
char tmp6[RX_BUFFER_SIZE] = {0};  

///  Вспомогательные переменные для парсинга основного входящего массива
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

/// Флаг, что пришли данные по USART
uint8_t rxFlag = 0;       

/// Значение ШИМ
int TIM_Pulse = 0;                              


/* Private function prototypes ---------------------------------------------------------------------------------------------------------*/

/// Инициализация всей периферии
void initAll();

/// Задает режим свечения светодиода с заданным временем включения/выключения
void blink(uint32_t timeOn, uint32_t timeOff);

/// Задает режим плавного зажигания и затухания светодиода
void fade(uint32_t time);

/// Вводит МК в режим зависания и через 10 сек. срабатывает таймер Watchdog(IWDG) и МК перезагружается.
void loop();

/// Парсинг приянтого по USART массива данных
void parseBuffer(char *buffer);


/// Вход в программу
int main()
{
  /* Разрешить прерывания IRQ */
   __enable_irq();
   
  /* Инициализация всей периферии */ 
    initAll();    
    
   /* Ждем пока завершится передача данных по USART */
  while (DMA_GetFlagStatus(USART1_Tx_DMA_FLAG) == RESET)
  {
  }
  
  /// Основной цикл     
  while (1)
  {   
    /* Если есть принятные по USART данные, то обрабатываем их */
    if (rxFlag)
    {
      parseBuffer(rxBuffer);
    }
    /* Режим свечения светодиода */
    if (!fadeFlag)
    {
     blink(time1, time2);
    }
    else
    {
     fade(fadeTime); 
    }
    /* Сброс таймера Watchdog*/
    IWDG_ReloadCounter();
  }
}

void initAll()
{    
  /* Включение тактирования периферии*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  
  
  /* Инициализация DMA */   
  DMA_StructInit(&dma);  
     
  /* Инициализация DMA для USART TX */ 
  DMA_DeInit(DMA1_Channel4);                                    // Выбор канала Dma
  dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);         // Указываем адрес регистра данных USART 
  dma.DMA_MemoryBaseAddr = (uint32_t)txBuffer;                  // Указываем буфер, куда писать данные из памяти     
  dma.DMA_BufferSize = TX_BUFFER_SIZE;                          // Размер буфера
  dma.DMA_DIR = DMA_DIR_PeripheralDST;                          // Направление передачи данных(из памяти в USART)
  dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            // Инкрементирование адреса в USART выключено    
  dma.DMA_MemoryInc = DMA_MemoryInc_Enable;                     // Инкрементирование адреса в памяти включено
  dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;     // Размер обработки данных в USART
  dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;             // Размер обработки данных в памяти
  dma.DMA_Mode = DMA_Mode_Normal;                               // Режим работы DMA
  dma.DMA_M2M = DMA_M2M_Disable;                                // Режим M2M выключен
  dma.DMA_Priority = DMA_Priority_High;                         // Приоритет высокий
  DMA_Init(DMA1_Channel4, &dma);                                // Инициализация DMA
  
  /* Инициализация DMA для USART RX */  
  DMA_DeInit(DMA1_Channel5);                                    // Выбор канала Dma
  dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);         // Указываем адрес регистра данных USART 
  dma.DMA_MemoryBaseAddr = (uint32_t)rxBuffer;                  // Указываем буфер, куда писать данные из памяти
  dma.DMA_BufferSize = RX_BUFFER_SIZE;                          // Размер буфера
  dma.DMA_DIR = DMA_DIR_PeripheralSRC;                          // Направление передачи данных(из USART в память)
  dma.DMA_Mode = DMA_Mode_Circular;                             // Режим работы DMA
  DMA_Init(DMA1_Channel5, &dma);                                // Инициализация DMA
  
  /* Инициализация GPIO */  
  GPIO_StructInit(&portB);                                      
  portB.GPIO_Mode = GPIO_Mode_Out_PP;                           // Режим работы пина - push-pull
  portB.GPIO_Pin = GPIO_Pin_6;                                  // Номер пииа для подключения светодиода
  portB.GPIO_Speed = GPIO_Speed_50MHz;                          // Частота тактирования порта
  GPIO_Init(GPIOB, &portB);                                     //
  
  portA.GPIO_Pin = USART1_TxPin;                                // Tx пин
  portA.GPIO_Speed = GPIO_Speed_50MHz;                          // Частота тактирования порта     
  portA.GPIO_Mode = GPIO_Mode_AF_PP;                            // Режим работы пина - альтернативная функция, push-pull
  GPIO_Init(USART1_GPIO, &portA);
  
  portA.GPIO_Pin = USART1_RxPin;                                // Rx пин
  portA.GPIO_Mode = GPIO_Mode_IN_FLOATING;                      // Режим работы пина - плавающий вход 
  GPIO_Init(USART1_GPIO, &portA);    
  
  /* Инициализация NVIC */  
  nvic.NVIC_IRQChannel = USART1_IRQn;                           // Канал прерываний USART1
  nvic.NVIC_IRQChannelPreemptionPriority = 0;                   // Приоритет 1
  nvic.NVIC_IRQChannelSubPriority = 0;                          // Приоритет 2
  nvic.NVIC_IRQChannelCmd = ENABLE;                             // Включение прерываний на канале
  NVIC_Init(&nvic);  
      
  /* Инициализация USART1 */  
  USART_StructInit(&usart);
  usart.USART_BaudRate = BAUDRATE;                              // Скорость передачи данных USART
  usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;             // Режим работы USART TX+RX
  USART_Init(USART1, &usart);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                // Включение прерывание по событию RXNE (Входящий регистр не пустой)  
  
  /* Инициализация Timer4 */
  TIM_TimeBaseStructInit(&timer);                               
  timer.TIM_Prescaler = 720;                                    // Предделитель таймера
  timer.TIM_Period = PERIOD;                                    // Период 
  timer.TIM_ClockDivision = 0;                                  // Деление частоты не используется
  timer.TIM_CounterMode = TIM_CounterMode_Up;                   // Режим счета таймера - прямой
  TIM_TimeBaseInit(TIM4, &timer);
  
  /* Инициализация ШИМ */
  TIM_OCStructInit(&timerPWM);
  timerPWM.TIM_OCMode = TIM_OCMode_PWM1;                        // Режим работы ШИМ
  timerPWM.TIM_OutputState = TIM_OutputState_Enable;            // Включаем генерацию ШИМ
  timerPWM.TIM_Pulse = 10;                                      // Время сигнала
  timerPWM.TIM_OCPolarity = TIM_OCPolarity_High;                // Полярность сигнала - Положителная
  TIM_OC1Init(TIM4, &timerPWM);

  /* Инициализация Watchdog */  
  RCC_LSICmd(ENABLE);
  while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);                 // Разрешаем запись в регистр сброса
  IWDG_SetPrescaler(IWDG_Prescaler_256);                        // Установка предделителя таймера Watchdog        
  IWDG_SetReload(1562);                                         // Установка таймера Watchdog на 10 сек. (40000/256*10)
  IWDG_ReloadCounter();                                         // Сбрасываем таймер Watchdog
  IWDG_Enable();                                                // Включаем Watchdog
  
  /* Включение периферии */  
  USART_DMACmd(USART1, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);  
  DMA_Cmd(DMA1_Channel4, ENABLE);
  DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
  DMA_Cmd(DMA1_Channel5, ENABLE);    
  USART_Cmd(USART1, ENABLE);
  TIM_Cmd(TIM4, ENABLE);
  
}

void parseBuffer(char *buffer)
{  
  /* Находим название команды во входящем буфере */
  char cmd[5] = {0};
  for (uint8_t i = 0; i < 5; i++)
  {
    cmd[i] = buffer[i];
  }
  
  // Blink
  if (strncmp(cmd, "blink", 5) == 0)
  {       
    /* Находим значения времени свечения светодиода во входящем буфере и записываем их в переменные*/
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
    
    /* Проверка на пустое значение, когда указан только один параметр команды "Blink"*/
    if (tmp2[0] != 0){    
    time1 = (uint32_t)atoi(tmp2)*10000;
    }
    if (tmp4[0] != 0){ 
    time2 = (uint32_t)atoi(tmp4)*10000;
    }    
    /* Очистка временных буферов*/
    memset(tmp1, 0, sizeof(tmp1));
    memset(tmp2, 0, sizeof(tmp2));
    memset(tmp3, 0, sizeof(tmp3));
    memset(tmp4, 0, sizeof(tmp4));
   
    /* Говорим, что нужно включить режим "Blink"*/
    fadeFlag = 0; 
    
    /* Переключение режима работы вывода для режима "Blink"*/
    portB.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &portB);

  }
  // Fade
  if (strncmp(cmd,"fade", 4) == 0)
  {  
    /* Находим значение ШИМ во входящем буфере и записываем его в переменную*/
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
    
    /* Запись значения ШИМ в переменную*/
    fadeTime = (uint32_t)atoi(tmp6)*10;
    
    /* Очистка временных буферов*/
    memset(tmp5, 0, sizeof(tmp5));
    memset(tmp6, 0, sizeof(tmp6));
    
    /* Говорим, что нужно включить режим "Fade"*/
    fadeFlag = 1;
    
    /* Переключение режима работы вывода для работы ШИМ*/
    portB.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &portB);
  }
  // Stop
  if (strncmp(cmd,"stop", 4) == 0)
  {
    loop();
  }
  
  /* Очистка входящего буфера */
  memset(rxBuffer, 0, sizeof(rxBuffer));
  
  /* Говорим, что входящий буфер обработан*/
  rxFlag = 0;
  
  /* Включение ранее отключенного прерывания USART*/
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}
/// Обработчик прерываний USART
void USART1_IRQHandler(void)
{
  /* Выключение прерывания USART, чтобы исключить ложное срабатывание*/
  USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
  
  /* Говорим, что пришли данные по USART и их нужно обработать*/
  rxFlag = 1;
  
  /* Задержка, чтобы предотвратить прием "битых" данных */
  for(int j = 0; j < 100000; j++);
  
  /* Переинициализация DMA, для того, чтобы при очередном приеме, данные из памяти записывались в начало входящего буфера*/  
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
  
  /* Светодиод включен*/
  GPIOB->ODR |= GPIO_Pin_6;   
  for(i = 0; i < timeOn; i++); 
  
  /* Светодиод выключен*/
  GPIOB->ODR &= ~GPIO_Pin_6;
  for(i = 0; i < timeOff; i++);
}

void fade(uint32_t time)
{
  int i;
  
  /* Плавное включение светодиода*/
  for (i = 0; i < PERIOD; i++)
  {        
    TIM4->CCR1 = i;
    for(int j = 0; j < time; j++);
  }
  /* Плавное выключение светодиода*/
  for (i = PERIOD; i > 0; i--)
  {
    TIM4->CCR1 = i;
    for(int j = 0; j < time; j++);
  }
}
void loop()
{
  /* Бесконечный цикл для срабатывания таймера Watchdog*/
  while(1);
}
