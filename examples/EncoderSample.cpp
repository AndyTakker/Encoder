#include "Encoder.h"
#include <debug.h>

// Подключаем энкодер к пинам PC0 и PC1. Для смены направления сменить порядок пинов
Encoder encoder({GPIOC, GPIO_Pin_1, EXTI_Line1}, {GPIOC, GPIO_Pin_0, EXTI_Line0});

//==============================================================================
// Настройка прерываний для работы энекодера на PC0 и PC1
//------------------------------------------------------------------------------
void EXTI_INIT(void) {
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  EXTI_InitTypeDef EXTI_InitStructure = {0};
  NVIC_InitTypeDef NVIC_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC, ENABLE);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; // PC0, PC1 на вход
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // GPIOC ----> EXTI_Line0
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource0);
  EXTI_InitStructure.EXTI_Line = EXTI_Line0;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  // GPIOC ----> EXTI_Line1
  GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource1);
  EXTI_InitStructure.EXTI_Line = EXTI_Line1;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

#ifdef __cplusplus
extern "C" {
#endif
void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
//==============================================================================
// При срабатывании контактов энкодера вызывается функция и вычисляет новое
// положение энкодера.
// Сюда же можно добавить обрабочики прерываний с других входов.
//------------------------------------------------------------------------------
void EXTI7_0_IRQHandler(void) {
  encoder.refresh();  // Вызываем функцию обработки энкодера.
}
#ifdef __cplusplus
}
#endif

int main() {
  SystemCoreClockUpdate();
  Delay_Init();
  Delay_Ms(4000); // Ждем, пока раздуплится терминал в VSCode
  USART_Printf_Init(115200);
  printf("SystemClk: %lu\r\n", SystemCoreClock);      // Для посмотреть частоту процесора (48мГц)
  printf("   ChipID: %08lx\r\n", DBGMCU_GetCHIPID()); // Для посмотреть ID чипа, от нефиг делать

  EXTI_INIT(); // Настройка прерываний
  encoder.init();
  while (1) {
    printf("Encoder: %ld\r\n", encoder.encPos);
    Delay_Ms(500);
  }
}
