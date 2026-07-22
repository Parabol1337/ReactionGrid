#pragma once
#include <Arduino.h>
#include "Matrix.h"
#include "Border.h"
class SelfTest {
public:
    void begin(Matrix* matrix, Border* border);
    void loop(bool active);
private:
    Matrix* matrix=nullptr;
    Border* border=nullptr;
    bool lastActive=false;
    uint8_t state[10][10]={0};
    void start(); void stop(); void clearAll(); void handlePress();
};
