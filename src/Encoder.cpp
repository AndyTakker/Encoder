#include "Encoder.h"

#ifdef USE_GPIO_LIB
#include <gpio.h>
Encoder::Encoder(uint8_t pinDT, uint8_t pinCLK) {
  _pinDt = pinDT;
  _pinClk = pinCLK;
}
#else
Encoder::Encoder(ExtiCfg pinDT, ExtiCfg pinCLK) {
  _pinDt = pinDT;
  _pinClk = pinCLK;
}
#endif

void Encoder::init() {
  encPos = 0;
#ifdef USE_GPIO_LIB
  // Порты на вход
  PIN_input_PU(_pinDt);
  PORT_enable(_pinClk);
  PIN_input_PU(_pinDt);
  PORT_enable(_pinClk);
  // Настройка прерываний
  PIN_INT_set(_pinDt, PIN_INT_RISING);
  PIN_INT_set(_pinClk, PIN_INT_RISING);
  PIN_INT_enable();
#else
  portSetting(_pinDt);  // Порт на вход с подтяжкой и прерывание
  portSetting(_pinClk); // Порт на вход с подтяжкой и прерывание
#endif
}

#ifndef USE_GPIO_LIB
//==============================================================================
// Настройка одного пина энкодера - сам порт и прерывания от него.
//------------------------------------------------------------------------------
void Encoder::portSetting(ExtiCfg pin) {
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  EXTI_InitTypeDef EXTI_InitStructure = {0};
  NVIC_InitTypeDef NVIC_InitStructure = {0};

  // Получим номер порта для прерываний из номера порта (значения от 0 до 3)
  uint8_t portSource = (uint8_t)(((uint32_t)(pin.port) & 0x0000FFFF) >> 10) - 2;
  // Получим номер пина для прерывания из номера пина. Преобразуем битовую маску
  // 0х01, 0х02, 0х04, 0х08, 0x10, 0x20, 0x40, 0x80 в индекс (0, 1, 2, 3, 4, 5, 6, 7)
  uint8_t pinSource = __builtin_ctz(pin.pin);
  // Получим номер линии EXTI
  uint32_t extiLine = (uint32_t)pin.pin; // Номер линии EXTI совпадает с номером пина
  // Получим порт для RCC
  uint32_t perifPort = 4 << portSource;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | perifPort, ENABLE); // Включаем тактирование для порта

  // Настройка порта и пина
  GPIO_InitStructure.GPIO_Pin = pin.pin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // Порт на вход с подтяжкой к питанию
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
  GPIO_Init(pin.port, &GPIO_InitStructure);

  // Настройка прерываний от пина
  GPIO_EXTILineConfig(portSource, pinSource);
  EXTI_InitStructure.EXTI_Line = extiLine;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  
  // Разрешение прерываний
  NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
#endif

//==============================================================================
// Метод вызывается из прерывания по входам.
// Проверяем, что прерывание происходит от контактов энкодера и вычисляем
// новое положение.
//------------------------------------------------------------------------------
void Encoder::refresh() {
  static uint8_t lastState = 0;

#ifdef USE_GPIO_LIB
  if (PIN_INTFLAG_read(_pinDt) != RESET || PIN_INTFLAG_read(_pinClk) != RESET) {
    uint8_t pin0 = PIN_read(_pinDt);
    uint8_t pin1 = PIN_read(_pinClk);
#else
  uint32_t extiLineDt = (int32_t)_pinDt.pin;
  uint32_t extiLineClk = (int32_t)_pinClk.pin;

  if (EXTI_GetITStatus(extiLineDt) != RESET || EXTI_GetITStatus(extiLineClk) != RESET) {
    uint8_t pin0 = GPIO_ReadInputDataBit(_pinDt.port, _pinDt.pin);
    uint8_t pin1 = GPIO_ReadInputDataBit(_pinClk.port, _pinClk.pin);
#endif

    uint8_t state = pin0 | pin1 << 1;

#ifdef USE_GPIO_LIB
    PIN_INTFLAG_clear(_pinDt);
    PIN_INTFLAG_clear(_pinClk);
#else
    EXTI_ClearITPendingBit(extiLineDt | extiLineClk);
#endif
    if (lastState == 1 && state == 3) {
      encPos++;
    } else if (lastState == 3 && state == 1) {
      encPos--;
    }
    lastState = state;
  }
}
