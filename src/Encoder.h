#ifndef ENCODER_H
#define ENCODER_H
#include <ch32v00x.h>

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint32_t extiLine;
} ExtiCfg;

class Encoder {
private: 
    ExtiCfg _pinDt;
    ExtiCfg _pinClk;

public: 
    volatile int32_t encPos = 0;     // Положение энкодера. За один полный оборот получаем 80 импульсов
    Encoder(ExtiCfg pinDT, ExtiCfg pinCLK);
    void init(void);
    void refresh();
};

#endif // ENCODER_H
