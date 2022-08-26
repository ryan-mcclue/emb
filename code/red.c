// SPDX-License-Identifier: zlib-acknowledgement

/*
 * 1. Working HAL build:
 *   - Driver files documented with usage case sections so ctags; grep -nr; cscope
 *   First section will be overall, however may need to substitute with subsection information
 *   - Specify appropriate include folders.
 *   - Specify appropriate driver source files.
 *   - Specify appropriate startup, system and linker script files.
 *   - Macro define MCU being used in main header
 *
 * 2. GPIO:
 *   - Identify on-board LEDs and buttons with board schematic and pinout
 *   - Enable appropriate clock, e.g. \case-insensitive search for GPIOB inside of rcc driver
 *   - Enable appropriate gpio, e.g. \case-insensitive search for init inside of gpio driver
 *   - Read, set and reset appropriate bits
 *
 * 3. USART TX:
 *   - Identify USART: 
 *     1. Connected to virtual COM port via on-board FTDI chip 
 *     and build machine RS232 driver (/dev/ttyACMx)
 *     2. UART pins to connect USB-to-TTL serial cable (/dev/ttyUSBx)
 *     3. Flying-wires (solder from MCU pin to board pin)
 *   - Enable appropriate gpio pins to alternate function and apply usart function
 *   - Enable appropriate clock for usart peripheral
 *   - Configure usart peripheral (driver files may contain an InitStruct() with default values to use)
 *   - Activate usart module
 *   - Send data when usart data register flag is empty. View with serial monitor
 *   TODO(Ryan): prescaler affect clock speed? 
 *
 * 4. USART RX:
 *   - Identify appropriate usart interrupt to enable
 *   - Enable appropriate IRQ with NVIC via CMSIS files
 *   - Identify interrupt handler symbol to implement via startup file
 *   - Identify particular type of interrupt in interrupt handler
 *
 * 5. ADC: 
 *   - Identify appropriate ADC pin (A0 on pinout graphic, i.e exploded pin view, may map to PA3 in datasheet pinout description)
 *   ADC123_IN3 means that this pin can connect to any of the three ADC units on the board via input 3, i.e channel 3  
 *   TODO(Ryan): What is a three terminal potentiometer?
 *   graphical pinout, board datasheet, mcu datasheet (mcu feature overview), mcu reference manual (adc registers, etc.)
 *   - reference manual contains register mapping for adc
 *
 * 6. ADC with DMA:
 *   - 
 *
 * 7. Timers:
 *
 */

#include "stm32f4xx.h"


void 
button_led(void)
{
  // IMPORTANT(Ryan): Certain clock signals only go to particular ports, e.g. AHB1 and AHB2
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
  
  GPIO_InitTypeDef blue_led_gpio = {0};
  blue_led_gpio.GPIO_Pin = GPIO_Pin_7;
  blue_led_gpio.GPIO_Mode = GPIO_Mode_OUT;
  blue_led_gpio.GPIO_Speed = GPIO_Speed_2MHz;
  // push-pull and open-drain?
  blue_led_gpio.GPIO_OType = GPIO_OType_PP;
  blue_led_gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &blue_led_gpio);

  GPIO_InitTypeDef user_button_gpio = {0};
  user_button_gpio.GPIO_Pin = GPIO_Pin_13;
  user_button_gpio.GPIO_Mode = GPIO_Mode_IN;
  user_button_gpio.GPIO_Speed = GPIO_Speed_2MHz;
  user_button_gpio.GPIO_OType = GPIO_OType_PP;
  // down?
  user_button_gpio.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOC, &user_button_gpio);

  // TODO(Ryan): Read solder bridge resistor value with multimeter

  while (1)
  {
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13))
    {
      GPIO_SetBits(GPIOB, GPIO_Pin_7);
    }
    else
    {
      GPIO_ResetBits(GPIOB, GPIO_Pin_7);
    }
  }
}

