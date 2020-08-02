#ifndef IOT_API_H
#define IOT_API_H


#include <WiFi.h>
#include <ESPAsyncWebServer.h>

#include <PubSubClient.h>
#include <HTTPClient.h>

#include <ArduinoJson.h>

void blink() {
  digitalWrite(2, HIGH);
}

namespace iot {
    /*struct TopicInfo {
      TopicInfo(String topic, void (*callback)(const String&)) 
        : topic(topic), callback(callback) {}
      
      String topic;
      void (*callback)(const String& response);
    };*/
    
    enum TopicMode {SWITCH=0, OPTIONS=1, SLIDER=2};
  
    struct TopicInfo {
      TopicInfo(const String topic) : topic(topic) {}
      
      const String topic;
      virtual void callback(const String& response) const = 0;
      virtual String getJson() const = 0;
    };

    struct TopicSwitch : public TopicInfo {
      TopicSwitch(String topic, void (*on)(), void (*off)())
          : TopicInfo(topic), on(on), off(off) {}
          
      void (*on)();
      void (*off)();

      void callback(const String& response) const override {
        Serial.println("TopicSwitch callback called: " + response);
        if (strcmp(response.c_str(), "1") == 0)
          this->on();
        else if (strcmp(response.c_str(), "0") == 0)
          this->off();
      }

      String getJson() const override {
        //DynamicJsonDocument jsonDoc(500);
        DynamicJsonBuffer jb;
        JsonObject& jsonObj = jb.createObject();
        jsonObj["mode"] = String(TopicMode::SWITCH);
        jsonObj["topic"] = topic;

        String str = "";
        //serializeJson(jsonDoc, str);
        jsonObj.printTo(str);
        return str;
      }
    };

    struct TopicOption {
      TopicOption(String option, void (*f)())
          : option(option), f(f) {}
          
      const String option;
      void (*f)();
    };

    struct TopicOptions : public TopicInfo {
      TopicOptions(const String topic, TopicOption** topicOptions, const int optionCnt)
          : TopicInfo(topic), topicOptions(topicOptions), optionCnt(optionCnt) {}
          
      TopicOption** topicOptions;
      const int optionCnt;

      void callback(const String& response) const override {
        Serial.println("TopicOptions callback called: " + response);
        int i = 0;
        while (i < optionCnt && !(topicOptions[i]->option == response))
          i++;
        if (i < optionCnt)
          topicOptions[i]->f();
      }

      String getJson() const override {
        //DynamicJsonDocument jsonDoc(50000);
        DynamicJsonBuffer jb;
        JsonObject& jsonObj = jb.createObject();
        jsonObj["mode"] = String(TopicMode::OPTIONS);
        jsonObj["topic"] = topic;

        DynamicJsonBuffer jbA;
        JsonArray& arr = jbA.createArray();
        for (int i = 0; i < optionCnt; i++) {
          arr.add(topicOptions[i]->option);
        }
        jsonObj["options"] = arr;

        String str = "";
        //serializeJson(jsonDoc, str);
        jsonObj.printTo(str);
        return str;
      }
    };

    struct TopicSlider : public TopicInfo {
      TopicSlider(const String topic, void (*f)(const double),
          const double min, const double max, const double interval = 1) 
          : TopicInfo(topic), f(f), min(min), max(max - interval*(((max-min)/interval) - (int)((max-min)/interval)) ), interval(interval) {}
          
      void (*f)(const double value);
      const double min, max, interval;

      void callback(const String& response) const override {
        Serial.println("TopicSlider callback called: " + response);
        try {
          double v = response.toFloat();
          if (v >= min && (v <= max || abs(v - max) < 0.0001))
            f(v);
          else
            Serial.println("Incorrect size (out of min-max range).");
            Serial.println(min);
            Serial.println(v);
            Serial.println(max);
        } catch (...) {
          Serial.println("Incorrect response (double) format.");
        }
      }

      String getJson() const override {
        DynamicJsonBuffer jb;
        JsonObject& jsonObj = jb.createObject();
        jsonObj["mode"] = String(TopicMode::SLIDER);
        jsonObj["topic"] = topic;
        jsonObj["min"] = min;
        jsonObj["max"] = max;
        jsonObj["interval"] = interval;
        jsonObj["value"] = min;

        String str = "";
        //serializeJson(jsonDoc, str);
        jsonObj.printTo(str);
        return str;
      }
    };
  
    enum DeviceState {RUNNING, MQINT_ERROR, MQTT_ERROR};
    
    static String getMacAddr();

    static String deviceName;
    static const String deviceId = "ESP32-" + getMacAddr();
    static const String mqintServer = "https://mqint.tharci.fail/iot";
    static HTTPClient* httpClient = new HTTPClient();

    long long lastConfirmedTime;

    static TopicInfo** topicInfos;
    static int topicCnt;

    static void handleLoginResponse(const String& response);
    
    DeviceState startDevice(String dName, TopicInfo** ti = 0, const int tiCnt = 0);
  
    void loop();

    namespace network {
        struct NetworkData {
            String ssid, rssi;
            int encType;
        };

        static const char* APssid     = "ESP32-Access-Point";
        static const char* APpassword = "87654321";
        static String header;

        static int netBufSize = 20;
        static NetworkData* networks = 
            (NetworkData*) malloc(netBufSize * sizeof(NetworkData));
        static int networkNum = 0;
        static AsyncWebServer* webServer = new AsyncWebServer(80);

        static bool connect();
        static bool isConnected();
        static void deleteCredentials();

        static void startHostMode();
        static void stopHostMode();

        static String translateEncType(int encryptionType);
        static void checkNetworks();
        static void rootPage(AsyncWebServerRequest *request);
    }

    namespace mqtt {
        static const char* ip = "mqtt.tharci.fail";
        static const int port = 8883;
        
        static void callback(char* topic, byte* buff, unsigned int buffLen);

        static WiFiClient wifiClient;
        static PubSubClient mqClient(ip, port, callback, wifiClient);

        static void init();
        static bool connect();
        static bool isConnected();

        static void confirmMsg();
    }
}

#endif
