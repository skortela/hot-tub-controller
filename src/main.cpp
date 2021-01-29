#include <Arduino.h>
#include "config.h"
#include "appSettings.h"
#include <eepromStream.h>
#include <utils.h>
#include "Ticker.h"
//#include <DNSServer.h>
//#include <ESP8266WebServer.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESPAsyncWiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <ESP8266WiFi.h>
//#define MQTT_MAX_PACKET_SIZE 300
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#include "AsyncJson.h"
#include <ArduinoJson.h>

#include <EEPROM.h>

//#include <OneWire.h>
//#include <DallasTemperature.h>

#include "board.h"
#include "utils.h"

static const char KINDEX_HTML[] PROGMEM = "<!DOCTYPE html><html lang=\"en\"><head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <title>Palju</title> <style>.container{border-radius: 5px; background-color: #f2f2f2; padding: 20px;}/*label{padding: 12px 12px 12px 0; display: inline-block;}*/.col-25{float: left; width: 25%; margin-top: 6px;}.col-75{float: left; width: 75%; margin-top: 6px;}.col-val-left{float: left; min-width: 70px; margin-top: 6px;}.col-val-right{float: left; /*width: 30;*/ margin-top: 1px;}/* Clear floats after the columns */.row:after{content: \"\"; display: table; clear: both;}.btn-group input{background-color: #4CAF50; /* Green background */ border: 1px solid green; /* Green border */ color: white; /* White text */ padding: 2px 2px; /* Some padding */ cursor: pointer; /* Pointer/hand icon */ display: block; /* Make the buttons appear below each other */}.input-group div{/*border: 1px solid black;*/ /*padding: 5px 1px;*/ padding-top: 10px; padding-left: 2px; float: left; /* Float the buttons side by side */ /*display: block;*/ min-width: 50px;}code{font-size: 14px; display: block; font-family: monospace; white-space: pre; margin: 1em 0;}/*toggle button*/ /* The switch - the box around the slider */.switch{position: relative; display: inline-block; width: 60px; height: 34px;}/* Hide default HTML checkbox */.switch input{opacity: 0; width: 0; height: 0;}/* The slider */.slider{position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; -webkit-transition: .4s; transition: .4s;}.slider:before{position: absolute; content: \"\"; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; -webkit-transition: .4s; transition: .4s;}input:checked + .slider{background-color: #2196F3;}input:focus + .slider{box-shadow: 0 0 1px #2196F3;}input:checked + .slider:before{-webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px);}/* Rounded sliders */.slider.round{border-radius: 34px;}.slider.round:before{border-radius: 50%;}</style></head><body><h1>Palju</h1><div class=\"container\"> <div class=\"row\"> <div class=\"col-25\"> <label for=\"btn_mainpower\">Main power</label> </div><div class=\"col-75\"> <label class=\"switch\"> <input type=\"checkbox\" id=\"btn_mainpower\"> <span class=\"slider round\"></span> </label> </div></div><div class=\"row\"> <div class=\"col-25\"> <label for=\"id_temp_water\">Water temperature:</label> </div><div class=\"col-val-left\"> <div id=\"id_temp_water\"></div></div></div><div class=\"row\"> <div class=\"col-25\"> <label for=\"id_water_wanted\">Water wanted:</label> </div><div class=\"col-75\"> <div class=\"col-val-left\"> <div id=\"id_water_wanted\"></div></div><div class=\"col-val-right\"> <div class=\"btn-group\"> <svg height=\"65\" width=\"65\"> <polygon points=\"0,30 25,0 50,30\" style=\"fill:blue;stroke:black;stroke-width:1\" cursor=pointer id=\"btn_up\"/> <polygon points=\"0,35 25,65 50,35\" style=\"fill:blue;stroke:black;stroke-width:1\" cursor=pointer id=\"btn_down\"/> </svg> </div></div></div></div><div class=\"row\"> <svg id=\"id_flame\" width=\"40px\" height=\"40px\" viewBox=\"0 0 611.999 611.999\"> <path d=\"M216.02,611.195c5.978,3.178,12.284-3.704,8.624-9.4c-19.866-30.919-38.678-82.947-8.706-149.952c49.982-111.737,80.396-169.609,80.396-169.609s16.177,67.536,60.029,127.585c42.205,57.793,65.306,130.478,28.064,191.029c-3.495,5.683,2.668,12.388,8.607,9.349c46.1-23.582,97.806-70.885,103.64-165.017c2.151-28.764-1.075-69.034-17.206-119.851c-20.741-64.406-46.239-94.459-60.992-107.365c-4.413-3.861-11.276-0.439-10.914,5.413c4.299,69.494-21.845,87.129-36.726,47.386c-5.943-15.874-9.409-43.33-9.409-76.766c0-55.665-16.15-112.967-51.755-159.531c-9.259-12.109-20.093-23.424-32.523-33.073c-4.5-3.494-11.023,0.018-10.611,5.7c2.734,37.736,0.257,145.885-94.624,275.089c-86.029,119.851-52.693,211.896-40.864,236.826C153.666,566.767,185.212,594.814,216.02,611.195z\"/> </svg> </div><div class=\"row\"> <code id=\"id_json_raw\"></code> </div></div><script>const statusUrl=\"json\"; const cmdUrl=\"set\"; function refreshTimeout(){if (document.visibilityState==='visible'){refreshData(); setTimeout(refreshTimeout, 10000);}}function refreshData(){let settings={method: \"GET\"}; fetch(statusUrl, settings) .then(res=> res.json()) .then((json)=>{document.getElementById(\"btn_mainpower\").checked=json.main_power; document.getElementById(\"id_temp_water\").innerHTML=json.temp_water + \" &#8451\"; document.getElementById(\"id_water_wanted\").innerHTML=json.water_wanted + \" &#8451\"; document.getElementById(\"id_json_raw\").innerHTML=JSON.stringify(json, null, 4); if (json.flame){document.getElementById(\"id_flame\").style.fill=\"red\";}else if (json.heater_power){document.getElementById(\"id_flame\").style.fill=\"black\";}else{document.getElementById(\"id_flame\").style.fill=\"gray\";}});}function setWaterWanted(value){let jsonObj={water_wanted: value}; let settings={method: \"POST\", body: JSON.stringify(jsonObj), headers:{'Content-Type': 'application/json'},}; fetch(cmdUrl, settings).then(data=>{refreshData();});}document.getElementById('btn_up').onclick=function(){let value=parseFloat(document.getElementById(\"id_water_wanted\").innerHTML) + 0.5; setWaterWanted(value);}; document.getElementById('btn_down').onclick=function(){let value=parseFloat(document.getElementById(\"id_water_wanted\").innerHTML) - 0.5; setWaterWanted(value);}; document.getElementById('btn_mainpower').onclick=function(){let jsonObj={main_power: this.checked}; let settings={method: \"POST\", body: JSON.stringify(jsonObj), headers:{'Content-Type': 'application/json'},}; fetch(cmdUrl, settings).then(data=>{refreshData();});}; document.addEventListener(\"visibilitychange\", function(){if (document.visibilityState==='visible') refreshTimeout();}); refreshTimeout(); </script></body></html> ";



