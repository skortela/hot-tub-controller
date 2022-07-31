#ifndef BOARD_H
#define BOARD_H

#include <Adafruit_MCP23017.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "averageTemperature.h"

#define KMCP_PIN_A0 	0 // GPA0...GPA7 -> Pin id: 0...7,  used as outputs
#define KMCP_PIN_B0 	8 // GPB0...GPB7 -> Pin id: 8...15, used as inputs

// All inputs are pulled high by default
#define KPIN_BUTTON_MAIN_POWER  KMCP_PIN_B0
#define KPIN_BUTTON_UP          KMCP_PIN_B0+1
#define KPIN_BUTTON_DOWN        KMCP_PIN_B0+2

#define KPIN_INPUT_HEATING_FLAME KMCP_PIN_A0+5
#define KPIN_INPUT_HEATING_ERROR KMCP_PIN_A0+7

#define KMAX_DALLAS_SENSOR_COUNT 5

class Adafruit_MCP23017;
//typedef void (*func)(void) ButtonPressedHandlerFunction;
//typedef std::function<void(AsyncWebServerRequest *request)> ArRequestHandlerFunction;

/**
 *  All hardware related input & output.
 */
class Board
{
public:
    Board();
    ~Board();

    void begin();

    // Call periodically
    void loop();
    bool isReady() const;

    bool hasFlame() const;
    bool hasHeatingError() const;
    bool hasMainPower() const;
    bool hasManualOverride() const;
    bool hasHeaterPower() const;
    bool hasWaterPumpPower() const;
    bool hasExtPower() const;

    void setMainPower(bool power);
    void setManualOverride(bool override);
    void setHeaterPower(bool power);
    void setWaterPumpPower(bool power);
    void setExtPower(bool power);

    float getTemperatureWaterTank() const;
    float getTemperatureSensorWaterIn() const;
    float getTemperatureSensorWaterOut() const;
    float getTemperatureOutside() const;

    float getTT() const;

    void setButtonPressedCallback(void (*func)(uint8_t) );

    DallasTemperature* dallasTemperature();
private:
    void checkButton(uint8_t pin, uint8_t indexStore);
    void checkPin(uint8_t pin, uint8_t indexStore);

private:
    mutable Adafruit_MCP23017 m_mcp;
    OneWire m_oneWire;
    mutable DallasTemperature m_sensors;
    bool m_mainPower;
    bool m_manualOverride;
    unsigned long m_manualOverrideStartupTime;

    void (*m_buttonPressedCallback)(uint8_t) = NULL;
    uint8_t m_inputLastState[10];
    uint8_t m_inputLastAcceptedState[10];
    unsigned long m_lastDebounceTime[10];
    unsigned long m_lastTemperatureRequestTime;
    bool m_hasTemperaturesRead;
    AverageTemperature m_temperatureCache[KMAX_DALLAS_SENSOR_COUNT];
    bool m_cacheReady;
};

#endif // BOARD_H