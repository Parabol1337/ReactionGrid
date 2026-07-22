#include "Connection.h"

void Connection::begin(const char *fwversion, const char *hwversion) {
    firmwareVersion = fwversion;
    hardwareVersion = hwversion;

    loadSettings();

    beginAccessPoint();
    beginEthernet();

    udp.begin(localPort);

    Serial.print("UDP gestartet auf Port ");
    Serial.println(localPort);
}

void Connection::loop() {
    receivePacket();
    updateHeartbeat();

    if (millis() < 5000) {
        statusType = CONNECTION_STARTUP;
        return;
    }
    if (!ETH.linkUp()) {
        statusType = CONNECTION_NO_LAN;
        return;
    }
    if (ETH.localIP() == IPAddress(0, 0, 0, 0)) {
        statusType = CONNECTION_DHCP;
        return;
    } 
    if (!serverOnline) {
        statusType = CONNECTION_WAIT_SERVER;
        return;
    } 
    statusType = CONNECTION_READY;    
}

void Connection::loadSettings() {
    preferences.begin("wall", true);

    hostName = preferences.getString("host_name", "ReactionWall");
    password = preferences.getString("password", "12345678");

    String serverIpString = preferences.getString("server_ip", "192.168.1.100");

    serverPort = preferences.getUShort("server_port", 5000);
    localPort = preferences.getUShort("local_port", 5001);

    heartbeatTimeout = preferences.getULong("hb_timeout", 0);

    ethDhcp = preferences.getBool("eth_dhcp", true);

    preferences.end();

    if (!serverIp.fromString(serverIpString)) {
        serverIp = IPAddress(192, 168, 1, 100);
    }

    ethIp = getIpSetting("eth_ip", IPAddress(192, 168, 1, 50));
    ethGateway = getIpSetting("eth_gateway", IPAddress(192, 168, 1, 1));
    ethSubnet = getIpSetting("eth_subnet", IPAddress(255, 255, 255, 0));
    ethDns = getIpSetting("eth_dns", IPAddress(192, 168, 1, 1));
}

IPAddress Connection::getIpSetting(String key, IPAddress fallback) {
    preferences.begin("wall", true);

    String value = preferences.getString(key.c_str(), "");

    preferences.end();

    IPAddress ip;

    if (!ip.fromString(value)) {
        return fallback;
    }

    return ip;
}

void Connection::beginAccessPoint() {
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);

    WiFi.softAPConfig(apIp, apGateway, apSubnet);
    WiFi.softAP(hostName.c_str(), password.c_str());

    Serial.print("WIFI AP gestartet: ");
    Serial.println(hostName);

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void Connection::beginEthernet() {
    Serial.println("Starte Ethernet...");

    ETH.setHostname(hostName.c_str());

    ETH.begin(
        ETH_PHY_ADDR,
        ETH_PHY_POWER,
        ETH_PHY_MDC,
        ETH_PHY_MDIO,
        ETH_PHY_TYPE,
        ETH_CLK_MODE
    );

    if (!ethDhcp) {
        Serial.println("Ethernet statische IP aktiv");

        if (!ETH.config(ethIp, ethGateway, ethSubnet, ethDns)) {
            Serial.println("ETH.config fehlgeschlagen");
        }
    } else {
        Serial.println("Ethernet DHCP aktiv");
    }
}

void Connection::sendPixelPressed(uint8_t row, uint8_t column, unsigned long reactionTime) {
    JsonDocument json;

    json["type"] = "press";
    json["row"] = row;
    json["column"] = column;
    json["reactionTime"] = reactionTime;

    udp.beginPacket(serverIp, serverPort);
    serializeJson(json, udp);
    udp.endPacket();
}

void Connection::sendHeartbeat() {
    JsonDocument json;

    json["type"] = "heartbeat";
    json["hostName"] = hostName;
    json["replyPort"] = localPort;
    json["fwversion"] = firmwareVersion;
    json["hwversion"] = hardwareVersion;

    udp.beginPacket(serverIp, serverPort);
    serializeJson(json, udp);
    udp.endPacket();

    Serial.println("Heartbeat gesendet");
}

void Connection::updateHeartbeat() {
    unsigned long currentMillis = millis();

    if (currentMillis - heartbeatLastSendMillis >= heartbeatInterval) {
        heartbeatLastSendMillis = currentMillis;
        sendHeartbeat();
    }

    if (heartbeatTimeout == 0) {
        serverOnline = true;
        return;
    }

    if (currentMillis - heartbeatLastAckMillis >= heartbeatTimeout) {
        serverOnline = false;
    }
}

void Connection::receivePacket() {
    int packetSize = udp.parsePacket();

    if (packetSize <= 0) {
        return;
    }

    int length = udp.read(receiveBuffer, sizeof(receiveBuffer) - 1);

    if (length <= 0) {
        return;
    }

    receiveBuffer[length] = '\0';

    JsonDocument json;
    DeserializationError error = deserializeJson(json, receiveBuffer);

    if (error) {
        Serial.print("JSON Fehler: ");
        Serial.println(error.c_str());
        return;
    }

    handleJson(json);
}