/**
 * Buttons:
 * - power  // PB0
 * - Heat up / selection up (D3, internal pull-up)         // PB1    // internal pullup
 * - Heat down / selection down (D8, internal pull-down)   // PB2
 * - menu/select (D0, internal pull-down)                  // PB3
 * 
 * 
 * Relays:
 * - Heatin     PA0 //(D5)
 * - Water pump PA1 //(D6)
 * - Ext 1      PA2 / (D7)
 * 
 * Leds:
 * - Heating (D4, internal led) // PA3
 * 
 * Water level sensor
 *  - connected to analog pin (A0)
 * 
 * MCP23017 extension module
 *  - I2C interface: SDA (D2/GPIO4), SLC (D1/GPIO5), (vcc, gnd)
 * 
 * Display
 *  - OLED
 *  - I2C interface: SDA (D2/GPIO4), SLC (D1/GPIO5), (vcc, gnd)
 * 
 * Inputs:
 *  - Heating flame ()
 *  - Heating error ()
 * 
 * Functions (mqtt / web ui / menu)
 *  - set wanted temperature
 *  - set heating offset/delta
 *  - set mode (off / anti-freeze / power save / normal)
 *  - view current temperature
 *  - view temperature history graph
 * 
 * Temperatures:
 *  - water output
 *  - water input
 *  - water tank
 *  - outside
 * 
 * Other:
 *  - Flame time counter (total minutes)
 *  - Afterheat, Jälkilämmön huomioon otto
 * 
 */

enum EPowerMode {
    mode_offline,
    mode_nofreeze,
    mode_power_save,
    mode_normal
};

