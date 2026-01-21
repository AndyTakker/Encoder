#include "Encoder.h"
#include <debug.h>

Encoder::Encoder(ExtiCfg pinDT, ExtiCfg pinCLK) {
  _pinDt = pinDT;
  _pinClk = pinCLK;
}

void Encoder::init() {
  encPos = 0;
}

//==============================================================================
// Метод вызывается из прерывания по входам.
// Проверяем, что прерываение происходит от контактов энкодера и вычисляем
// новое положение. 
//------------------------------------------------------------------------------
void Encoder::refresh() {
  static uint8_t lastState = 0;

  if (EXTI_GetITStatus(_pinDt.extiLine) != RESET || EXTI_GetITStatus(_pinClk.extiLine) != RESET) {
    uint8_t pin0 = GPIO_ReadInputDataBit(_pinDt.port, _pinDt.pin);
    uint8_t pin1 = GPIO_ReadInputDataBit(_pinClk.port, _pinClk.pin);
    uint8_t state = pin0 | pin1 << 1;
    EXTI_ClearITPendingBit(_pinDt.extiLine | _pinClk.extiLine);

    if (lastState == 1 && state == 3) {
      encPos++;
    } else if (lastState == 3 && state == 1) {
      encPos--;
    }
    lastState = state;
  }
}
