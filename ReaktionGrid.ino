/*
 * ==========================================================
 * ReactionGrid Firmware
 * ==========================================================
 *
 * Arduino ESP32 Core : 2.0.17
 * ArduinoJson        : 7.2.1
 * Adafruit NeoPixel  : 1.15.1
 *
 * Board              : ESP32-ETH01
 *
 * Getestet am        : 01.06.2026
 *
 * ==========================================================
 */
#include "Border.h"
#include "Matrix.h"
#include "Connection.h"
#include "Interface.h"
#include "SelfTest.h"

#define FW_VERSION __DATE__ " " __TIME__
#define HW_VERSION "10x10b20" //10x10 Matrix, 20 Segment Border
Border border;
Matrix matrix;
Connection connection;
Interface interface;
SelfTest selfTest;
ConnectionStatusType currentStatus = CONNECTION_NONE;


void setup() {
    Serial.begin(115200);

    Serial.println();
    Serial.println("================================");
    Serial.println("ReactionGrid startet...");
    Serial.println("================================");

    border.begin();
    matrix.begin();
    connection.begin(FW_VERSION,HW_VERSION);
    interface.begin();
    selfTest.begin(&matrix, &border);

    Serial.println("================================");
    Serial.println("System bereit");
    Serial.println("================================");
}

void loop() {
    interface.loop();
    border.loop();
    matrix.loop();
    connection.loop();

    bool selfTestActive = interface.hasSelfTest();
    selfTest.loop(selfTestActive);
    
    if (!selfTestActive) {
      ConnectionStatusType status = connection.getStatusType(); 
      if (status != currentStatus) {
          currentStatus = status;
          switch (status) {
              case CONNECTION_STARTUP:
                  matrix.setEffect("textscroll", 80,  matrix.color(255,255,255), "STARTING");
                  border.setEffect("loader", 40, border.color(0,0,255));
                  break;
      
              case CONNECTION_NO_LAN:
                  matrix.setEffect("textscroll", 80, matrix.color(255,0,0), "NO LINK");
                  border.setEffect("flash", 300, border.color(255,0,0));
                  break;
      
              case CONNECTION_DHCP:
                  matrix.setEffect("textscroll", 80, matrix.color(255,255,0), "WAIT DHCP");
                  border.setEffect("loader", 40, border.color(255,255,0));
                  break;
      
              case CONNECTION_WAIT_SERVER:
                  matrix.setEffect("textscroll", 80, matrix.color(255,255,255), "WAIT SERVER");
                  border.setEffect("loader", 40, border.color(255,255,255));
                  break;
      
              case CONNECTION_READY:
                  matrix.setEffect("textscroll", 80, matrix.color(0,255,0), "READY");
                  border.setEffect("fade", 30, border.color(0,255,0));
                  break;
      
              default:
                  break;
          }
      }
      
      if (status == CONNECTION_READY) {
          handleMatrixPressEvent();
          handleConnectionEvent();
      }
        
    }
}

// Tastendruck der Matrix verarbeiten
void handleMatrixPressEvent() {
    if (!matrix.hasEvent()) {
        return;
    }

    uint8_t row = matrix.getPressedRow();
    uint8_t column = matrix.getPressedColumn();
    unsigned long reactionTime = matrix.getReactionTime();

    Serial.print("PRESS EVENT -> R");
    Serial.print(row);
    Serial.print("T");
    Serial.print(column);
    Serial.print(" | ");
    Serial.print(reactionTime);
    Serial.println(" ms");

    connection.sendPixelPressed(row, column, reactionTime);
    matrix.clearEvent();
}

// Eingehende Connection-Befehle verarbeiten
void handleConnectionEvent() {
    if (!connection.hasEvent()) {
        return;
    }

    switch (connection.getEventType()) {
        case EVENT_SET_PIXEL:
            handleSetPixel();
            break;
            
       case EVENT_SET_ROW:
            handleSetRow();
            break;
            
       case EVENT_SET_COLUMN:
            handleSetColumn();
            break;

        case EVENT_SET_BORDER:
            handleSetBorder();
            break;

        case EVENT_SET_SEGMENT:
            handleSetSegment();
            break;
            
        case EVENT_SET_COUNTER:
            handleSetCounter();
            break;

        case EVENT_SET_MATRIX:
            handleSetMatrix();
            break;

        case EVENT_MATRIX_EFFECT:
            handleMatrixEffect();
            break;

        case EVENT_BORDER_EFFECT:
            handleBorderEffect();
            break;

        default:
            break;
    }

    connection.clearEvent();
}