void Connection::handleJson(JsonDocument &json) {
    const char *type = json["type"];

    if (type == nullptr) {
        Serial.println("JSON ohne type");
        return;
    }
        
    heartbeatLastAckMillis = millis();
    serverOnline = true;

    if (strcmp(type, "heartbeatAck") == 0) {
        Serial.println("Heartbeat ACK empfangen");
        return;
    }

    uint8_t r = json["r"] | 0;
    uint8_t g = json["g"] | 0;
    uint8_t b = json["b"] | 0;

    color = makeColor(r, g, b);

    if (strcmp(type, "setPixel") == 0) {
    
        multiPixelCount = 0;
    
        if (json["pixels"].is<JsonArray>()) {
            JsonArray pixels = json["pixels"].as<JsonArray>();
    
            for (JsonObject pixel : pixels) {
                if (multiPixelCount >= 100) {
                    break;
                }
    
                uint8_t row = pixel["row"] | 0;
                uint8_t column = pixel["column"] | 0;
    
                if (row > 9 || column > 9) {
                    continue;
                }
    
                multiPixelRows[multiPixelCount] = row;
                multiPixelColumns[multiPixelCount] = column;
                multiPixelCount++;
            }
    
            eventType = EVENT_SET_PIXEL;
            return;
        }
    
        uint8_t row = json["row"] | 0;
        uint8_t column = json["column"] | 0;
    
        if (row > 9 || column > 9) {
            Serial.println("setPixel ungueltige Koordinaten");
            return;
        }
    
        multiPixelRows[0] = row;
        multiPixelColumns[0] = column;
        multiPixelCount = 1;
    
        eventType = EVENT_SET_PIXEL;
        return;
    }

    if (strcmp(type, "setSegment") == 0) {
    
        segmentCount = 0;
    
        if (json["segments"].is<JsonArray>()) {
            JsonArray segments = json["segments"].as<JsonArray>();
    
            for (JsonVariant segmentValue : segments) {
                if (segmentCount >= 20) {
                    break;
                }
    
                uint8_t segment = segmentValue | 0;
    
                if (segment >= 20) {
                    continue;
                }
    
                segmentNumbers[segmentCount] = segment;
                segmentCount++;
            }
    
            eventType = EVENT_SET_SEGMENT;
            return;
        }
    
        uint8_t segment = json["segment"] | 0;
    
        if (segment >= 20) {
            Serial.println("setSegment ungueltiges Segment");
            return;
        }
    
        segmentNumbers[0] = segment;
        segmentNumber = segment;
        segmentCount = 1;
    
        eventType = EVENT_SET_SEGMENT;
        return;
    }

    if (strcmp(type, "setMatrix") == 0) {
        eventType = EVENT_SET_MATRIX;
        return;
    }

    if (strcmp(type, "setColumn") == 0) {
        pixelColumn = json["column"] | 0;
        eventType = EVENT_SET_COLUMN;
        return;
    }
    if (strcmp(type, "setRow") == 0) {
        pixelRow = json["row"] | 0;
        eventType = EVENT_SET_ROW;
        return;
    }

    if (strcmp(type, "setCounter") == 0) {
        counterValue = json["value"] | 0;
        eventType = EVENT_SET_COUNTER;
        return;
    }
    
    if (strcmp(type, "setBorder") == 0) {
        eventType = EVENT_SET_BORDER;
        return;
    }

    if (strcmp(type, "effectMatrix") == 0) {
        effectName = json["effect"] | "";
        effectName.trim();
        effectName.toLowerCase();
        text = json["text"] | "";
                
        effectInterval = json["interval"] | 30;
        eventType = EVENT_MATRIX_EFFECT;
        return;
    }

    if (strcmp(type, "effectBorder") == 0) {
        effectName = json["effect"] | "";
        effectName.trim();
        effectName.toLowerCase();

        effectInterval = json["interval"] | 30;
        eventType = EVENT_BORDER_EFFECT;
        return;
    }
}

uint32_t Connection::makeColor(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

bool Connection::hasEvent() {
    return eventType != EVENT_NONE;
}

void Connection::clearEvent() {
    eventType = EVENT_NONE;
}

ConnectionEventType Connection::getEventType() {
    return eventType;
}

uint8_t Connection::getValue() {
    return counterValue;
}

uint8_t Connection::getPixelRow() {
    return pixelRow;
}

uint8_t Connection::getPixelColumn() {
    return pixelColumn;
}

uint8_t Connection::getSegmentCount() {
    return segmentCount;
}

uint8_t Connection::getSegmentNumber(uint8_t index) {
    if (index >= segmentCount) {
        return 0;
    }

    return segmentNumbers[index];
}

uint32_t Connection::getColor() {
    return color;
}

unsigned long Connection::getInterval() {
    return effectInterval;
}

String Connection::getText() {
    return text;
}

String Connection::getEffectName() {
    return effectName;
}

ConnectionStatusType Connection::getStatusType() {
    return statusType;
}

uint8_t Connection::getMultiPixelCount() {
    return multiPixelCount;
}

uint8_t Connection::getMultiPixelRow(uint8_t index) {
    if (index >= multiPixelCount) return 0;
    return multiPixelRows[index];
}

uint8_t Connection::getMultiPixelColumn(uint8_t index) {
    if (index >= multiPixelCount) return 0;
    return multiPixelColumns[index];
}
