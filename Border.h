#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define BORDER_LED_PIN        5
#define BORDER_LED_COUNT      292
#define BORDER_BRIGHTNESS     80
#define BORDER_SEGMENT_COUNT  20

enum BorderEffectType {
    BORDER_EFFECT_NONE,
    BORDER_EFFECT_RAINBOW,
    BORDER_EFFECT_FADE,
    BORDER_EFFECT_FADEUP,
    BORDER_EFFECT_FADEDOWN,
    BORDER_EFFECT_FLASH,
    BORDER_EFFECT_LOADER
};

struct SegmentRange {
    uint16_t start;
    uint16_t end;
};

const SegmentRange borderSegments[BORDER_SEGMENT_COUNT] = {
    {58, 72},   {44, 57},   {29, 43},   {15, 28},   {0, 14},
    {277, 291}, {263, 276}, {248, 262}, {234, 247}, {219, 233},
    {204, 218}, {190, 203}, {175, 189}, {161, 174}, {146, 160},
    {131, 145}, {117, 130}, {102, 116}, {88, 101},  {73, 87}
};

class Border {
public:
    void begin();
    void loop();
    void clear();

    BorderEffectType getEffectType();

    uint32_t color(uint8_t r, uint8_t g, uint8_t b);

    void setBorder(uint32_t color);
    void setSegment(uint8_t segment, uint32_t color);
    void setEffect(String name, unsigned long interval, uint32_t color = 0);
    void setCounter(uint8_t percent, uint32_t color);

private:
    Adafruit_NeoPixel pixels = Adafruit_NeoPixel(
        BORDER_LED_COUNT,
        BORDER_LED_PIN,
        NEO_GRB + NEO_KHZ800
    );

    BorderEffectType effectType = BORDER_EFFECT_NONE;
    BorderEffectType parseEffectType(String name);

    unsigned long effectInterval = 30;
    unsigned long effectLastMillis = 0;
    unsigned long lastShowMillis = 0;

    uint8_t effectBrightness = 0;
    bool effectState = true;
    uint32_t effectColor = 0;

    uint8_t effectCounter = 0;
    uint16_t effectPosition = 0;
    uint8_t effectWidth = 5;

    void updateEffect();
    void updateRainbowEffect();
    void updateFadeEffect();
    void updateFadeUpEffect();
    void updateFadeDownEffect();
    void updateFlashEffect();
    void updateLoaderEffect();

    void fill(uint32_t color);

    uint32_t wheel(uint8_t position);
    uint32_t scaleColor(uint32_t color, uint8_t brightness);
    uint32_t getContrastColor(uint32_t color);
};
