// Дефолтная конфигурация - сделано под TE-STM32F103_BOARD
		
		//#define USART1                   USART1
		#define USART1_GPIO              GPIOA
		#define USART1_CLK               RCC_APB2Periph_USART1
		#define USART1_GPIO_CLK          RCC_APB2Periph_GPIOA
		#define USART1_RxPin             GPIO_Pin_10
		#define USART1_TxPin             GPIO_Pin_9
		#define USART1_Tx_DMA_Channel    DMA1_Channel4
		#define USART1_Tx_DMA_FLAG       DMA1_FLAG_TC4
                #define USART1_Rx_DMA_Channel    DMA1_Channel5
		#define USART1_Rx_DMA_FLAG       DMA1_FLAG_TC5
		#define USART1_DR_Base           0x40013804



        #define USB_DISCONNECT                      GPIOB  
        #define USB_DISCONNECT_PIN                  GPIO_Pin_5
        #define USB_DISCONNECT_LOG1                 DISABLE
        #define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOB

        #define LED1                                0
        #define LED1_GPIO_PORT                      GPIOA
        #define LED1_GPIO_CLK                       RCC_APB2Periph_GPIOA  
        #define LED1_GPIO_PIN                       GPIO_Pin_13
        

        #define USE_USART1_DEFAULT_PA9_PA10

