#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stm32l1xx.h"
#include "stm32l1xx_conf.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_usart.h"
#include "stm32l1xx_ll_adc.h"

// --- FUNCTION PROTOTYPES ---
void SystemClock_Config(void);
void UART2_Init(void);
void ADC_Temp_Init(void);
uint16_t ADC_Temp_ReadRaw(void);
void UART_SendString(const char *s);
int16_t Temp_From_mV(uint32_t mv);
//Mosfet driven switch control function
void MosfetSet(uint8_t mode);

// --- SYSTEM CLOCK CONFIG ---
void SystemClock_Config(void)
{
    LL_UTILS_PLLInitTypeDef sUTILS_PLLInitStruct = {
        LL_RCC_PLL_MUL_3,
        LL_RCC_PLL_DIV_3
    };
    LL_UTILS_ClkInitTypeDef sUTILS_ClkInitStruct = {
        LL_RCC_SYSCLK_DIV_1,
        LL_RCC_APB1_DIV_1,
        LL_RCC_APB2_DIV_1
    };

    LL_PLL_ConfigSystemClock_HSI(&sUTILS_PLLInitStruct, &sUTILS_ClkInitStruct);
    LL_Init1msTick(SystemCoreClock);
}

// --- UART2 INITIALIZATION ---
void UART2_Init(void)
{
    LL_GPIO_InitTypeDef  GPIO_InitStruct = {0};
    LL_USART_InitTypeDef USART_InitStruct = {0};

    // Enable clocks
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

    // PA2 (TX), PA3 (RX)
    GPIO_InitStruct.Mode        = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed       = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType  = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull        = LL_GPIO_PULL_NO;

    GPIO_InitStruct.Pin        = LL_GPIO_PIN_2;
    GPIO_InitStruct.Alternate  = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin        = LL_GPIO_PIN_3;
    GPIO_InitStruct.Alternate  = LL_GPIO_AF_7;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // UART params
    USART_InitStruct.BaudRate            = 115200;
    USART_InitStruct.DataWidth           = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits            = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity              = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    LL_USART_Init(USART2, &USART_InitStruct);

    // Enable directions BEFORE enabling USART
    LL_USART_EnableDirectionTx(USART2);
    LL_USART_EnableDirectionRx(USART2);

    LL_USART_Enable(USART2);
}

// --- UART helper: send zero-terminated string ---
void UART_SendString(const char *s)
{
    while (*s != '\0')
    {
        LL_USART_TransmitData8(USART2, (uint8_t)*s++);
        while (!LL_USART_IsActiveFlag_TXE(USART2))
        {
            // wait
        }
    }
}

// --- ADC init for Temp sensor on PB0 (ADC1 channel 8) ---
void ADC_Temp_Init(void)
{
    // Enable GPIOB and ADC1 clocks
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

    // PB0 as analog input
    LL_GPIO_InitTypeDef GPIO_InitStruct;
    LL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.Pin  = LL_GPIO_PIN_0;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Basic ADC configuration
    LL_ADC_InitTypeDef     ADC_InitStruct;
    LL_ADC_REG_InitTypeDef ADC_REG_InitStruct;

    LL_ADC_StructInit(&ADC_InitStruct);
    LL_ADC_REG_StructInit(&ADC_REG_InitStruct);

    ADC_InitStruct.Resolution    = LL_ADC_RESOLUTION_12B;
    ADC_InitStruct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
    LL_ADC_Init(ADC1, &ADC_InitStruct);

    ADC_REG_InitStruct.TriggerSource   = LL_ADC_REG_TRIG_SOFTWARE;
    ADC_REG_InitStruct.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE;
    ADC_REG_InitStruct.ContinuousMode  = LL_ADC_REG_CONV_SINGLE;
    ADC_REG_InitStruct.DMATransfer     = LL_ADC_REG_DMA_TRANSFER_NONE;
    LL_ADC_REG_Init(ADC1, &ADC_REG_InitStruct);

    // PB0 = ADC1_IN8, use as rank 1
    LL_ADC_REG_SetSequencerRanks(ADC1,
                                 LL_ADC_REG_RANK_1,
                                 LL_ADC_CHANNEL_8);

    // Sampling time
    LL_ADC_SetChannelSamplingTime(ADC1,
                                  LL_ADC_CHANNEL_8,
                                  LL_ADC_SAMPLINGTIME_16CYCLES);

    // Enable ADC
    LL_ADC_Enable(ADC1);
}

