//// iot

static String iot::getMacAddr() {
    uint8_t mac[6];
    WiFi.macAddress(mac);

    String result;

    for (int i = 0; i < 6; ++i) {
        result += String(mac[i], 16);
        if (i < 5) result += ':';
    }

    return result;
}

static void iot::handleLoginResponse(const String& response) {
  
}

iot::DeviceState iot::startDevice(String dName, TopicInfo** ti, const int tiCnt) {
    Serial.println("Starting device...");
  
    deviceName = dName;
    topicInfos = ti;
    topicCnt = tiCnt;
  
    if (!network::connect()) {
        network::startHostMode();
    }

    Serial.println("Connected to WiFi.");
    Serial.println("Connecting to MQTT server...");

    if ( mqtt::connect() ) {
      Serial.println("Connected to MQTT server.");
      mqtt::init();
    } else {
        return MQTT_ERROR;
    }


    httpClient->begin(mqintServer);
    httpClient->addHeader("Content-Type", "text/plain");

    //DynamicJsonDocument jsonDoc(100000);
    DynamicJsonBuffer jb;
    JsonObject& jsonObj = jb.createObject();
    jsonObj["msgId"] = "0";
    jsonObj["deviceId"] = iot::deviceId;
    jsonObj["deviceName"] = iot::deviceName;

    DynamicJsonBuffer jbA;
    JsonArray& arr = jbA.createArray();
    for (int i = 0; i < topicCnt; i++) {
      //arr.add(serialized(topicInfos[i]->getJson()));
      arr.add(RawJson(topicInfos[i]->getJson()));
    }
    jsonObj["topicInfos"] = arr;
    
    String msg = "";
    // serializeJson(jsonDoc, msg);
    jsonObj.printTo(msg);

    Serial.println("msg:\n" + msg);
    int httpResponseCode = httpClient->POST(msg);

    if (httpResponseCode > 0) {
        String response = httpClient->getString();
        Serial.println("MqInt POST response: " + response);
        
        lastConfirmedTime = (long long)esp_timer_get_time() - 29 * 1000000;
        
        //handleLoginResponse(response);
        if (response != "1")
          return MQINT_ERROR;
    } else {
        Serial.println("Error connectiong to MQINT server.");
        return MQINT_ERROR;
    }
    
    httpClient->end();
    
    Serial.println("Device is up and running.");

    return RUNNING;
}

void iot::loop() {
  static int i = 0;
  static long long currTime;
  static const long confirmFreq = 30 * 1000000; /* sec * scale */

  if (!i) {
    if (network::isConnected()) {
      if (!mqtt::isConnected()) {
        Serial.println("MQTT lost connection. Retrying...");
        if (mqtt::connect()) {
          mqtt::init();
          Serial.println("Connected to MQTT server.");
       } else {
          Serial.println("Connection Failed.");
       }
      }
    } else {
      Serial.println("WiFi lost connection. Retrying...");
      if (network::connect())
          Serial.println("Connected to WiFi.");
      else
          network::startHostMode();
    }
  }

  currTime  = (long long)esp_timer_get_time();
  if (currTime - lastConfirmedTime >= confirmFreq) {
    lastConfirmedTime = currTime;
    mqtt::confirmMsg();
    Serial.println("Confirm msg sent.");
  }
  
  mqtt::mqClient.loop();
  
  i = (i+1) % 100;
}



//// network

static bool iot::network::connect() {
    Serial.println("Attempting connecting to WiFi...");
    
    WiFi.begin();
    delay(1000);
    for (int i = 0; i < 5; i++) {
        if (isConnected()) return true;
        delay(1000);
    }
    return isConnected();
}

static bool iot::network::isConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

static void iot::network::deleteCredentials() {
    connect();
    WiFi.disconnect();
}

static void iot::network::startHostMode() {
    Serial.println("Starting host mode...");
    
    WiFi.softAP(APssid, APpassword);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Access Piont started. IP address: ");
    Serial.println(IP);
    
    webServer->on("/", HTTP_GET, rootPage);
    webServer->begin();

    checkNetworks();
    
    Serial.println("Host mode stopped.");
    
    webServer->end();
    WiFi.softAPdisconnect(true);
}

static String iot::network::translateEncType(int encryptionType) {
    switch (encryptionType) {
        case (0):
        return "Open";
        case (1):
        return "WEP";
        case (2):
        return "WPA_PSK";
        case (3):
        return "WPA2_PSK";
        case (4):
        return "WPA_WPA2_PSK";
        case (5):
        return "WPA2_ENTERPRISE";
        default:
        return "UNKOWN";
    }
}

static void iot::network::checkNetworks() {
    //WiFi.mode(WIFI_STA);
    
    while (!isConnected()) {
        WiFi.disconnect();
        networkNum = WiFi.scanNetworks();
        if (networkNum > netBufSize) {
            free(networks);
            netBufSize = networkNum + 2;
            networks = (NetworkData*) malloc(netBufSize * sizeof(NetworkData));
        }
        
        Serial.print("Network list updated (");
        Serial.print(networkNum);
        Serial.println(")");
        
        for (int i = 0; i < networkNum; ++i) {
            networks[i].ssid = WiFi.SSID(i);
            networks[i].rssi = WiFi.RSSI(i);
            networks[i].encType = WiFi.encryptionType(i);
        
            Serial.println(networks[i].ssid + " " + networks[i].rssi + " " + networks[i].encType);
        }
        
        WiFi.begin();
        delay(7000);
    }
    Serial.println("Connected to WiFi.");
}