void
usart(void)
{
  // TODO(Ryan): Difference between synchronous and asynchronous
  // usart can be synchronous also, i.e clocked

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

  GPIO_InitTypeDef usart_gpio = {0};
  usart_gpio.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
  usart_gpio.GPIO_Mode = GPIO_Mode_AF;
  usart_gpio.GPIO_Speed = GPIO_Speed_50MHz;
  usart_gpio.GPIO_OType = GPIO_OType_PP;
  usart_gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

  GPIO_Init(GPIOD, &usart_gpio);

  GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);

  USART_InitTypeDef usart_init = {0};
  // IMPORTANT(Ryan): Even though we have specified 9600, the input PLLs and dividers may not be correct,
  // giving peripheral clock speed lower than what they should be
  // TODO(Ryan): Inside of system file, 
  // systemclock different to HSE frequency (input crystal?)
  // Ensure that master PLL divider is same as input crystal frequency
  usart_init.USART_BaudRate = 9600;
  usart_init.USART_WordLength = USART_WordLength_8b;
  usart_init.USART_StopBits = USART_StopBits_1;
  usart_init.USART_Parity = USART_Parity_No;
  usart_init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
  // NOTE(Ryan): These various parameters combine together to form UART protocol,
  // e.g 8NI, (we actually send a start bit as well, so for 8 bits of data, 10 bits transmitted)
   
  USART_Init(USART3, &usart_init);
  USART_Cmd(USART3, ENABLE);

  // often interrupts and exceptions used interchangeably
  // In ARM, an interrupt is a type of exception (changing normal flow of program)
  // Exception number (offset into vector table, i.e. exception handler), priority level,
  // synchronous/asynchronous, state (pending, active, inactive)
  // Index 0 of vector table is reset value of stack pointer (rest exception handlers) 
  // On Cortex-M, 6 exceptions always supported: reset, nmi, hardfault, SVCall, PendSV, SysTick 
  // External interrupts start from 16 and are configured via NVIC (specifically registers within it)
  // Will have to first enable device to generate the interrupt, then set NVIC accordingly
  // The startup file will define the interrupt handlers names to which we should define
  // Typically an NVIC interrupt handler will have 'sub interrupts' which we can determine with ITStatus()
  //USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
  //NVIC_EnableIRQ(USART3_IRQn); // cmsis m4


  while (1)
  {
    while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) != SET) {}
    // (num % 10) + '0';
    USART_SendData(USART3, 80);
    for (unsigned long i = 0; i < 10000000; i++) {}
  }
}

void USART3_IRQHandler(void) // cmsis startup (a weak symbol)
{
  if (USART_GetITStatus(USART3, USART_IT_RXNE))
  {
    // arduino serial smart enough to send data via an interrupt?
    if (USART_ReceiveData(USART3) == 'K')
    {
      // GPIO_ToggleBits(GPIOB, GPIO_Pin_7);
      // \n is two bytes, so send control sequence 0xa
      USART_SendData(USART3, 70);
    }
  }
}


volatile uint16_t adc_data[2];

