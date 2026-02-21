#include <Encoder.h>

Encoder::Encoder(PinName pinDT, PinName pinCLK, uint32_t (*timeFunc)()) {
  _pinDt = pinDT;
  _pinClk = pinCLK;
  _timeFunc = timeFunc; // Функция измерения времени
  encPos = 0;
  // pinMode(_pinDt, GPIO_Mode_IN_FLOATING); // Для энкодера с конденсаторами
  // pinMode(_pinClk, GPIO_Mode_IN_FLOATING);
  pinMode(_pinDt, GPIO_Mode_IPU); // Для голого энкодера (с кондерами тоже работает)
  pinMode(_pinClk, GPIO_Mode_IPU);
  pinExtiInit(_pinDt, EXTI_Trigger_Falling); // Для энкодера с высоким уровнем в состоянии покоя
  pinExtiInit(_pinClk, EXTI_Trigger_Falling);
  // pinExtiInit(_pinDt);  // Для энкодеров с низким уровнем в состоянии покоя
  // pinExtiInit(_pinClk);
}

//==============================================================================
// Метод вызывается из прерывания по входам.
// Проверяем, что прерывание происходит от контактов энкодера и вычисляем
// новое положение энкодера. Одновременно обновляем скорость вращения.
//------------------------------------------------------------------------------
void Encoder::refresh() {
  static uint8_t lastState = 0;
  uint8_t pin0, pin1;
  int32_t delta = 0;

  ITStatus stDt = EXTI_GetITStatus(extiLine(_pinDt));
  ITStatus stClk = EXTI_GetITStatus(extiLine(_pinClk));

  if (stDt == RESET && stClk == RESET) { // Прерывание не от наших пинов
    return;
  }
  pin0 = pinRead(_pinDt);
  pin1 = pinRead(_pinClk);
  uint8_t state = pin0 | pin1 << 1;

  delta = 0;
  // Логика для энкодера E4High
  if (state == 0b00) { // Состояние в котором нужно делать инкремент/декремент
    if (lastState == 0b01) {
      delta = -1;
    } else if (lastState == 0b10) {
      delta = 1;
    }
  }

  lastState = state;

  // Обновляем позицию и скорость только при изменении
  if (delta != 0) {
    encPos += delta;

    if (_timeFunc) {
      uint32_t now = _timeFunc();
      uint32_t dt = now - _lastTime;

      int32_t dir = delta > 0 ? 1 : (delta < 0 ? -1 : 0); // Направление
      if (_lastDirection != dir) {                        // Если направление изменилось
        _speed = 0;                                       // Скорость - ноль
        dt = 0;                                           // Время - ноль
      }
      _lastDirection = dir;

      if (dt == 0 || dt > SPEED_MEASUREMENT_INTERVAL) {
        _speed = 0;        // Не крутили энкодер дольше заданного времени или крутнули как-то нереально быстро
      } else if (dt > 0) { // Защита от деления на ноль
        // Скорость в шагах/мс с экспоненциальным сглаживанием (альфа = 0.3)
        int32_t instantSpeed = (delta * 1000L) / (int32_t)dt;           // шагов в 1ms
        int32_t alpha = 3;                                              // 0.3
        _speed = (_speed * (10L - alpha) + instantSpeed * alpha) / 10L; // Сглаживаниеs
      }
      _lastTime = now;
      _lastPos = encPos;
    }
  }

  // Чистим флаги прерывания от пинов энкодера в самом конце, т.е. после того,
  // как обработаем текущее вращение.
  if (stDt != RESET) {
    EXTI_ClearITPendingBit(extiLine(_pinDt));
  }
  if (stClk != RESET) {
    EXTI_ClearITPendingBit(extiLine(_pinClk));
  }
}

//==============================================================================
// Получить текущую скорость вращения (шагов/секунду)
// Положительное значение — по часовой, отрицательное — против
//------------------------------------------------------------------------------
int32_t Encoder::getSpeed() const {
  if (!_timeFunc)
    return 0;

  uint32_t elapsed = _timeFunc() - _lastTime;
  // Если энкодер не двигался дольше N мс — считаем скорость = 0
  return (elapsed < FORGETTING_INTERVAL) ? _speed : 0;
}

//==============================================================================
// Сбросить скорость (например, при остановке)
//------------------------------------------------------------------------------
void Encoder::resetSpeed() {
  _speed = 0;
  _lastTime = (_timeFunc) ? _timeFunc() : 0;
}
