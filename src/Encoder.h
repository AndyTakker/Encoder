//==============================================================================
// Библиотека для работы с энкодером на CH32V003.
// Минималистический вариант, только вращения. Никак не учитывает нажатие кнопки
// (т.к. от просто кнопка) и не отслеживает скорость вращения (это несложно 
// реализовать в прикладном коде). 
// Реализована в двух вариантах:
// 1. С использованием "стандартных" функций Noneos-sdk (в стиле STM32)
// 2. С использованием библиотеки gpio.h
// Выбор варианта осуществляется флагом USE_GPIO_LIB.
// Преимущества использования библиотеки gpio:
// - код получается немного меньше. Экономия flash около 300 байт.
// - позволяет обращаться к пинам по алиасам PD0, PD1, PD2, PD3, PD4, PD5, PD6 и т.д.
//   что упрощает конфигурирование пинов через #define.
// К минусам стоит отнести то, что вся gpio построена на дефайнах и нагладностью
// кода не отличается. Но включив build_flag = -save-temps можно потом посмотреть
// в .ii файлах фактически получившийся код.
//
// Для использования библиотеки энкодера нужно:
// 1. Создать объект encoder
// 2. Создать обрабочик прерывания EXTI7_0_IRQHandler и в нем вызвать encoder.refresh()
//    Если обработчик уже есть, то просто добавить в него вызов encoder.refresh()
// 3. В начале main вызвать encoder.init()
// 4. Положение энкодера доступно через encoder.encPos
// Смотри пример в examples/EncoderSample.cpp
//------------------------------------------------------------------------------
#pragma once
// #define USE_GPIO_LIB // Подключить библиотеку GPIO через флаг USE_GPIO_LIB в platformio.ini
#include <ch32v00x.h>

#ifndef USE_GPIO_LIB
typedef struct {
  GPIO_TypeDef *port;
  uint16_t pin;
} ExtiCfg;
#endif

class Encoder {
  private:
#ifdef USE_GPIO_LIB
  uint8_t _pinDt;
  uint8_t _pinClk;  
#else
  ExtiCfg _pinDt;
  ExtiCfg _pinClk;
  void portSetting(ExtiCfg pin);
#endif

  public:
  volatile int32_t encPos = 0; // Положение энкодера
#ifdef USE_GPIO_LIB
  Encoder(uint8_t pinDT, uint8_t pinCLK);
#else  
  Encoder(ExtiCfg pinDT, ExtiCfg pinCLK);
#endif  
  void init(void);
  void refresh();
};