Board m_board;

EPowerMode m_powerMode;

AppSettings m_settings;

AsyncEventSource m_events("/events");
AsyncWebServer server(80);
DNSServer dns;

WiFiClient m_espClient;
PubSubClient m_mqttClient(m_espClient);

unsigned long m_lastStatusUpdate;

bool m_shouldSaveConfig = false;
bool m_shouldReboot = false;

bool m_lastFlameStatus;
unsigned long m_flameStartupTime; // system millis at flame startup, undefined if no flame
unsigned long m_flameLastDuration; // flame duration at last cycle
uint64_t m_flameTotalDuration; // flame total duration without current flame duration
// forward declarations
void setMainPower(bool value);
void sendStatus();

void loadConfig() {
    EepromStream stream;
   
    stream.setUnderlyingData( const_cast<uint8_t*>(EEPROM.getConstDataPtr()), 150);

    if (stream.readInt32() != KConfigValidationMagic)
    {
        // no existing configuration
        return;
    }
    m_settings.clear();
    m_settings.setDeviceHostname(stream.readString());
    m_settings.m_mqtt_server = stream.readStringDup();
    m_settings.m_mqtt_port = stream.readInt32();
    m_settings.m_mqtt_user = stream.readStringDup();
    m_settings.m_mqtt_passw = stream.readStringDup();
    m_settings.print();

    m_settings.m_temperature_mode_anti_freeze = (double)stream.readInt32() / 10;
    m_settings.m_temperature_mode_normal = (double)stream.readInt32() / 10;
    m_settings.m_temperature_mode_power_save = (double)stream.readInt32() / 10;
    m_settings.m_temperature_delta = (double)stream.readInt32() / 10;

    m_settings.m_last_saved_flame_lifetime_seconds = stream.readInt64();

}

void saveConfig() {
    Serial.print("saveConfig");
    /*
    Serial.print("arg count: ");  Serial.println(wifiManager.server->args());
    for (int i=0; i< wifiManager.server->args(); i++)
    {
        Serial.print("key: "); Serial.println(wifiManager.server->argName(i)); 
        Serial.print("value: "); Serial.println(wifiManager.server->arg(i)); 
    }

    delay(5000);

    wifiManager.resetSettings();
    ESP.reset();
    delay(1000);

    return;
    */
    EepromStream stream;
    stream.setUnderlyingData( EEPROM.getDataPtr(), 150);
    
    stream.writeInt32(KConfigValidationMagic);

    stream.writeString(m_settings.m_deviceHostname);
    stream.writeString(m_settings.m_mqtt_server);
    stream.writeInt32(m_settings.m_mqtt_port);
    stream.writeString(m_settings.m_mqtt_user);
    stream.writeString(m_settings.m_mqtt_passw);


    stream.writeInt32(m_settings.m_temperature_mode_anti_freeze * 10);
    stream.writeInt32(m_settings.m_temperature_mode_normal * 10);
    stream.writeInt32(m_settings.m_temperature_mode_power_save * 10);
    stream.writeInt32(m_settings.m_temperature_delta * 10);

    stream.writeInt64(m_settings.m_last_saved_flame_lifetime_seconds);


    EEPROM.commit();
}

