//============================================================= (c) A.Kolesov ==
// Пример использования библиотеки энкодера
//------------------------------------------------------------------------------
#include "Encoder.h"
#include <debug.h>

// Подключаем энкодер. Для смены направления сменить порядок пинов
Encoder encoder(PC4, PC3);

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// При срабатывании контактов энкодера вызывается функция и вычисляет новое
// положение энкодера.
// Сюда же можно добавить обработчики прерываний с других входов.
//------------------------------------------------------------------------------
void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void) {
  encoder.refresh(); // Вызываем функцию обработки энкодера.
}

#ifdef __cplusplus
}
#endif

int main() {
  SystemCoreClockUpdate();

#ifdef LOG_ENABLE
  Delay_Init();
  USART_Printf_Init(115200);
  printf("SystemClk: %lu\r\n", SystemCoreClock);      // Для посмотреть частоту процесора (48мГц)
  printf("   ChipID: %08lx\r\n", DBGMCU_GetCHIPID()); // Для посмотреть ID чипа, от нефиг делать
#endif

  // encoder.init(); // Настройка портов энкодера и прерываний и пр.
  while (1) {
#ifdef LOG_ENABLE
    printf("Encoder: %ld\r\n", encoder.encPos);
    Delay_Ms(500);
#endif
  }
}
