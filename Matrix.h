#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "Font5x7.h"

#define MATRIX_LED_PIN 17
#define MATRIX_LED_COUNT 100
#define MATRIX_BRIGHTNESS 255
#define SDA1 4
#define SCL1 14
#define SDA2 32
#define SCL2 33
#define DEBOUNCE_MS 35

enum MatrixEffectType { MATRIX_EFFECT_NONE, MATRIX_EFFECT_TEXT_CENTER, MATRIX_EFFECT_TEXT_SCROLL, MATRIX_EFFECT_WINNER, MATRIX_EFFECT_LOOSER };

class Matrix {
public:
    void begin(); void loop(); void clear(); MatrixEffectType getEffectType(); uint32_t color(uint8_t r,uint8_t g,uint8_t b);
    void setPixel(uint8_t row,uint8_t column,uint32_t color,bool startTimer=true); void setMatrix(uint32_t color); void setRow(uint8_t row,uint32_t color); void setColumn(uint8_t column,uint32_t color);
    void setEffect(String name,unsigned long interval,uint32_t color=0,String text="");
    bool hasEvent(); void clearEvent(); uint8_t getPressedRow(); uint8_t getPressedColumn(); unsigned long getReactionTime();
private:
    Adafruit_NeoPixel pixels=Adafruit_NeoPixel(MATRIX_LED_COUNT,MATRIX_LED_PIN,NEO_RGB+NEO_KHZ800);
    MatrixEffectType effectType=MATRIX_EFFECT_NONE; MatrixEffectType parseEffectType(String name);
    unsigned long effectInterval=80,effectLastMillis=0,lastShowMillis=0; uint32_t effectColor=0; uint8_t effectCounter=0; bool effectState=false;
    String effectText=""; int16_t effectTextOffset=10; uint8_t effectTextCharIndex=0;
    TwoWire I2C_1=TwoWire(0),I2C_2=TwoWire(1);
    uint8_t pcfAddress[10]={0x24,0x23,0x22,0x21,0x20,0x24,0x23,0x22,0x21,0x20};
    uint8_t buttonPins[10]={3,4,5,6,7,8,9,10,11,12};
    bool pressEvent=false,debouncedButtonState[10][10]={false},lastButtonState[10][10]={false};
    unsigned long lastButtonChangeMillis[10][10]={0},pixelActivatedMillis[10][10]={0}; uint8_t pressedRow=0,pressedColumn=0; unsigned long reactionTime=0;
    void updateEffect(); void updateTextCenterEffect(); void updateTextScrollEffect(); void updateWinnerEffect(); void updateLooserEffect(); void checkPixels();
    void setDot(uint8_t row,uint8_t column,uint32_t color); void fill(uint32_t color); uint16_t getLedIndex(uint8_t row,uint8_t column); bool getTextPixel(String text,int16_t x,uint8_t y); uint32_t wheel(uint8_t position);
    void pcfWrite(uint8_t row,uint16_t portValue); uint16_t pcfRead(uint8_t row);
};