void resetConfig() {
    // clear first 100 bytes. it's enought to clear first int32 val, but erasing user sensitive data is more secure aproach.
    for (int i=0; i< 100; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
}

void initOTA() {
    //m_udp.begin(KUdpLocalPort);

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");

    Serial.print("FreeSketchSpace: ");
    Serial.println(ESP.getFreeSketchSpace());
    

    // No authentication by default
    ArduinoOTA.setPassword("your-secret-password");

    ArduinoOTA.onStart([]() {
        setMainPower(false);
        sendStatus();
        m_mqttClient.disconnect();
        
        //lcd.clear();
        //lcd.setCursor(0, 0);
        //lcd.print("OTA: Updating");
        Serial.println("OTA updating");
    });
    ArduinoOTA.onEnd([]() {
        /*lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("OTA: Update finished");
        lcd.setCursor(5, 1);
        lcd.print("Rebooting");*/
        Serial.println("Update finished, rebooting");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        int prog = progress / (total / 100);
        Serial.printf("Progress: %u%%\r", prog);
        
        //lcd.setCursor(6, 2);
        //lcd.print(prog);
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Update error [%u]: ", error);
        //Serial.printf("Error[%u]: ", error);
        //lcd.setCursor(3, 2);
        //lcd.print("Update error");
        //lcd.setCursor(3, 3);
        if (error == OTA_AUTH_ERROR) {Serial.println("Auth Failed");}
        else if (error == OTA_BEGIN_ERROR) {Serial.println("Begin Failed");}
        else if (error == OTA_CONNECT_ERROR) {Serial.println("Connect Failed");}
        else if (error == OTA_RECEIVE_ERROR) {Serial.println("Receive Failed");}
        else if (error == OTA_END_ERROR) {Serial.println("End Failed");}
        else {Serial.println("Unknown error");}
    });
    ArduinoOTA.begin();
}


// wanted temperature
float wantedTemperature() {
    return m_settings.m_temperature_mode_normal;
}

void setMainPower(bool value) {
    Serial.print("setMainPower: ");
    Serial.println(value);

    m_lastStatusUpdate = 0; // reset timer, so statusUpdate is sent
    m_board.setMainPower(value);
}

void sendInfo() {

    StaticJsonDocument<300> doc;
    JsonObject root = doc.to<JsonObject>();

    root["ip"] = WiFi.localIP().toString();
    root["mac"] = WiFi.macAddress();
    root["hostname"] = WiFi.hostname();
    
    //createSensorsArr(root);
    //char* buffer = new char[20];

    //IPAddress ip = WiFi.localIP();
    //sprintf(buffer, "ip: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

    char* jsonBuffer = new char[300];

    serializeJson(root, jsonBuffer, 300);
    Serial.println("sending info");
    Serial.println(jsonBuffer);

    m_mqttClient.publish(m_settings.m_infoTopic, jsonBuffer);
    delete jsonBuffer;
}

/*void sendState(bool refresh) {
    
}*/

void getStatus(char* pBuffer) {
    StaticJsonDocument<400> doc;
    JsonObject root = doc.to<JsonObject>();

    //const char* KStrOn = "ON";
    //const char* KStrOff = "OFF";

    /*
    if ( m_vallox.hasPower())
        root["power"] = KStrOn;
    else
        root["power"] = KStrOff;
    */
    
    //uint8_t devCount = sensors.getDeviceCount();
    // locate devices on the bus
    //Serial.print("Found ");
    //Serial.print(devCount, DEC);
    //Serial.println(" devices.");

    //JsonObject sensorsObj = root.createNestedObject("sensors");

    /**
{
    "main_power": true,
    "heating": false,
    "flame": false,
    "mode": "normal",
    "temp_water": 13.5,
    "temp_water_in": 12.9,
    "temp_water_out": 32,
    "temp_out": -5.4,
    "current_flame_time_minutes": 6,
    "total_flame_time_minutes": 135,
    "water_wanted": 13.5,
    "water_level": 80,
    "fuel_level": 50
}
     * 
     **/
    root["main_power"] = m_board.hasMainPower();
    root["heater_power"] = m_board.hasHeaterPower();
    root["water_pump_power"] = m_board.hasWaterPumpPower();
    root["ext_power"] = m_board.hasExtPower();

    bool hasFlame = m_board.hasFlame();
    root["flame"] = hasFlame;
    root["heating_error"] = m_board.hasHeatingError();
    root["water_wanted"] = wantedTemperature();

    if (hasFlame) {
        unsigned long flameDuration = (unsigned long)(millis() - m_flameStartupTime);
        root["flame_duration"] = flameDuration / 1000; // flame duration in seconds
        root["flame_duration_total"] = (long)((m_flameTotalDuration + flameDuration) / 1000);
    }
    else {
        root["flame_duration"] = m_flameLastDuration / 1000; // flame last duration in seconds
        root["flame_duration_total"] = (long)(m_flameTotalDuration / 1000);
    }
    

    switch (m_powerMode) {
        case mode_offline:
            root["mode"] = "off";
            break;
        case mode_nofreeze:
            root["mode"] = "no-freeze";
            break;
        case mode_power_save:
            root["mode"] = "power save";
            break;
        case mode_normal:
            root["mode"] = "normal";
            break;
        default:
            break;
    }

    if (m_board.getTemperatureWaterTank() != DEVICE_DISCONNECTED_C)
        root["temp_water"] = round1(m_board.getTemperatureWaterTank()); //round1(sensors.getTempCByIndex(KWaterTankTemperatureSensorIndex));
    else
        root["temp_water"] = nullptr;
    
    if (m_board.getTemperatureSensorWaterIn() != DEVICE_DISCONNECTED_C)
        root["temp_water_in"] = round1(m_board.getTemperatureSensorWaterIn());
    else
        root["temp_water_in"] = nullptr;

    if (m_board.getTemperatureSensorWaterOut() != DEVICE_DISCONNECTED_C)
        root["temp_water_out"] = round1(m_board.getTemperatureSensorWaterOut());
    else
        root["temp_water_out"] = nullptr;

    if (m_board.getTemperatureOutside() != DEVICE_DISCONNECTED_C)
        root["temp_outside"] = round1(m_board.getTemperatureOutside());
    else
        root["temp_outside"] = nullptr;

    //root["current_flame_time_minutes"] = 6;
    //root["total_flame_time_minutes"] = 135;
    //root["water_wanted"] = m_settings.m_temperature_mode_normal;
    //root["water_level"] = 80;
    //root["fuel_level"] = 50;

    serializeJson(root, pBuffer, 400);
    //Serial.println("sending info");
    //Serial.println(pBuffer);

}

void sendStatus() {

    // board is not ready yeat, don't send status
    if (!m_board.isReady())
        return;

    char* jsonBuffer = new char[400];

    getStatus(jsonBuffer);

    bool ok = m_mqttClient.publish(m_settings.m_stateTopic, jsonBuffer);
    delete jsonBuffer;
    if (!ok) {
        Serial.println("failed to publish!");
    }
    m_lastStatusUpdate = millis();
}

void setWaterWanted(double value) {
    Serial.println("setWaterWanted: ");
    Serial.println(value);

    // sanity check
    if (value < 0.01 || value > 50) {
        Serial.println("Invalid value!");
        Serial.println(value);
        return;
    }
    
    m_settings.m_temperature_mode_normal = value;
    saveConfig();
    m_lastStatusUpdate = 0;
}

void sendSensors() {
    DallasTemperature* pSensor = m_board.dallasTemperature();
    const uint8_t count = pSensor->getDeviceCount();

    
    StaticJsonDocument<200+JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(KMAX_DALLAS_SENSOR_COUNT) + KMAX_DALLAS_SENSOR_COUNT*JSON_OBJECT_SIZE(3)> doc;
    JsonObject root = doc.to<JsonObject>();

    JsonArray sensorsArr = root.createNestedArray("sensors");

    DeviceAddress addr;
    char addrStr[19];

    //pSensor->requestTemperatures();
    //root["TT"] = m_board.getTT();
    root["delta"] = m_settings.m_temperature_delta;
    
    for (uint8_t i=0; i< count; i++) {
        //JsonObject sensorObj = sensorsArr.createNestedObject();
        //sensorObj["test"] = "jeah";
        if (pSensor->getAddress(addr, i)) {
            addressToStr(addr, addrStr);

            JsonObject sensorObj = sensorsArr.createNestedObject();
            //sensorObj["address"] = "0x0102030405060708";//addrStr;
            sensorObj["address"] = addrStr;
            sensorObj["temperature"] = round1(pSensor->getTempC(addr));
            if (pSensor->isConnected(addr))
                sensorObj["available"].set(true);
            else
                sensorObj["available"].set(false);
        }
    }

    const int KBufferSize = 21+10 + count * 80;
    char* pBuffer = new char[KBufferSize];
    serializeJson(root, pBuffer, KBufferSize);

    bool ok = m_mqttClient.publish("palju/sensors" /*m_settings.m_stateTopic*/, pBuffer);
    delete pBuffer;
    if (!ok) {
        Serial.println("failed to publish!");
    }
}

void doTest()
{
    DeviceAddress addr = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
    DeviceAddress addr2;
    //

    char addrStr[19];
    addressToStr(addr, addrStr);
    String str = "addr: " + String(addrStr);

    uint64toUInt8Array(0x0001020304050607ULL, addr2);
    
    addressToStr(addr2, addrStr);
    str += "\naddr2: " + String(addrStr);

    //arrayToUint64(addr);
    
    bool ok = m_mqttClient.publish("palju/test", str.c_str());
}

void mqtt_message_received(char* topic, byte* payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    // set null termination as it may be missing
    payload[length] = 0;
    const char* strPayload = (const char*) payload;
    Serial.println(strPayload);

    size_t len = strlen(m_settings.m_commandListen) -1; // remove last '#' char
    if (strlen(topic) <= len)
        return;


    const char* topicCmd = topic + len;
    //char* pos = strrchr(topic, '/');
    //if (!pos)
    //    return;

    //pos++;

    char* commandEndPos = strrchr(topicCmd, '/');
    int commandLen;
    if (!commandEndPos)
        commandLen = strlen(topicCmd);
    else
        commandLen = commandEndPos-topicCmd-1;

    
    Serial.println(topicCmd);

    Serial.println(commandLen);

    if (strncmp(topicCmd, "info", commandLen) == 0) {
        sendInfo();
    }
    else if (strncmp(topicCmd, "sensors", commandLen) == 0) {
        sendSensors();
    }
    else if (strncmp(topicCmd, "status", commandLen) == 0) {
        sendStatus();
    }
    else if (strncmp(topicCmd, "test", commandLen) == 0) {
        doTest();
    }
    else if (strncmp(topicCmd, "set_main_power", commandLen) == 0) {
        if (strcasecmp(strPayload, "true") == 0) {
            setMainPower(true);
        }
        else if (strcasecmp(strPayload, "false") == 0) {
            setMainPower(false);
        }
    }
    else if (strncmp(topicCmd, "set_water_pump_power", commandLen) == 0) {
        if (strcasecmp(strPayload, "true") == 0) {
            m_board.setWaterPumpPower(true);
        }
        else if (strcasecmp(strPayload, "false") == 0) {
            m_board.setWaterPumpPower(false);
        }
    }
    else if (strncmp(topicCmd, "set_ext_power", commandLen) == 0) {
        if (strcasecmp(strPayload, "true") == 0) {
            m_board.setExtPower(true);
        }
        else if (strcasecmp(strPayload, "false") == 0) {
            m_board.setExtPower(false);
        }
    }
    else if (strncmp(topicCmd, "set_water_wanted", commandLen) == 0) {
        double value = atof(strPayload);
        setWaterWanted(value);
    }
    else if (strncmp(topicCmd, "restart", commandLen) == 0) {
        m_shouldReboot = true;
        //ESP.restart();
    }
    else if (strncmp(topicCmd, "reset_flame_counter", commandLen) == 0) {
        m_flameTotalDuration = 0;
        m_flameLastDuration = 0;
        m_settings.m_last_saved_flame_lifetime_seconds = 0;
        saveConfig();
        m_lastStatusUpdate = 0;
    }
}

void configModeCallback(AsyncWiFiManager*) {
    Serial.println("Entering config mode");
    //m_led_ticker.attach_ms(KTickerIntervalWifiConfig, blink);
}
//callback notifying us of the need to save config
void saveConfigCallback () {
    Serial.println("Should save config");
    m_shouldSaveConfig = true;
}
void wifiSetup()
{
    WiFi.hostname(m_settings.m_deviceHostname);

    char strPort[10];
    sprintf(strPort, "%d", m_settings.m_mqtt_port);

    // The extra parameters to be configured (can be either global or just in the setup)
    // After connecting, parameter.getValue() will get you the configured value
    // id/name placeholder/prompt default length

    AsyncWiFiManagerParameter custom_deviceHostname("clientname", "clientname", m_settings.m_deviceHostname, 40);

    AsyncWiFiManagerParameter custom_mqtt_server("server", "mqtt server", m_settings.m_mqtt_server, 40);
    AsyncWiFiManagerParameter custom_mqtt_port("port", "mqtt port", strPort, 6);
    //WiFiManagerParamInt custom_mqtt_port("port", "mqtt port", m_settings.m_mqtt_port, 6);
    AsyncWiFiManagerParameter custom_mqtt_user("username", "username", m_settings.m_mqtt_user, 40);
    AsyncWiFiManagerParameter custom_mqtt_passw("password", "password", m_settings.m_mqtt_passw, 40);

    //AsyncWiFiManager wifiManager;
    AsyncWiFiManager wifiManager(&server,&dns);

    //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(configModeCallback);
    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    if (WiFi.SSID() != "")
    {
        Serial.println("Saved SSID found, set timeout for config portal");
        wifiManager.setConfigPortalTimeout(300); // 5 minutes timeout
    }

    //add all your parameters here
    
    wifiManager.addParameter(&custom_deviceHostname);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_passw);

    //wifiManager.addParameter(&custom_onewire_pin);
   


    if(!wifiManager.autoConnect(m_settings.m_deviceHostname)) {
        Serial.println("failed to connect and hit timeout");
        //reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(1000);
    }

    Serial.println("autoconnect continue");

     //wifiManager.addParameter(&custom_onewire_pin);
    /*if(wifiManager.server->hasArg("cars")) {
        String value = wifiManager.server->arg("cars");
        Serial.println("value: " + value);
    }*/

    


    
    m_settings.clear();
    m_settings.setDeviceHostname(custom_deviceHostname.getValue());
    m_settings.m_mqtt_server = strdup(custom_mqtt_server.getValue());
    m_settings.m_mqtt_port = atoi(custom_mqtt_port.getValue());
    //m_settings.m_mqtt_port = custom_mqtt_port.getValue();
    m_settings.m_mqtt_user = strdup(custom_mqtt_user.getValue());
    m_settings.m_mqtt_passw = strdup(custom_mqtt_passw.getValue());

    //m_settings.m_oneWirePin = custom_onewire_pin.getValue();

    m_settings.print();


    if (m_shouldSaveConfig) {
        // Save custom params
        saveConfig();
        Serial.println("config saved");
    }

    Serial.println("Connected");
    Serial.println("local ip");
    Serial.println(WiFi.localIP());

    //m_led_ticker.detach();
}

