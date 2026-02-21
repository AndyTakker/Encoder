//============================================================= (c) A.Kolesov ==
// Пример использования библиотеки энкодера
//------------------------------------------------------------------------------
#include <Encoder.h>
#include <Logs.h>
#include <SysClock.h>
#include <debug.h>

// Подключаем энкодер. Для смены направления сменить порядок пинов
// Encoder encoder(PC4, PC3);                  // Если нужно только положение энкодера
Encoder encoder(PC4, PC3, Sysclock.Millis); // Если нужно мерять скорость вращения

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
  USART_Printf_Init(115200);
  logs("SystemClk: %lu\r\n", SystemCoreClock);        // Для посмотреть частоту процесора (48мГц)
  logs("   ChipID: 0x%08lX\r\n", DBGMCU_GetCHIPID()); // Для посмотреть ID чипа, от нефиг делать
#endif

  while (1) {
#ifdef LOG_ENABLE
    logs("Encoder: %ld, Speed: %ld \r\n", encoder.encPos, encoder.getSpeed());
    delay(200);
#endif
  }
}
