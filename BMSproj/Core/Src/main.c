
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_system.h"
#include "stm32l1xx.h"              // Device header (core + registers)
#include "stm32l1xx_ll_bus.h"       // For enabling peripheral clocks
#include "stm32l1xx_ll_gpio.h"      // For configuring TX/RX pins
#include "stm32l1xx_ll_usart.h"     // For USART functions
#include "stm32l1xx_ll_utils.h"     // For delay, SysTick helpers



//Mosfet driven switch control function
void MosfetSet(uint8_t mode);
//USART2 functions
void USART2_Init(void);
void USART2_SendData(uint8_t *data, uint16_t size);
//clock config
void SystemClock_Config(void);



int main(void)
{
    // Configure system clock (basic setup)
    SystemClock_Config();

    // Enable clock for GPIOA
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    // Enabling GPIOB (Pins D14 PB9 and D15 PB8) for Mosfet control
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

    // Configure PA5 as output
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_5, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_5, LL_GPIO_PULL_NO);
    // configure PB8 for Output
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_8, LL_GPIO_PULL_NO);
    // Configure PB9 for output
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_9, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_9, LL_GPIO_PULL_NO);

    // PA1 (A1)
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_1, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_1, LL_GPIO_PULL_NO);

    // PA4 (A2)
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_4, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_4, LL_GPIO_PULL_NO);

    // PB0 (A3)
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_0, LL_GPIO_MODE_ANALOG);
    LL_GPIO_SetPinPull(GPIOB, LL_GPIO_PIN_0, LL_GPIO_PULL_NO);


    //Usart init
    USART2_Init();

    //Loop
    while (1)
    {
    	MosfetSet(0);
    	uint8_t msg1[] = "Switch off\r\n";
    	USART2_SendData(msg1, sizeof(msg1)-1);
    	LL_mDelay(1000);

    	MosfetSet(1);
    	uint8_t msg2[] = "Switch charge\r\n";
    	USART2_SendData(msg2, sizeof(msg2)-1);
    	LL_mDelay(1000);

    	MosfetSet(2);
    	uint8_t msg3[] = "Switch discharge\r\n";
       	USART2_SendData(msg3, sizeof(msg3)-1);
       	LL_mDelay(1000);

    }
}

// Function to control D14 (PB9) and D15 (PB8)
void MosfetSet(uint8_t mode)
{
    switch(mode)
    {
        case 0: // Both OFF
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_8); // D15
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_9); // D14
            break;

        case 1: // D14 ON, D15 OFF
            LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_9);   // D14 ON
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_8); // D15 OFF
            break;

        case 2: // D14 OFF, D15 ON
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_9); // D14 OFF
            LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_8);   // D15 ON
            break;

        default: // Invalid value → both OFF
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_8);
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_9);
            break;
    }
}

void USART2_Init(void)
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    // PA2 = TX
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_2, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_2, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_2, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_2, LL_GPIO_PULL_UP);
    LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_2, LL_GPIO_AF_7);

    // PA3 = RX
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_3, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_3, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_3, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_3, LL_GPIO_PULL_UP);
    LL_GPIO_SetAFPin_0_7(GPIOA, LL_GPIO_PIN_3, LL_GPIO_AF_7);

    // USART2 configuration
    LL_USART_Disable(USART2);
    LL_USART_SetTransferDirection(USART2, LL_USART_DIRECTION_TX_RX);
    LL_USART_ConfigCharacter(USART2, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
    LL_USART_SetBaudRate(USART2, SystemCoreClock, LL_USART_OVERSAMPLING_16, 115200);
    LL_USART_Enable(USART2);
}



void USART2_SendData(uint8_t *data, uint16_t size)
{
    for(uint16_t i = 0; i < size; i++)
    {
        // Wait until TXE (Transmit Data Register Empty) flag is set
        while(!LL_USART_IsActiveFlag_TXE(USART2));

        // Send byte
        LL_USART_TransmitData8(USART2, data[i]);
    }

    // Wait until TC (Transmission Complete) flag is set
    while(!LL_USART_IsActiveFlag_TC(USART2));
}

void SystemClock_Config(void)
{
    // Default HSI = 16 MHz
    LL_InitTick(SystemCoreClock, 1000); // 1ms tick
    LL_SetSystemCoreClock(SystemCoreClock);
}
