#include "Interface.h"

// Webinterface starten
void Interface::begin() {
    loadSettings();
    beginWebServer();

    Serial.println("Interface gestartet");
}

// Webserver bedienen
void Interface::loop() {
    server.handleClient();
}

// Einstellungen laden
void Interface::loadSettings() {
    preferences.begin("wall", true);

    hostName = preferences.getString("host_name", "ReactionWall");
    password = preferences.getString("password", "12345678");

    serverIp = preferences.getString("server_ip", "192.168.1.100");
    serverPort = preferences.getUShort("server_port", 5000);
    localPort = preferences.getUShort("local_port", 5001);

    ethDhcp = preferences.getBool("eth_dhcp", true);
    ethIp = preferences.getString("eth_ip", "192.168.1.50");
    ethGateway = preferences.getString("eth_gateway", "192.168.1.1");
    ethSubnet = preferences.getString("eth_subnet", "255.255.255.0");
    ethDns = preferences.getString("eth_dns", "192.168.1.1");

    heartbeatTimeout = preferences.getULong("hb_timeout", 0);

    preferences.end();
}

// Webserver-Routen
void Interface::beginWebServer() {
    server.on("/", HTTP_GET, [this]() {
        handleRoot();
    });

    server.on("/settings", HTTP_GET, [this]() {
        if (!checkAuth()) return;
        handleSettings();
    });

    server.on("/settings", HTTP_POST, [this]() {
        if (!checkAuth()) return;
        handleSaveSettings();
    });

    server.on("/selftest", HTTP_POST, [this]() {
        
        if(selfTestActive){
          selfTestActive = false;
        }else{
          selfTestActive = true;
        }
        handleRoot();
    });

    server.on("/update", HTTP_GET, [this]() {
        if (!checkAuth()) return;
        handleUpdate();
    });

    server.on(
        "/update",
        HTTP_POST,
        [this]() {
            if (!checkAuth()) return;

            server.sendHeader("Connection", "close");
            server.send(
                200,
                "text/plain",
                Update.hasError() ? "Update FEHLER" : "Update OK - Neustart..."
            );

            delay(500);
            ESP.restart();
        },
        [this]() {
            if (!checkAuth()) return;
            handleUpdateUpload();
        }
    );

    server.on("/restart", HTTP_POST, [this]() {
        if (!checkAuth()) return;
        handleRestart();
    });

    server.begin();

    Serial.println("Webserver gestartet");
}

// Basic Auth prüfen
bool Interface::checkAuth() {
    if (server.authenticate(authUser.c_str(), password.c_str())) {
        return true;
    }

    server.requestAuthentication();
    return false;
}

// Startseite
void Interface::handleRoot() {
    String html = getHtmlHeader("Reaction Wall");

    html += "<h1>Reaction Wall</h1>";

    html += "<p><b>Hostname:</b> " + hostName + "</p>";
    html += "<p><b>Ethernet IP:</b> " + ETH.localIP().toString() + "</p>";
    html += "<p><b>AP IP:</b> " + WiFi.softAPIP().toString() + "</p>";

    html += "<p><a href='/settings'>Einstellungen</a></p>";
    html += "<p><a href='/update'>Firmware Update</a></p>";

    html += "<form method='POST' action='/restart'>";
    html += "<button type='submit'>ESP neustarten</button>";
    html += "</form>";

    html += "<form method='POST' action='/selftest'>";
    if (selfTestActive) {
        html += "<button type='submit'>Selftest stoppen</button>";
    } else {
        html += "<button type='submit'>Selftest starten</button>";
    }
    html += "</form>";

    html += getHtmlFooter();

    server.send(200, "text/html", html);
}

// Einstellungsformular
void Interface::handleSettings() {
    String html = getHtmlHeader("Einstellungen");

    html += "<h1>Einstellungen</h1>";
    html += "<form method='POST' action='/settings'>";

    html += "<h2>System</h2>";

    html += "<label>Hostname</label><br>";
    html += "<input name='host_name' value='" + hostName + "'><br><br>";

    html += "<label>Passwort</label><br>";
    html += "<input name='password' type='password' value='" + password + "'><br><br>";

    html += "<h2>UDP Server</h2>";

    html += "<label>Server IP</label><br>";
    html += "<input name='server_ip' value='" + serverIp + "'><br><br>";

    html += "<label>Server Port</label><br>";
    html += "<input name='server_port' value='" + String(serverPort) + "'><br><br>";

    html += "<label>Lokaler UDP Port</label><br>";
    html += "<input name='local_port' value='" + String(localPort) + "'><br><br>";

    html += "<label>Heartbeat Timeout ms (0 = aus / Debug)</label><br>";
    html += "<input name='hb_timeout' value='" + String(heartbeatTimeout) + "'><br><br>";

    html += "<h2>Ethernet</h2>";

    html += "<label>";
    html += "<input type='checkbox' name='eth_dhcp' value='1' " + checked(ethDhcp) + ">";
    html += " DHCP verwenden";
    html += "</label><br><br>";

    html += "<label>Statische IP</label><br>";
    html += "<input name='eth_ip' value='" + ethIp + "'><br><br>";

    html += "<label>Gateway</label><br>";
    html += "<input name='eth_gateway' value='" + ethGateway + "'><br><br>";

    html += "<label>Subnetzmaske</label><br>";
    html += "<input name='eth_subnet' value='" + ethSubnet + "'><br><br>";

    html += "<label>DNS</label><br>";
    html += "<input name='eth_dns' value='" + ethDns + "'><br><br>";

    html += "<button type='submit'>Speichern</button>";
    html += "</form>";

    html += "<p><a href='/'>Zurueck</a></p>";
    html += getHtmlFooter();

    server.send(200, "text/html", html);
}

