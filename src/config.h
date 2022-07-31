#ifndef CONFIG_H
#define CONFIG_H

#include <inttypes.h>
#include <DallasTemperature.h>

#define KDeviceDefaultHostname "palju"
#define KDefaultAvailabilityTopic "/availability"
#define KDefaultCommandTopic "/cmd/#"
#define KDefaultStateTopic "/state"
#define KDefaultInfoTopic "/info"

#define KDefaultMqttPort 1883

#define KOffline "offline"
#define KOnline  "online"

#define KStatusUpdateInterval 60000
#define KTickerIntervalWifiConfig 200
#define KTickerIntervalMQTTConnect 1000
#define KMaxManualOverrideTime 3600000 // 1h

// Magic number to check if eeprom config is valid
#define KConfigValidationMagic 0x19278264

#define KWaterTankTemperatureSensorIndex 0
#define KOutsideTemperatureSensorIndex 1
#define KWaterInTemperatureSensorIndex 2 // water pipe bottom
#define KWaterOutTemperatureSensorIndex 3 // water pipe upper

const int KCacheIndex_WaterPipeUpperTemperature = 0;
const int KCacheIndex_WaterPipeLowerTemperature = 1;
const int KCacheIndex_OutsideTemperature = 2;
const int KCacheIndex_WaterTankTemperature = 3;

const DeviceAddress KWaterPipeUpperTemperatureSensorAddr = {0x28, 0xd3, 0xd3, 0x77, 0x91, 0x0d, 0x02, 0x0b};

//#define KWaterPipeUpperTemperatureSensorAddr {0x28, 0xd3, 0xd3, 0x77, 0x91, 0x0d, 0x02, 0x0b}
const DeviceAddress  KWaterPipeLowerTemperatureSensorAddr {0x28, 0xaa, 0xce, 0xfd, 0x1b, 0x13, 0x02, 0x6f};

const DeviceAddress  KOutsideTemperatureSensorAddr        {0x28, 0xde, 0x08, 0x43, 0x98, 0x0c, 0x00, 0xdf};
const DeviceAddress  KWaterTankTemperatureSensorAddr      {0x28, 0x39, 0x91, 0x79, 0xa2, 0x01, 0x03, 0xbb};

#endif