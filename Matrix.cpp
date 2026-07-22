#include "Matrix.h"

void Matrix::begin() {
    pixels.begin();
    pixels.setBrightness(MATRIX_BRIGHTNESS);
    pixels.show();
    I2C_1.begin(SDA1, SCL1, 400000);
    I2C_2.begin(SDA2, SCL2, 400000);
    for (uint8_t row = 0; row < 10; row++) pcfWrite(row, 0xFFFF);
    clear();
    Serial.println("Matrix gestartet");
}

void Matrix::loop() {
    checkPixels();
    updateEffect();
    if(effectType == MATRIX_EFFECT_NONE && millis() - lastShowMillis >= 50) {
        pixels.show();
        lastShowMillis = millis();
    }
}

uint32_t Matrix::color(uint8_t r, uint8_t g, uint8_t b) { return pixels.Color(r, g, b); }
MatrixEffectType Matrix::getEffectType() { return effectType; }

MatrixEffectType Matrix::parseEffectType(String name) {
    name.trim(); name.toLowerCase();
    if (name == "textcenter") return MATRIX_EFFECT_TEXT_CENTER;
    if (name == "textscroll") return MATRIX_EFFECT_TEXT_SCROLL;
    if (name == "winner") return MATRIX_EFFECT_WINNER;
    if (name == "looser" || name == "loser") return MATRIX_EFFECT_LOOSER;
    return MATRIX_EFFECT_NONE;
}

void Matrix::clear() { effectType = MATRIX_EFFECT_NONE; pixels.clear(); pixels.show(); }
void Matrix::setMatrix(uint32_t color) { effectType = MATRIX_EFFECT_NONE; fill(color); pixels.show(); }

void Matrix::setPixel(uint8_t row, uint8_t column, uint32_t color, bool startTimer) {
    if (row >= 10 || column >= 10) return;
    effectType = MATRIX_EFFECT_NONE;
    setDot(row, column, color);
    if (startTimer) pixelActivatedMillis[row][column] = millis();
    pixels.show();
}

void Matrix::setRow(uint8_t row, uint32_t color) {
    if (row >= 10) return;
    effectType = MATRIX_EFFECT_NONE;
    for (uint8_t column = 0; column < 10; column++) setDot(row, column, color);
    pixels.show();
}

void Matrix::setColumn(uint8_t column, uint32_t color) {
    if (column >= 10) return;
    effectType = MATRIX_EFFECT_NONE;
    for (uint8_t row = 0; row < 10; row++) setDot(row, column, color);
    pixels.show();
}

void Matrix::setEffect(String name, unsigned long interval, uint32_t color, String text) {
    effectType = parseEffectType(name);
    effectInterval = interval;
    effectColor = color;
    effectText = text;
    effectTextOffset = 10;
    effectTextCharIndex = 0;
    effectCounter = 0;
    effectState = false;
    effectLastMillis = millis();
}

void Matrix::updateEffect() {
    if (effectType == MATRIX_EFFECT_NONE || millis() - effectLastMillis < effectInterval) return;
    switch (effectType) {
        case MATRIX_EFFECT_TEXT_CENTER: updateTextCenterEffect(); break;
        case MATRIX_EFFECT_TEXT_SCROLL: updateTextScrollEffect(); break;
        case MATRIX_EFFECT_WINNER: updateWinnerEffect(); break;
        case MATRIX_EFFECT_LOOSER: updateLooserEffect(); break;
        default: break;
    }
    pixels.show();
    effectLastMillis = millis();
}

void Matrix::updateTextScrollEffect() {
    pixels.clear();
    for (uint8_t row = 0; row < 10; row++)
        for (uint8_t column = 0; column < 10; column++)
            if (getTextPixel(effectText, column - effectTextOffset, row)) setDot(row, column, effectColor);
    effectTextOffset--;
    int16_t textWidth = effectText.length() * 6;
    if (effectTextOffset < -textWidth) effectTextOffset = 10;
}

