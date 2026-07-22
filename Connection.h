#pragma once

#include <Arduino.h>
#include <ETH.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Preferences.h>

#define ETH_PHY_TYPE    ETH_PHY_LAN8720
#define ETH_PHY_ADDR    1
#define ETH_PHY_MDC     23
#define ETH_PHY_MDIO    18
#define ETH_PHY_POWER   16
#define ETH_CLK_MODE    ETH_CLOCK_GPIO0_IN

enum ConnectionEventType { EVENT_NONE, EVENT_SET_PIXEL, EVENT_SET_BORDER, EVENT_SET_ROW, EVENT_SET_COLUMN, EVENT_SET_MATRIX, EVENT_SET_SEGMENT, EVENT_SET_COUNTER, EVENT_MATRIX_EFFECT, EVENT_BORDER_EFFECT };
enum ConnectionStatusType { CONNECTION_NONE, CONNECTION_STARTUP, CONNECTION_NO_LAN, CONNECTION_DHCP, CONNECTION_WAIT_SERVER, CONNECTION_READY };

class Connection {
public:
    void begin(const char *fwversion="unknown",const char *hwversion="unknown"); void loop();
    void sendPixelPressed(uint8_t row,uint8_t column,unsigned long reactionTime); void sendHeartbeat(); void updateHeartbeat();
    bool hasEvent(); void clearEvent(); ConnectionEventType getEventType();
    uint8_t getPixelRow(); uint8_t getValue(); uint8_t getPixelColumn();
    uint8_t getMultiPixelCount(); uint8_t getMultiPixelRow(uint8_t index); uint8_t getMultiPixelColumn(uint8_t index);
    uint8_t getSegmentCount(); uint8_t getSegmentNumber(uint8_t index);
    uint32_t getColor(); unsigned long getInterval(); String getText(); String getEffectName(); ConnectionStatusType getStatusType();
private:
    const char *firmwareVersion="unknown"; const char *hardwareVersion="unknown"; WiFiUDP udp; Preferences preferences; IPAddress serverIp;
    IPAddress apIp=IPAddress(192,168,10,1),apGateway=IPAddress(192,168,10,1),apSubnet=IPAddress(255,255,255,0);
    String hostName="ReactionWall",password="12345678"; uint16_t serverPort=5000,localPort=5001; ConnectionStatusType statusType=CONNECTION_STARTUP; bool serverOnline=false;
    unsigned long heartbeatInterval=5000,heartbeatTimeout=0,heartbeatLastSendMillis=0,heartbeatLastAckMillis=0;
    bool ethDhcp=true; IPAddress ethIp=IPAddress(192,168,1,50),ethGateway=IPAddress(192,168,1,1),ethSubnet=IPAddress(255,255,255,0),ethDns=IPAddress(192,168,1,1);
    char receiveBuffer[2048]; ConnectionEventType eventType=EVENT_NONE; uint8_t counterValue=0,pixelRow=0,pixelColumn=0,segmentNumber=0; uint32_t color=0; unsigned long effectInterval=30; String text="",effectName="";
    void loadSettings(); void beginAccessPoint(); void beginEthernet(); void receivePacket(); void handleJson(JsonDocument &json);
    uint8_t multiPixelRows[100],multiPixelColumns[100],multiPixelCount=0,segmentNumbers[20],segmentCount=0;
    uint32_t makeColor(uint8_t r,uint8_t g,uint8_t b); IPAddress getIpSetting(String key,IPAddress fallback);
};