// --- One-shot ADC read on temp channel (PB0) ---
uint16_t ADC_Temp_ReadRaw(void)
{
    // Start conversion
    LL_ADC_REG_StartConversionSWStart(ADC1);

    // Wait for end of conversion
    while (!LL_ADC_IsActiveFlag_EOCS(ADC1))
    {
        // wait
    }

    // Clear flag
    LL_ADC_ClearFlag_EOCS(ADC1);

    // Read 12-bit result
    return (uint16_t)LL_ADC_REG_ReadConversionData12(ADC1);
}

// --- Convert mV to temperature in Â°C (integer) ---
// Mapping from your HW doc: 0.03 V -> -20 Â°C, 0.55 V -> 50 Â°C
// Use linear interpolation in integer math.
int16_t Temp_From_mV(uint32_t mv)
{
    // Clamp to sensor range 30â€“550 mV
    if (mv <= 30U)  return -20;
    if (mv >= 550U) return 50;

    int32_t num = ((int32_t)mv - 30) * 70; // 70 Â°C span
    int32_t t   = -20 + num / 520;         // 520 mV span
    return (int16_t)t;
}

void MosfetSet(uint8_t mode)
{
    char resp[32];
    switch(mode)
    {
        case 0: // Both OFF
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_8); // D15
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_9); // D14
	    
            // Integer Â°C, Robot will Convert To Number
            snprintf(resp, sizeof(resp), "Mosfet fully closed\r\n");
            UART_SendString(resp);

            break;

        case 1: // D14 ON, D15 OFF
            LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_9);   // D14 ON
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_8); // D15 OFF
            // Integer Â°C, Robot will Convert To Number
            snprintf(resp, sizeof(resp), "Mosfet charge\r\n");
            UART_SendString(resp);

            break;

        case 2: // D14 OFF, D15 ON
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_9); // D14 OFF
            LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_8);   // D15 ON
            // Integer Â°C, Robot will Convert To Number
            snprintf(resp, sizeof(resp), "Mosfet discharge\r\n");
            UART_SendString(resp);

            break;
        case 3:
            LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_9);   // D14 ON
            LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_8);   // D15 ON
            // Integer Â°C, Robot will Convert To Number
            snprintf(resp, sizeof(resp), "Mosfet fully open\r\n");
            UART_SendString(resp);
	    break;

        default: // Invalid value â†’ both OFF
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_8);
            LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_9);
	    snprintf(resp, sizeof(resp), "Invalid value Mosfet off\r\n");
            UART_SendString(resp);
       
            break;
    }
}

// --- MAIN ---
int main(void)
{
    SystemClock_Config();
    UART2_Init();
    ADC_Temp_Init();

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

    // Optional: LED on PA5
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_GPIO_InitTypeDef GPIO_InitStruct;
    LL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.Pin        = LL_GPIO_PIN_5;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    while (1)
    {
        // Read exactly 4 characters for "temp"
        char    buf[5] = {0};
        uint8_t index  = 0;

        while (index < 4)
        {
            if (LL_USART_IsActiveFlag_RXNE(USART2))
            {
                buf[index++] = LL_USART_ReceiveData8(USART2);
            }
        }

        // If command is "temp"
        if (strncmp(buf, "temp", 4) == 0)
        {
            uint16_t adc_raw = ADC_Temp_ReadRaw();

            // Convert ADC raw to millivolts (12-bit ADC, 3.3 V ref)
            uint32_t mv = (uint32_t)adc_raw * 3300U / 4095U;

            int16_t temp_c = Temp_From_mV(mv);

            char resp[32];
            // Integer Â°C, Robot will Convert To Number
            snprintf(resp, sizeof(resp), "%d\r\n", (int)temp_c);
            UART_SendString(resp);
        }
        //Mosfet commands 
	if  (strncmp(buf, "MfCl", 4) == 0){
            MosfetSet(0);
        }
	if  (strncmp(buf, "MfCh", 4) == 0){
            MosfetSet(1);
        }
        if  (strncmp(buf, "MfDc", 4) == 0){
            MosfetSet(2);
        }
        if  (strncmp(buf, "MfOp", 4) == 0){
            MosfetSet(3);
        }

        // else: ignore other commands for now
    }

    // Never reached
    return 0;
}