void Matrix::updateTextCenterEffect() {
    pixels.clear();
    if (effectText.length() == 0) return;
    String oneChar = effectText.substring(effectTextCharIndex, effectTextCharIndex + 1);
    for (uint8_t row = 0; row < 10; row++)
        for (uint8_t column = 0; column < 10; column++)
            if (getTextPixel(oneChar, column - 2, row - 1)) setDot(row, column, effectColor);
    effectTextCharIndex++;
    if (effectTextCharIndex >= effectText.length()) effectTextCharIndex = 0;
}

void Matrix::updateWinnerEffect() {
    for (uint16_t i = 0; i < MATRIX_LED_COUNT; i++) pixels.setPixelColor(i, wheel(effectCounter + i * 8));
    effectCounter += 4;
}

void Matrix::updateLooserEffect() { effectState = !effectState; fill(effectState ? color(255, 0, 0) : 0); }
void Matrix::setDot(uint8_t row, uint8_t column, uint32_t color) { if (row < 10 && column < 10) pixels.setPixelColor(getLedIndex(row, column), color); }
void Matrix::fill(uint32_t color) { for (uint16_t i = 0; i < MATRIX_LED_COUNT; i++) pixels.setPixelColor(i, color); }

uint16_t Matrix::getLedIndex(uint8_t row, uint8_t column) {
    uint8_t stripRow = 9 - row;
    return stripRow % 2 == 0 ? stripRow * 10 + (9 - column) : stripRow * 10 + column;
}

bool Matrix::getTextPixel(String text, int16_t x, uint8_t y) {
    if (y < 1 || y > 7 || x < 0) return false;
    uint8_t charIndex = x / 6;
    uint8_t charColumn = x % 6;
    if (charIndex >= text.length() || charColumn >= 5) return false;
    int index = fontIndex(text[charIndex]);
    return bitRead(FONT[index][charColumn], y - 1);
}

uint32_t Matrix::wheel(uint8_t position) {
    position = 255 - position;
    if (position < 85) return pixels.Color(255 - position * 3, 0, position * 3);
    if (position < 170) { position -= 85; return pixels.Color(0, position * 3, 255 - position * 3); }
    position -= 170;
    return pixels.Color(position * 3, 255 - position * 3, 0);
}

void Matrix::checkPixels() {
    unsigned long currentMillis = millis();
    for (uint8_t row = 0; row < 10; row++) {
        uint16_t pcfState = pcfRead(row);
        for (uint8_t column = 0; column < 10; column++) {
            bool currentButtonState = bitRead(pcfState, buttonPins[column]) == 0;
            if (currentButtonState != lastButtonState[row][column]) {
                lastButtonState[row][column] = currentButtonState;
                lastButtonChangeMillis[row][column] = currentMillis;
            }
            if (currentMillis - lastButtonChangeMillis[row][column] < DEBOUNCE_MS) continue;
            if (debouncedButtonState[row][column] == currentButtonState) continue;
            debouncedButtonState[row][column] = currentButtonState;
            if (debouncedButtonState[row][column]) {
                pressEvent = true;
                pressedRow = row;
                pressedColumn = column;
                reactionTime = currentMillis - pixelActivatedMillis[row][column];
            }
        }
    }
}

bool Matrix::hasEvent() { return pressEvent; }
void Matrix::clearEvent() { pressEvent = false; }
uint8_t Matrix::getPressedRow() { return pressedRow; }
uint8_t Matrix::getPressedColumn() { return pressedColumn; }
unsigned long Matrix::getReactionTime() { return reactionTime; }

void Matrix::pcfWrite(uint8_t row, uint16_t portValue) {
    TwoWire *i2cBus = row < 5 ? &I2C_1 : &I2C_2;
    i2cBus->beginTransmission(pcfAddress[row]);
    i2cBus->write(portValue & 0xFF);
    i2cBus->write((portValue >> 8) & 0xFF);
    i2cBus->endTransmission();
}

uint16_t Matrix::pcfRead(uint8_t row) {
    TwoWire *i2cBus = row < 5 ? &I2C_1 : &I2C_2;
    i2cBus->requestFrom(pcfAddress[row], (uint8_t)2);
    if (i2cBus->available() < 2) return 0xFFFF;
    uint8_t lowByte = i2cBus->read();
    uint8_t highByte = i2cBus->read();
    return lowByte | (highByte << 8);
}