// Einstellungen speichern
void Interface::handleSaveSettings() {
    preferences.begin("wall", false);

    if (server.hasArg("host_name")) {
        preferences.putString("host_name", server.arg("host_name"));
    }

    if (server.hasArg("password")) {
        preferences.putString("password", server.arg("password"));
    }

    if (server.hasArg("server_ip")) {
        preferences.putString("server_ip", server.arg("server_ip"));
    }

    if (server.hasArg("server_port")) {
        preferences.putUShort("server_port", server.arg("server_port").toInt());
    }

    if (server.hasArg("local_port")) {
        preferences.putUShort("local_port", server.arg("local_port").toInt());
    }

    if (server.hasArg("hb_timeout")) {
        unsigned long value = strtoul(server.arg("hb_timeout").c_str(), nullptr, 10);

        Serial.print("Speichere hb_timeout: ");
        Serial.println(value);
    
        preferences.putULong("hb_timeout", value);
    }

    preferences.putBool("eth_dhcp", server.hasArg("eth_dhcp"));

    if (server.hasArg("eth_ip")) {
        preferences.putString("eth_ip", server.arg("eth_ip"));
    }

    if (server.hasArg("eth_gateway")) {
        preferences.putString("eth_gateway", server.arg("eth_gateway"));
    }

    if (server.hasArg("eth_subnet")) {
        preferences.putString("eth_subnet", server.arg("eth_subnet"));
    }

    if (server.hasArg("eth_dns")) {
        preferences.putString("eth_dns", server.arg("eth_dns"));
    }

    preferences.end();

    String html = getHtmlHeader("Gespeichert");

    html += "<h1>Gespeichert</h1>";
    html += "<p>Bitte ESP neu starten, damit alle Einstellungen aktiv werden.</p>";

    html += "<form method='POST' action='/restart'>";
    html += "<button type='submit'>Jetzt neustarten</button>";
    html += "</form>";

    html += "<p><a href='/'>Zurueck</a></p>";
    html += getHtmlFooter();

    server.send(200, "text/html", html);
}

// Firmware-Update-Seite
void Interface::handleUpdate() {
    String html = getHtmlHeader("Firmware Update");

    html += "<h1>Firmware Update</h1>";

    html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
    html += "<input type='file' name='firmware' accept='.bin'><br><br>";
    html += "<button type='submit'>Update starten</button>";
    html += "</form>";

    html += "<p><a href='/'>Zurueck</a></p>";
    html += getHtmlFooter();

    server.send(200, "text/html", html);
}

// Firmware-Upload verarbeiten
void Interface::handleUpdateUpload() {
    HTTPUpload &upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        Serial.print("Update Start: ");
        Serial.println(upload.filename);

        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
        }

        return;
    }

    if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            Update.printError(Serial);
        }

        return;
    }

    if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) {
            Serial.print("Update erfolgreich: ");
            Serial.print(upload.totalSize);
            Serial.println(" Bytes");
        } else {
            Update.printError(Serial);
        }
    }
}

// Neustart auslösen
void Interface::handleRestart() {
    server.send(
        200,
        "text/html",
        "<html><body><h1>Neustart...</h1><p>ESP startet neu.</p></body></html>"
    );

    delay(500);
    ESP.restart();
}

// HTML-Kopf
String Interface::getHtmlHeader(String title) {
    String html;

    html += "<!DOCTYPE html>";
    html += "<html>";
    html += "<head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>" + title + "</title>";
    html += "</head>";
    html += "<body>";

    return html;
}

// HTML-Ende
String Interface::getHtmlFooter() {
    return "</body></html>";
}

// Checkbox-Status
String Interface::checked(bool value) {
    return value ? "checked" : "";
}

bool Interface::hasSelfTest() {
    return selfTestActive;
}
