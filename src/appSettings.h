#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <inttypes.h>

class AppSettings {
    public:
        AppSettings();
        ~AppSettings();

        void clear();

        void print();

        void setDeviceHostname(const char* pHostname);

        

    public:
        char* m_deviceHostname;
        char* m_mqtt_server;
        int m_mqtt_port;
        char* m_mqtt_user;
        char* m_mqtt_passw;

        char* m_availabilityTopic;
        char* m_commandListen;
        char* m_stateTopic;
        char* m_infoTopic;
        //int m_oneWirePin;

        double m_temperature_mode_normal;
        double m_temperature_mode_anti_freeze;
        double m_temperature_mode_power_save;
        double m_temperature_delta;
        
        int64_t m_last_saved_flame_lifetime_seconds;

    private:
};

#endif