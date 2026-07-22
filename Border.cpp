#include "Border.h"

void Border::begin(){pixels.begin();pixels.setBrightness(BORDER_BRIGHTNESS);pixels.show();clear();Serial.println("Border gestartet");}
void Border::loop(){updateEffect();if(effectType==BORDER_EFFECT_NONE&&millis()-lastShowMillis>=50){pixels.show();lastShowMillis=millis();}}
uint32_t Border::color(uint8_t r,uint8_t g,uint8_t b){return pixels.Color(r,g,b);} BorderEffectType Border::getEffectType(){return effectType;}
BorderEffectType Border::parseEffectType(String name){name.trim();name.toLowerCase();if(name=="rainbow")return BORDER_EFFECT_RAINBOW;if(name=="fade")return BORDER_EFFECT_FADE;if(name=="fadeup")return BORDER_EFFECT_FADEUP;if(name=="fadedown")return BORDER_EFFECT_FADEDOWN;if(name=="flash")return BORDER_EFFECT_FLASH;if(name=="loader")return BORDER_EFFECT_LOADER;return BORDER_EFFECT_NONE;}
void Border::clear(){effectType=BORDER_EFFECT_NONE;pixels.clear();pixels.show();}
void Border::setBorder(uint32_t c){effectType=BORDER_EFFECT_NONE;fill(c);pixels.show();}
void Border::setSegment(uint8_t s,uint32_t c){if(s>=BORDER_SEGMENT_COUNT)return;effectType=BORDER_EFFECT_NONE;for(uint16_t i=borderSegments[s].start;i<=borderSegments[s].end;i++)pixels.setPixelColor(i,c);pixels.show();}
void Border::setCounter(uint8_t p,uint32_t c){if(p>100)p=100;effectType=BORDER_EFFECT_NONE;uint16_t n=((uint32_t)BORDER_LED_COUNT*p)/100;for(uint16_t i=0;i<BORDER_LED_COUNT;i++)pixels.setPixelColor(i,i<n?c:0);pixels.show();}
void Border::setEffect(String name,unsigned long interval,uint32_t c){effectType=parseEffectType(name);effectInterval=interval;effectColor=c;effectBrightness=0;effectState=true;effectPosition=0;effectLastMillis=millis();}
void Border::updateEffect(){if(effectType==BORDER_EFFECT_NONE||millis()-effectLastMillis<effectInterval)return;switch(effectType){case BORDER_EFFECT_RAINBOW:updateRainbowEffect();break;case BORDER_EFFECT_FADE:updateFadeEffect();break;case BORDER_EFFECT_FADEUP:updateFadeUpEffect();break;case BORDER_EFFECT_FADEDOWN:updateFadeDownEffect();break;case BORDER_EFFECT_FLASH:updateFlashEffect();break;case BORDER_EFFECT_LOADER:updateLoaderEffect();break;default:break;}pixels.show();effectLastMillis=millis();}
void Border::updateRainbowEffect(){for(uint16_t i=0;i<BORDER_LED_COUNT;i++)pixels.setPixelColor(i,wheel(effectCounter+(i*255/BORDER_LED_COUNT)));effectCounter++;}
void Border::updateFadeEffect(){if(effectState){effectBrightness+=5;if(effectBrightness>=250){effectBrightness=255;effectState=false;}}else{effectBrightness-=5;if(effectBrightness<=5){effectBrightness=0;effectState=true;}}fill(scaleColor(effectColor,effectBrightness));}
void Border::updateFadeUpEffect(){effectBrightness+=5;if(effectBrightness>=255)effectBrightness=0;fill(scaleColor(effectColor,effectBrightness));}
void Border::updateFadeDownEffect(){if(effectBrightness==0)effectBrightness=255;effectBrightness-=5;fill(scaleColor(effectColor,effectBrightness));}
void Border::updateFlashEffect(){effectState=!effectState;fill(effectState?effectColor:0);}
void Border::updateLoaderEffect(){uint32_t pointColor=getContrastColor(effectColor);fill(effectColor);for(uint8_t i=0;i<effectWidth;i++)pixels.setPixelColor((effectPosition+i)%BORDER_LED_COUNT,pointColor);effectPosition=(effectPosition+1)%BORDER_LED_COUNT;}
void Border::fill(uint32_t c){for(uint16_t i=0;i<BORDER_LED_COUNT;i++)pixels.setPixelColor(i,c);}
uint32_t Border::wheel(uint8_t p){p=255-p;if(p<85)return pixels.Color(255-p*3,0,p*3);if(p<170){p-=85;return pixels.Color(0,p*3,255-p*3);}p-=170;return pixels.Color(p*3,255-p*3,0);}
uint32_t Border::scaleColor(uint32_t c,uint8_t b){uint8_t r=(uint8_t)(c>>16),g=(uint8_t)(c>>8),bl=(uint8_t)c;r=(uint16_t)r*b/255;g=(uint16_t)g*b/255;bl=(uint16_t)bl*b/255;return pixels.Color(r,g,bl);}
uint32_t Border::getContrastColor(uint32_t c){return pixels.Color(255-(uint8_t)(c>>16),255-(uint8_t)(c>>8),255-(uint8_t)c);}