void mqtt_reconnect() {
    //m_led_ticker.attach_ms(KTickerIntervalMQTTConnect, blink);
    if (!m_mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "PaljunAivot";
        // Attempt to connect
        if (m_mqttClient.connect(clientId.c_str(), m_settings.m_mqtt_user, m_settings.m_mqtt_passw, m_settings.m_availabilityTopic, MQTTQOS0, true, KOffline)) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            m_mqttClient.publish(m_settings.m_availabilityTopic, KOnline, true);
            // ... and resubscribe
            m_mqttClient.subscribe(m_settings.m_commandListen);

            sendInfo();
            //sendState();
            sendStatus();
        } else {
            Serial.print("failed, rc=");
            Serial.println(m_mqttClient.state());
            //Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            //delay(5000);
        }
        
    }
    //m_led_ticker.detach();
}

void notFound(AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
        request->send(200);
    } else {
        request->send(404, "text/plain", "Not found");
    }
}

void buttonPressed(uint8_t pin) {
    Serial.print("buttonPressed: ");
    Serial.println(pin);

    if (pin == KPIN_BUTTON_MAIN_POWER) {
        Serial.println("buttonPressed: MainPower");
        // toggle main power
        setMainPower( !m_board.hasMainPower() );
    }
    else if (pin == KPIN_BUTTON_UP) {
        setWaterWanted(wantedTemperature() + 0.5f);
    }
    else if (pin == KPIN_BUTTON_DOWN) {
        setWaterWanted(wantedTemperature() - 0.5f);
    }
    else if (pin == KPIN_INPUT_HEATING_FLAME) {
        
    }
    else if (pin == KPIN_INPUT_HEATING_ERROR) {

    }
    m_events.send("my event content","myevent",millis());
}

