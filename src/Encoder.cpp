#include "Encoder.h"

Encoder::Encoder(PinName pinDT, PinName pinCLK) {
  _pinDt = pinDT;
  _pinClk = pinCLK;
  encPos = 0;
  pinMode(_pinDt, GPIO_Mode_IPU);
  pinMode(_pinClk, GPIO_Mode_IPU);
  pinExtiInit(_pinDt);
  pinExtiInit(_pinClk);
}

//==============================================================================
// Метод вызывается из прерывания по входам.
// Проверяем, что прерывание происходит от контактов энкодера и вычисляем
// новое положение.
//------------------------------------------------------------------------------
void Encoder::refresh() {
  static uint8_t lastState = 0;
  uint8_t pin0 = 0, pin1 = 0;

  if (EXTI_GetITStatus(extiLine(_pinDt)) != RESET || EXTI_GetITStatus(extiLine(_pinClk)) != RESET) {
    pin0 = pinRead(_pinDt);
    pin1 = pinRead(_pinClk);
  }
  EXTI_ClearITPendingBit(extiLine(_pinDt) | extiLine(_pinClk));

  uint8_t state = pin0 | pin1 << 1;

  if (lastState == 1 && state == 3) {
    encPos++;
  } else if (lastState == 3 && state == 1) {
    encPos--;
  }
  lastState = state;
}