static void iot::network::rootPage(AsyncWebServerRequest *request) {
    int paramsNr = request->params();
    Serial.println(paramsNr);
    String ssid;
    String pw;
    int gotIt = 0;

    for(int i = 0; i < paramsNr; i++){
        AsyncWebParameter* param = request->getParam(i);
        if (param->name() == "ssid") {
            ssid = param->value();
            gotIt++;
        }
        else if (param->name() == "pw") {
            pw = param->value();
            gotIt++;
        }
    }
    if (gotIt == 2) {
        WiFi.begin(ssid.c_str(), pw.c_str());
        
        delay(1000);
        for (int i = 0; i < 5; i++) {
            if (isConnected()) break;
            delay(1000);
        }
        if (isConnected()) {
            request->send(200, "text/html", "Successfully connected to WiFi.");
            return;
        } else {
            request->send(200, "text/html", "Failed to connect to WiFi. <p><a href=\"/\">Back to page</a></p>");
            return;
        }
    }
    String wifis = "";
    if (networkNum <= 0) {
        wifis = "<h3>No networks avaible.</h3>";
    } else {
        wifis = "<h3>Available Networks:</h3>";
        for (int i = 0; i < networkNum; i++) {
            wifis += "<p><a class=\"network\" onclick=\"pickNetwork(this)\" id=\"" + 
            networks[i].ssid + "\">" + String(i+1) + ".: " + networks[i].ssid + 
            " (" + translateEncType(networks[i].encType) + ")</a></p>";
            //client.print(" (");
            //client.print(networks[i].rssi);
            //client.print(")");
        }
    }
    
    String msg = "<!DOCTYPE html>\
                <html lang=\"en\">\
                <head>\
                    <meta charset=\"UTF-8\">\
                    <title>ESP32</title>\
                </head>\
                \
                <body>\
                    <h1>ESP32 Connect to WiFi</h1>" + 
                wifis + 
                
                "<p> SSID: <input id=\"ssid\"> </p>\
                    <p> Password: <input id=\"password\" type=\"password\"> </p>\
                    <button class=\"button\" onclick=\"connect()\">Connect</button>\
                </body>\
                </html>\
                \
                \
                <script>\
                    function connect() {\
                        var ssid = document.getElementById(\"ssid\").value;\
                        var password = document.getElementById(\"password\").value;\
                \
                        window.location.replace(\"?ssid=\" + ssid + \"&pw=\" + password);\
                    }\
                    \
                    function pickNetwork(element) {\
                        document.getElementById(\"ssid\").value = element.id;\
                    }\
                </script>\
                \
                \
                <style>\
                    body {\
                        background: #404040;\
                        color: whitesmoke;\
                        padding: 0;\
                        margin: 0;\
                        font-size: 1.5em;\
                    }\
                \
                    html {\
                        font-family: Helvetica;\
                        display: inline-block;\
                        margin: 0 auto;\
                        text-align: center;\
                    }\
                \
                    h1 {\
                        background: #595959;\
                        margin: 0;\
                        padding-top: 15px;\
                        padding-bottom: 15px;\
                        color: #ffcd4d;\
                    }\
                \
                    .button {\
                        background-color: #529dcd;\
                        color: #404040;\
                        border: none;\
                        padding: 10px 20px;\
                        text-decoration: none;\
                        font-size: 20px;\
                        margin: 2px;\
                        cursor: pointer;\
                        font-weight: bold;\
                    }\
                \
                    .network {\
                        width: fit-content;\
                        padding: 5px 10px;\
                        cursor: pointer;\
                        background: #529dcd;\
                        border-radius: 2px;\
                        color: #404040;\
                    }\
                </style>\n";
    request->send(200, "text/html", msg.c_str());
}



//// mqtt


void iot::mqtt::callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  String t = String(topic);
  t = t.substring(iot::deviceId.length()+1);
  Serial.println(t);


  for (int i = 0; i < iot::topicCnt; i++) {
    if (strcmp(topicInfos[i]->topic.c_str(), t.c_str()) == 0) {
      char* s = (char*) malloc(length+1);
      s[length] = 0;
      for (int i = 0; i < length; i++) {
        s[i] = payload[i];
      }
      topicInfos[i]->callback(String(s));
    }
  }
}

static bool iot::mqtt::connect() {
    return ( mqClient.connect((char*) deviceId.c_str()) );
}

static void iot::mqtt::init() {
  String t;
  for (int i = 0; i < iot::topicCnt; i++)  {
    t = iot::deviceId + "/" + iot::topicInfos[i]->topic;
    mqClient.subscribe(t.c_str());
  }
}

static bool iot::mqtt::isConnected() {
    return mqClient.connected();
}

static void iot::mqtt::confirmMsg() {
  mqClient.publish("confirm", iot::deviceId.c_str());
}