// Einzelnes Matrix-Pixel setzen
void handleSetPixel() {
    uint32_t color = connection.getColor();
    Serial.print("SETPIXEL -> R");
    Serial.print(connection.getPixelRow());
    Serial.print("T");
    Serial.print(connection.getPixelColumn());
    Serial.print(" ");
    printColor(color);
    for (uint8_t i = 0; i < connection.getMultiPixelCount(); i++) {
         Serial.print("SETPIXEL -> R");
         Serial.print(connection.getMultiPixelRow(i));
         Serial.print("T");
         Serial.print(connection.getMultiPixelColumn(i));
         Serial.print(" ");
        matrix.setPixel(
            connection.getMultiPixelRow(i),
            connection.getMultiPixelColumn(i),
            color,
            false
        );
    }
}

void handleSetRow() {
    uint32_t color = connection.getColor();
    Serial.print("SETROW -> R");
    Serial.print(connection.getPixelRow());
    Serial.print(" ");
    printColor(color);
    matrix.setRow(
        connection.getPixelRow(),
        color
    );
}

void handleSetColumn() {
    uint32_t color = connection.getColor();
    Serial.print("SETCOLUMN -> T");
    Serial.print(connection.getPixelColumn());
    Serial.print(" ");
    printColor(color);
    matrix.setColumn(
        connection.getPixelColumn(),
        color
    );
}

// Rahmen komplett setzen
void handleSetCounter() {
    uint32_t color = connection.getColor();
    Serial.print("SETCOUNTER -> ");
    printColor(color);
    border.setCounter(connection.getValue(),color);
}



// Rahmen komplett setzen
void handleSetBorder() {
    uint32_t color = connection.getColor();
    Serial.print("SETBORDER -> ");
    printColor(color);
    border.setBorder(color);
}

// Rahmen-Segment setzen
void handleSetSegment() {
    uint32_t color = connection.getColor();
    for (uint8_t i = 0; i < connection.getSegmentCount(); i++) {
        Serial.print("SETSEGMENT -> ");
        Serial.print(connection.getSegmentNumber(i));
        Serial.print(" ");
        printColor(color);
        border.setSegment(
            connection.getSegmentNumber(i),
            color
        );
    }
}

// Matrix komplett setzen
void handleSetMatrix() {
    uint32_t color = connection.getColor();
    Serial.print("SETMATRIX -> ");
    printColor(color);
    matrix.setMatrix(color);
}

// Matrix-Effekt starten
void handleMatrixEffect() {
    uint32_t color = connection.getColor();
    Serial.print("MATRIXEFFECT -> ");
    Serial.print(connection.getEffectName());
    Serial.print(" | ");
    Serial.print(connection.getInterval());
    Serial.print(" ms | ");
    printColor(color);

    matrix.setEffect(
        connection.getEffectName(),
        connection.getInterval(),
        color,
        connection.getText()
    );
}

// Rahmen-Effekt starten
void handleBorderEffect() {
    uint32_t color = connection.getColor();
    Serial.print("BORDEREFFECT -> ");
    Serial.print(connection.getEffectName());
    Serial.print(" | ");
    Serial.print(connection.getInterval());
    Serial.print(" ms | ");
    printColor(color);

    border.setEffect(
        connection.getEffectName(),
        connection.getInterval(),
        color
    );
}

// Farbe lesbar auf Serial ausgeben
void printColor(uint32_t color) {
    Serial.print("RGB(");
    Serial.print((color >> 16) & 0xFF);
    Serial.print(", ");
    Serial.print((color >> 8) & 0xFF);
    Serial.print(", ");
    Serial.print(color & 0xFF);
    Serial.println(")");
}