void setup() {
    Serial.begin(115200);
    Serial.println("\n\nStarted");

    EEPROM.begin(300);

    delay(1000);
    m_board.begin();
    
    //resetConfig();

    //WiFiManager wifiManager;
    //wifiManager.resetSettings();

    m_lastStatusUpdate = 0;
    //m_lastSensorsUpdate = 0;


    loadConfig();


    Serial.print("last used SSID: ");
    Serial.println(WiFi.SSID());

    //sendState(true);

    wifiSetup();
    initOTA();

    m_mqttClient.setBufferSize(500);
    m_mqttClient.setServer(m_settings.m_mqtt_server, m_settings.m_mqtt_port);
    m_mqttClient.setCallback(mqtt_message_received);
    delay(1000);
    Serial.println("Started!");
    digitalWrite(D4, HIGH);

    m_events.onConnect([](AsyncEventSourceClient *client){
        if(client->lastId()){
            Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
        }
        //send event with message "hello!", id current millis
        // and set reconnect delay to 1 second
        client->send("hello!",NULL,millis(),1000);
    });
    server.addHandler(&m_events);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", KINDEX_HTML);
    });
    server.on("/json", HTTP_GET, [](AsyncWebServerRequest *request){
        char* jsonBuffer = new char[300];
        getStatus(jsonBuffer);
        request->send(200, "application/json", jsonBuffer);
        delete jsonBuffer;
        
    });

    // Simple Firmware Update Form
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
    });
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
        m_shouldReboot = !Update.hasError();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", m_shouldReboot?"OK":"FAIL");
        response->addHeader("Connection", "close");
        request->send(response);
    },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
        Serial.printf("Update Start: %s\n", filename.c_str());
        Update.runAsync(true);
        if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
            Update.printError(Serial);
        }
    }
    if(!Update.hasError()){
        if(Update.write(data, len) != len){
            Update.printError(Serial);
        }
    }
    if(final){
        if(Update.end(true)){
            Serial.printf("Update Success: %uB\n", index+len);
        } else {
            Update.printError(Serial);
        }
    }
    });
    
    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/set", [](AsyncWebServerRequest *request, JsonVariant &json) {
        const JsonObject& jsonObj = json.as<JsonObject>();
        if (jsonObj.containsKey("main_power")) {
            setMainPower(jsonObj["main_power"]);
        }
        if (jsonObj.containsKey("water_wanted")) {
            m_settings.m_temperature_mode_normal = jsonObj["water_wanted"];
            saveConfig();
        }
        Serial.println("got set request");

        char* jsonBuffer = new char[300];
        getStatus(jsonBuffer);
        request->send(200, "application/json", jsonBuffer);
        delete jsonBuffer;
    });
    server.addHandler(handler);

    server.onNotFound(notFound);

    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    server.begin();

    m_board.setButtonPressedCallback(buttonPressed);

    m_lastFlameStatus = false;
    m_flameLastDuration = 0;
    m_flameTotalDuration = m_settings.m_last_saved_flame_lifetime_seconds * 1000;
}

