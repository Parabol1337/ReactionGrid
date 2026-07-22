#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ETH.h>
#include <WiFi.h>
#include <Update.h>

class Interface {
public:
    void begin();
    void loop();
    bool hasSelfTest();

private:
    WebServer server{80};
    Preferences preferences;

    // Login / AP
    String authUser = "admin";
    String hostName = "ReactionWall";
    String password = "12345678";

    // UDP
    String serverIp = "192.168.1.100";
    uint16_t serverPort = 5000;
    uint16_t localPort = 5001;

    // Ethernet
    bool ethDhcp = true;
    String ethIp = "192.168.1.50";
    String ethGateway = "192.168.1.1";
    String ethSubnet = "255.255.255.0";
    String ethDns = "192.168.1.1";

    unsigned long heartbeatTimeout = 0;
    bool selfTestActive = false;

    void loadSettings();
    void beginWebServer();

    bool checkAuth();

    void handleRoot();
    void handleSettings();
    void handleSaveSettings();
    void handleUpdate();
    void handleUpdateUpload();
    void handleRestart();

    String getHtmlHeader(String title);
    String getHtmlFooter();
    String checked(bool value);
    
};
