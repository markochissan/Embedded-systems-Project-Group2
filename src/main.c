#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "stm32l1xx.h"
#include "stm32l1xx_conf.h"
#include "stm32l1xx_ll_utils.h"
#include "stm32l1xx_ll_gpio.h"
#include "stm32l1xx_ll_usart.h"
#include "stm32l1xx_ll_adc.h"

void     SystemClock_Config(void);
void     UART2_Init(void);
void     ADC_Init_All(void);
uint16_t ADC_Temp_ReadRaw(void);
uint16_t ADC_Current_ReadRaw(void);
void     UART_SendString(const char *s);
float    ntc_temp_c_from_adc(uint16_t adc);
uint32_t ADC_To_mV(uint16_t adc);
int16_t  Current_From_mV(uint32_t mv);

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

void UART2_Init(void)
{
    LL_GPIO_InitTypeDef  GPIO_InitStruct = {0};
    LL_USART_InitTypeDef USART_InitStruct = {0};

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

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

    USART_InitStruct.BaudRate            = 115200;
    USART_InitStruct.DataWidth           = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits            = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity              = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    LL_USART_Init(USART2, &USART_InitStruct);

    LL_USART_EnableDirectionTx(USART2);
    LL_USART_EnableDirectionRx(USART2);
    LL_USART_Enable(USART2);
}

void UART_SendString(const char *s)
{
    while (*s != '\0')
    {
        while (!LL_USART_IsActiveFlag_TXE(USART2))
        {
        }
        LL_USART_TransmitData8(USART2, (uint8_t)*s++);
    }

    while (!LL_USART_IsActiveFlag_TC(USART2))
    {
    }
}

void ADC_Init_All(void)
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);

    LL_GPIO_InitTypeDef GPIO_InitStruct;
    LL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.Pin  = LL_GPIO_PIN_0;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    LL_GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.Pin  = LL_GPIO_PIN_1;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

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

    LL_ADC_Enable(ADC1);
}

uint16_t ADC_Temp_ReadRaw(void)
{
    LL_ADC_REG_SetSequencerRanks(ADC1,
                                 LL_ADC_REG_RANK_1,
                                 LL_ADC_CHANNEL_8);
    LL_ADC_SetChannelSamplingTime(ADC1,
                                  LL_ADC_CHANNEL_8,
                                  LL_ADC_SAMPLINGTIME_16CYCLES);

    LL_ADC_REG_StartConversionSWStart(ADC1);

    while (!LL_ADC_IsActiveFlag_EOCS(ADC1))
    {
    }

    LL_ADC_ClearFlag_EOCS(ADC1);
    return (uint16_t)LL_ADC_REG_ReadConversionData12(ADC1);
}

uint16_t ADC_Current_ReadRaw(void)
{
    LL_ADC_REG_SetSequencerRanks(ADC1,
                                 LL_ADC_REG_RANK_1,
                                 LL_ADC_CHANNEL_1);
    LL_ADC_SetChannelSamplingTime(ADC1,
                                  LL_ADC_CHANNEL_1,
                                  LL_ADC_SAMPLINGTIME_16CYCLES);

    LL_ADC_REG_StartConversionSWStart(ADC1);

    while (!LL_ADC_IsActiveFlag_EOCS(ADC1))
    {
    }

    LL_ADC_ClearFlag_EOCS(ADC1);
    return (uint16_t)LL_ADC_REG_ReadConversionData12(ADC1);
}


float ntc_temp_c_from_adc(uint16_t adc)
{
    const float T0 = 298.15f;   
    const float R0 = 100.0f;    
    const float B  = 3560.0f;   
    const float Rs = 200.0f;    

    if (adc == 0)
        adc = 1;
    if (adc >= 4095)
        adc = 4094;

    float r_ntc = Rs * (float)adc / (4095.0f - (float)adc);

    float inv_T = (1.0f / T0) + (1.0f / B) * logf(r_ntc / R0);
    float T     = 1.0f / inv_T;  

    return T - 273.15f;           
}

uint32_t ADC_To_mV(uint16_t adc)
{
    return (uint32_t)adc * 3300u / 4095u;
}


int16_t Current_From_mV(uint32_t mv)
{
    if (mv <= 1050u)
        return -10;
    if (mv >= 2250u)
        return 10;

    int32_t num = ((int32_t)mv - 1050) * 20; 
    int32_t i   = -10 + num / 1200;         

    return (int16_t)i;
}

int main(void)
{
    SystemClock_Config();
    UART2_Init();
    ADC_Init_All();

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
        uint16_t adc_temp   = ADC_Temp_ReadRaw();
        float    temp_c_raw = ntc_temp_c_from_adc(adc_temp);

        float temp_c_scaled = temp_c_raw / 2.0f;
        int   temp_int      = (int)(temp_c_scaled + 0.5f);   // round

        uint16_t adc_curr = ADC_Current_ReadRaw();
        uint32_t mv_curr  = ADC_To_mV(adc_curr);
        int16_t  curr_a   = Current_From_mV(mv_curr);        // -10 .. 10 A

        char out[32];
        snprintf(out, sizeof(out), "%d\r\n", temp_int);
        UART_SendString(out);

        char out2[32];
        snprintf(out2, sizeof(out2), "I=%d\r\n", (int)curr_a);
        UART_SendString(out2);

        LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_5);

        LL_mDelay(2000);
    }

}
