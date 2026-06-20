#include "main.h"

void SystemClock_Config(void);

volatile uint8_t metronom_tick_requestet = 0;

uint16_t ADC_Read_Channel2(void)
{
  ADC1->CR2 |= ADC_CR2_SWSTART;

  while (!(ADC1->SR & ADC_SR_EOC))
  {
  }

  return (uint16_t)(ADC1->DR);
}

int main(void)
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
  SystemClock_Config();

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // GPIO Port A
  RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;  // Timer 2
  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; //adc1

  // PA1 alternate function 
  GPIOA->MODER &= ~GPIO_MODER_MODER1;
  GPIOA->MODER |= GPIO_MODER_MODER1_1;

  GPIOA->AFR[0] &= ~(0xF << 4);
  GPIOA->AFR[0] |= (1 << 4);

  // PA2 analog input
  GPIOA->MODER |= GPIO_MODER_MODER2;

  // PA0 input
  GPIOA->MODER &= ~GPIO_MODER_MODER0;

  //exti0 port a config
  SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0_PA;
  EXTI->RTSR |= EXTI_RTSR_TR0; //rising trigger
  EXTI->IMR |= EXTI_IMR_MR0; //interrupt mask
  NVIC_EnableIRQ(EXTI0_IRQn);

  // channel 2 start
  ADC1->SQR3 = 2;
  ADC1->CR2 |= ADC_CR2_ADON;

  TIM2->PSC = 84000 - 1;
  TIM2->ARR = 1000 - 1;

  TIM2->CCR2 = 50;
  TIM2->CCMR1 &= ~TIM_CCMR1_OC2M;
  TIM2->CCMR1 |= (TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE);
  TIM2->CCER |= TIM_CCER_CC2E;

  // enbale interrupt
  // TIM2->DIER |= TIM_DIER_UIE;
  // NVIC_EnableIRQ(TIM2_IRQn);

  // update & start
  TIM2->EGR |= TIM_EGR_UG;
  TIM2->CR1 |= TIM_CR1_CEN;

  uint16_t poti_wert = 0;

  while (1)
  {
    poti_wert = ADC_Read_Channel2();

    uint32_t bpm = 40 + ((poti_wert * 200) / 4095);
    uint32_t arr_wert = (60000 / bpm) - 1;
    TIM2->ARR = arr_wert;

    if (TIM2->CNT > arr_wert)
    {
      TIM2->EGR |= TIM_EGR_UG;
    }
  }
}

// void TIM2_IRQHandler(void)
// {
//   // test update interrupt flat und status register
//   if (TIM2->SR & TIM_SR_UIF)
//   {
//     // deactivate interrupt
//     TIM2->SR &= ~TIM_SR_UIF;

//     metronom_tick_requestet = 1;
//   }
// }

void EXTI0_IRQHandler(void) {
  //pending
  if(EXTI->PR & EXTI_PR_PR0) {
    EXTI->PR = EXTI_PR_PR0; //clear by writing 1

    TIM2->CR1 ^= TIM_CR1_CEN; //counter enable
  }
}

void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
  while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_5)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_HSE_Enable();

  /* Wait till HSE is ready */
  while (LL_RCC_HSE_IsReady() != 1)
  {
  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_4, 168, LL_RCC_PLLP_DIV_2);
  LL_RCC_PLL_Enable();

  /* Wait till PLL is ready */
  while (LL_RCC_PLL_IsReady() != 1)
  {
  }
  while (LL_PWR_IsActiveFlag_VOS() == 0)
  {
  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

  /* Wait till System clock is ready */
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  }
  LL_Init1msTick(168000000);
  LL_SetSystemCoreClock(168000000);
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