void
dma_adc(void)
{
  // pa3, input 3
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);   

  GPIO_InitTypeDef adc_gpio = {0};
  adc_gpio.GPIO_Pin = GPIO_Pin_3;
  adc_gpio.GPIO_Mode = GPIO_Mode_AN;
  adc_gpio.GPIO_Speed = GPIO_Speed_50MHz;
  adc_gpio.GPIO_OType = GPIO_OType_PP;
  adc_gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &adc_gpio);

  // global adc config
  ADC_CommonInitTypeDef adc_common_init = {0};
  adc_common_init.ADC_Mode = ADC_Mode_Independent;
  // IMPORTANT(Ryan): Relate this to the clock speed of the bus configured with RCC
  // So, if not working, perhaps have to increase this value as too fast
  adc_common_init.ADC_Prescaler = ADC_Prescaler_Div2; // lowest, so fastest
  adc_common_init.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  adc_common_init.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&adc_common_init);

  // specific adc config
  ADC_InitTypeDef adc_init = {0};
  adc_init.ADC_Resolution = ADC_Resolution_12b;
  // We are switching between multiple channels
  adc_init.ADC_ScanConvMode = ENABLE;
  // We are not concerned with energy-consumption, so keep converting
  adc_init.ADC_ContinuousConvMode = ENABLE;
  adc_init.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
  adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
  adc_init.ADC_DataAlign = ADC_DataAlign_Right;
  adc_init.ADC_NbrOfConversion = 2;
  ADC_Init(ADC1, &adc_init);

  // NOTE(Ryan): Rank is the order in which channels are processed on the ADC
  // The sample time here is acquistion time
  ADC_RegularChannelConfig(ADC1, ADC_Channel3, 1, ADC_SampleTime_144Cycles);
  // temp sensor
  ADC_RegularChannelConfig(ADC1, ADC_Channel18, 2, ADC_SampleTime_144Cycles);
  // NOTE(Ryan): Any onboard peripheral will require some init
  ADC_TempSensorVrefintCmd(ENABLE);

  // Enable DMA and generate DMA request when data transfered
  ADC_DMACmd(ADC1, ENABLE);
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);

  ADC_Cmd(ADC1, ENABLE);
  // these functions are broken up to save battery
  ADC_SoftwareStartConv(ADC1);

  // as different peripherals run at different clocks, when getting data often have to wait to ensure getting the most recent
  // often, this will be obtained by searching for 'flags' in driver file
  // however, better to use interrupts for efficiency
  while (1)
  {
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)) {}
    uint16_t adc_data = ADC_GetConversionValue(ADC1);
    float volts_read = (adc_data / ((1 << 12) - 1)) * 5.0f;
    /* temp sensor reading:
     * volts = (adc_data[1] / ((1 << 12) - 1)) * 3.3f;
     * (NOTE: ADCs typically have their own reference voltage to filter out noise)
     * volts -= 0.76; 
     * float celsius = volts / 0.0025f;
     * celsius += 25; (this may not have to be added?)
     */
  }

  // DMA has streams (where data comes in). Channels feed into streams
  // identify the channel, stream and DMA controller number for desired peripheral 
  // (found in reference manual)
  // identify where inside the peripheral we are actually reading data from (most likely a data register)

  // channel 0, stream 0, DMA2 
  RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA2, ENABLE);

  DMA_InitTypeDef adc_dma_init = {0};
  adc_dma_init.DMA_Channel = 0;
  adc_dma_init.DMA_PeripheralBaseAddr = &ADC1->DR;
  adc_dma_init.DMA_Memory0BaseAddr = adc_data;
  adc_dma_init.DMA_DIR = DMA_DIR_PeripheralToMemory;
  // NOTE(Ryan): We are going to copy 2 data units
  adc_dma_init.DMA_BufferSize = 2;
  adc_dma_init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  adc_dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
  adc_dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  adc_dma_init.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  // NOTE(Ryan): Continually copy data over without explicit request
  adc_dma_init.DMA_Mode = DMA_Mode_Circular;
  adc_dma_init.DMA_Priority = DMA_Priority_Medium;
  adc_dma_init.DMA_FIFOMode = DMA_FIFOMode_Disable;
  adc_dma_init.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
  adc_dma_init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  adc_dma_init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  // NOTE(Ryan): Most peripherals are configured then started
  DMA_Init(DMA2_Stream0, &adc_dma_init);
  DMA_Cmd(DMA2_Stream0, ENABLE);
}

// TODO(Ryan): Have file directories to open for later inspection, e.g. driver, system, startup, CMSIS, etc.
// ~/prog/hals/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/STM32F4xx_StdPeriph_Driver/src
// ~/prog/hals/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c

