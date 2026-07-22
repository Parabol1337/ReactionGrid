#include "SelfTest.h"
void SelfTest::begin(Matrix* m,Border* b){matrix=m;border=b;}
void SelfTest::loop(bool active){if(active&&!lastActive)start();if(!active&&lastActive)stop();lastActive=active;if(active)handlePress();}
void SelfTest::start(){memset(state,0,sizeof(state));clearAll();Serial.println("SELFTEST START");}
void SelfTest::stop(){memset(state,0,sizeof(state));clearAll();Serial.println("SELFTEST STOP");}
void SelfTest::clearAll(){if(matrix)matrix->setMatrix(matrix->color(0,0,0));if(border)border->clear();}
void SelfTest::handlePress(){if(!matrix||!border||!matrix->hasEvent())return;uint8_t row=matrix->getPressedRow(),column=matrix->getPressedColumn();state[row][column]=(state[row][column]+1)%4;switch(state[row][column]){case 0:matrix->setPixel(row,column,matrix->color(0,0,0),false);border->setBorder(border->color(0,0,0));break;case 1:matrix->setPixel(row,column,matrix->color(255,0,0),false);border->setBorder(border->color(255,0,0));break;case 2:matrix->setPixel(row,column,matrix->color(0,255,0),false);border->setBorder(border->color(0,255,0));break;case 3:matrix->setPixel(row,column,matrix->color(0,0,255),false);border->setBorder(border->color(0,0,255));break;}matrix->clearEvent();}