bool isTooHotWater() {
    // check temperature from two different sensors. If either one is too hot -> too hot
    return (m_board.getTemperatureWaterTank() > wantedTemperature() 
        || m_board.getTemperatureSensorWaterIn() > wantedTemperature());
}
bool isTooColdWater() {
    // check temperature from two different sensors. If both are below limit -> too cold
    return (m_board.getTemperatureWaterTank() < wantedTemperature() - m_settings.m_temperature_delta
     && m_board.getTemperatureSensorWaterIn() < wantedTemperature() - m_settings.m_temperature_delta);
}

// returns current flame duration, or 0 if no flame detectec
int currentFlameDuration()
{
    if (m_lastFlameStatus)
        return (unsigned long)(millis() - m_flameStartupTime);
    else
        return 0;
}

void loop() {
    if(m_shouldReboot){
        Serial.println("Rebooting...");
        delay(100);
        ESP.restart();
    }

    ArduinoOTA.handle();
    if (!m_mqttClient.connected()) {
        mqtt_reconnect();
        digitalWrite(LED_BUILTIN, LOW); // Turn led on
    }
    m_mqttClient.loop();
    

    m_board.loop();
    
    if (m_board.hasMainPower()) {
        if (m_board.hasHeaterPower()) {
            // temperature too high, and flame has been on at least 30 sec.
            if (isTooHotWater() && currentFlameDuration() > 30000) {
                // stop heating
                m_board.setHeaterPower(false);
                m_lastStatusUpdate = 0;
            }
        }
        else {
            // temperature too low
            if (m_board.isReady() && isTooColdWater()) {
                // start heating
                m_board.setHeaterPower(true);
                m_lastStatusUpdate = 0;
            }
        }
    }
    else {
        if (m_board.hasHeaterPower()) {
            m_board.setHeaterPower(false);
            m_lastStatusUpdate = 0;
        }
    }

    if (m_board.hasFlame()) {
        // calculate time
        if (!m_lastFlameStatus) {
            // flame started.
            m_lastFlameStatus = true;
            m_flameStartupTime = millis();
            m_lastStatusUpdate = 0;
        }
    }
    else if (m_lastFlameStatus) { // flame just stopped
        m_flameLastDuration = (unsigned long)(millis() - m_flameStartupTime);
        m_lastFlameStatus = false;
        m_flameTotalDuration += m_flameLastDuration;
        m_lastStatusUpdate = 0;

        m_settings.m_last_saved_flame_lifetime_seconds = m_flameTotalDuration / 1000;
        saveConfig();
    }

    if (m_lastStatusUpdate == 0 || (unsigned long)(millis() - m_lastStatusUpdate) > KStatusUpdateInterval) {
        sendStatus();
    }

}