void
timer_output_compare(void)
{
  // init timer gpio pins as AF and apply AF timer config
  
  // NOTE(Ryan): Create a 1 second pulse
  timer_init.TIM_Prescaler = 8399; // 84MHz, so 8400 - 1
  timer_init.TIM_Period = 9999; // divide by 10000, so 10000 - 1 
  // The period would affect the resolution for PWM, the prescaler the frequency of PWM

  TIM_OCInitTypeDef oc_init = {0};
  oc_init.TIM_OCMode = TIM_OCMode_Toggle;
  oc_init.TIM_OutputState = TIM_OutputState_Enable;
  oc_init.TIM_OCPolarity = TIM_OCPolarity_High;
  oc_init.TIM_OCIdleState = TIM_OCIdleState_Reset;
  // This is the capture compare register value, so trigger every time hits 0
  oc_init.TIM_Pulse = 0;

  // NOTE(Ryan): Timer channels relevent for compare register
  TIM_OC1Init(TIM4, &oc_init);
  oc_init.TIM_Pulse = 5000;
  TIM_OC2Init(TIM4, &oc_init);

  TIM_Cmd(TIM4, ENABLE);

  // TODO(Ryan): PWM require enabling a 'shadow register' for compare register?  
  // It holds a copy of it in case of it being overwritten?
  TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
  // IMPORTANT(Ryan): For PWM, we require higher frequency, e.g. 1kHz so it appears silky smooth  
  // We relate this value to the period to obtain duty cycle.
  // This is half period, so a 50/50, 50% duty cycle
  // A lower value hear gives a lower duty cycle, giving a dimmer light
  // period/pulse = %duty_cycle
  oc_init.TIM_OCMode = TIM_OCMode_PWM1;
  oc_init.TIM_Pulse = 4999;
  // IMPORTANT(Ryan): A way to avoid having to pass init structure across functions, e.g.
  // TIM_TimeBaseInitTypeDef, just look at what register it affects and set that,
  // e.g. TIM_Pulse == CCR1, so TIM4->CCR1 = value;
}

int
main(void)
{
  // A GPIO AF is timer

  // RCC sets clocks and PLLs from information in system file? 
  RCC_ClocksTypeDef clock_info = {0};
  // NOTE(Ryan): hclock is AHB, pclock APB
  RCC_GetClocksFreq(&clock_info); 
  
  // TODO(Ryan): How does this come about to be every 1us?
  // This is the system timer, as oppose to general purpose timers
  SysTick_Config(SystemCoreClock / 1000000);
  // And associated SysTick_Handler(void) interrupt function
  
  // IMPORTANT(Ryan): Often it is easy to obtain specific information from BSP files
  // system_file.c contains clock information, e.g. oscillator frequency.
  // Use this information for applying numbers to the clock tree diagram
  // reference manual contains clock tree.
  // HSE -> pll multipliers and dividers -> system clock
  // system clock -> prescalers (dividers) -> bus clocks
  // After interpreting this, we create a timer on a specific bus
  // Identify general purpose timer peripheral we want to use from datasheet
  // Timers use counters which compare value in an 'auto-reload register' to overflow
  // They generate an output pulse on overflow
  // Also have capture/compare registers that can be used trigger on counter values in-between range
  // Say we want an output frequency of 1Hz, we play with prescaler and counter values that fit within the 16 bits provided in relation to the input clock
  // E.g. 42MHz in, divide by 42000 (in range of 16bit) gives 1000Hz. Then count to 1000
  // A formula for calculating this will often be given (update event period)
  
  // TODO(Ryan): List clock speeds for each bus
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  TIM_TimeBaseInitTypeDef timer3_init = {0};
  // NOTE(Ryan): These first two values, prescaler and period are typically what changes
  timer3_init.TIM_Prescaler = 0x0000;
  // Amount to count
  timer3_init.TIM_Period = 83;
  timer3_init.TIM_ClockDivision = TIM_CKD_DIV1;
  timer3_init.TIM_CounterMode = TIM_CounterMode_Up;
  timer3_init.TIM_RepetitionCounter = 0x0000;
  TIM_TimeBaseInit(TIM3, &timer3_init); 

  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  NVIC_Enable(TIM3_IRQn);

  return 0;
}

void
TIM3_IRQHandler(void)
{
  if (TIM_GetFlagStatus(TIM3, TIM_FLAG_Update))
  {
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);
    ++n_ticks;
  }
}

void
delay_ms(int ms)
{
  n_ticks = 0;
  TIM_Cmd(TIM3, ENABLE);
  // IMPORTANT(Ryan): For optimisations, most BSPs will have a NOP
  while (n_ticks < 1000) {}
  TIM_Cmd(TIM3, DISABLE);
}
