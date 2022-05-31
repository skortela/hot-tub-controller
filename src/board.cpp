#include "board.h"
#include "config.h"
#include "utils.h"


#define KMCP_PIN_RELAY_HEATER      KMCP_PIN_A0     // CH1 relay, GPA0
#define KMCP_PIN_RELAY_WATERPUMP   KMCP_PIN_A0+1   // CH2 relay, GPA1
#define KMCP_PIN_RELAY_EXT         KMCP_PIN_A0+2   // CH3 relay, GPA2
#define KMCP_PIN_RELAY_LED         KMCP_PIN_A0+3   // CH3 relay, GPA3

#define ONE_WIRE_PIN D4
#define KTEMPERATURE_RESOLUTION 12 // used temperature resolution (bits)


//#define KPIN_BUTTON_UP D3
//#define KPIN_BUTTON_DOWN D8
#define KPIN_BUTTON_SELECT D0

#define KPIN_HEATIN_LED D4
#define KPIN_RELAY_HEATIN D5
#define KPIN_RELAY_WATER_PUMP D6
#define KPIN_RELAY_EXT1 D7

#define KPIN_WATER_LEVEL A0
#define KPIN_OLED_SDA D2
#define KPIN_OLED_SLC D1

/*int cacheIndexOfAddress(const DeviceAddress& addr)
{
    if (memcmp(addr, KWaterPipeUpperTemperatureSensorAddr, sizeof(DeviceAddress)) == 0)
    {

    }
}*/


unsigned long KDebounceDelay = 50; // the debounce time; increase if the output flickers

Board::Board()
{
    m_sensors.setOneWire(&m_oneWire);   
}
Board::~Board()
{}

void Board::begin()
{
    m_mcp.begin();
    // set all channel A pins to output
    for (uint8_t i=KMCP_PIN_A0; i<KMCP_PIN_A0+5; i++) {
        
        m_mcp.pinMode(i, OUTPUT);
        m_mcp.digitalWrite(i, LOW);
    }
    m_mainPower = false;

    m_mcp.pinMode(KPIN_INPUT_HEATING_FLAME, INPUT);
    m_mcp.pullUp(KPIN_INPUT_HEATING_FLAME, HIGH);
    m_mcp.pinMode(KPIN_INPUT_HEATING_ERROR, INPUT);
    m_mcp.pullUp(KPIN_INPUT_HEATING_ERROR, HIGH);


    // set all channel B pins to input with internal pullup
    for (uint8_t i=KMCP_PIN_B0; i<KMCP_PIN_B0+8; i++) {
        m_mcp.pinMode(i, INPUT);
        m_mcp.pullUp(i, HIGH);  // turn on a 100K pullup internally

        m_lastDebounceTime[i-KMCP_PIN_B0] = millis();
        m_inputLastAcceptedState[i-KMCP_PIN_B0] = HIGH; // high is non-pressed state
    }

    m_oneWire.begin(ONE_WIRE_PIN);
    m_sensors.begin();
    m_sensors.setWaitForConversion(false);
    m_sensors.setResolution(KTEMPERATURE_RESOLUTION);

    Serial.print("getDeviceCount: ");
    Serial.println(m_sensors.getDeviceCount());

    m_lastTemperatureRequestTime = 0;
    m_hasTemperaturesRead = false;
    m_cacheReady = false;

    //sensors.setResolution(11);
    //pinMode(LED_BUILTIN, OUTPUT);
}

void Board::checkButton(uint8_t pin, uint8_t indexStore)
{
    uint8_t state = m_mcp.digitalRead(pin);

    if (m_inputLastState[indexStore] != state) {
        // State changed, reset last changed time
        m_lastDebounceTime[indexStore] = millis();
    }

    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    if (millis() - m_lastDebounceTime[indexStore] > KDebounceDelay) {

        // if accepted state has changed
        if (m_inputLastAcceptedState[indexStore] != state) {
            m_inputLastAcceptedState[indexStore] = state;
        
            if (state == LOW) { // LOW = button pressed
                // call the callback!
                if ( m_buttonPressedCallback != NULL) {
                    m_buttonPressedCallback(pin);
                }
            }

        }
    }

    // save the state for next round
    m_inputLastState[indexStore] = state;
}


void Board::loop()
{
    for (uint8_t i=KMCP_PIN_B0; i<KMCP_PIN_B0+8/*+2*/; i++) {
        /*uint8_t pin;
        if (i < KMCP_PIN_B0+8) {
            pin = i - KMCP_PIN_B0;
            index = pinMode;
        }
        else {
            if (i == KMCP_PIN_B0)
        }*/
        uint8_t pin = i;
        uint8_t index = i - KMCP_PIN_B0;
        checkButton(pin, index);
        
    }

    //checkPin(KPIN_INPUT_HEATING_FLAME, 8);
    //checkPin(KPIN_INPUT_HEATING_ERROR, 9);

    // update temperatures
    if ((unsigned long)(millis() - m_lastTemperatureRequestTime) > 5000) {
        m_lastTemperatureRequestTime = millis();
        m_hasTemperaturesRead = false;
        m_sensors.requestTemperatures();
    }
    if (!m_hasTemperaturesRead && (unsigned long)(millis() - m_lastTemperatureRequestTime) > (unsigned long)m_sensors.millisToWaitForConversion(KTEMPERATURE_RESOLUTION)) {
        /*uint8_t devCount = m_sensors.getDeviceCount();
        for (uint8_t i=0; i < devCount && i < KMAX_DALLAS_SENSOR_COUNT; i++) {
            m_temperatureCache[i] = m_sensors.getTempCByIndex(i);
            Serial.println(m_temperatureCache[i] );
        }*/
        m_temperatureCache[KCacheIndex_WaterPipeUpperTemperature].addValue(
            m_sensors.getTempC(KWaterPipeUpperTemperatureSensorAddr));
        
        m_temperatureCache[KCacheIndex_WaterPipeLowerTemperature].addValue(
            m_sensors.getTempC(KWaterPipeLowerTemperatureSensorAddr));
        
        m_temperatureCache[KCacheIndex_OutsideTemperature].addValue(
            m_sensors.getTempC(KOutsideTemperatureSensorAddr));

        m_temperatureCache[KCacheIndex_WaterTankTemperature].addValue(
            m_sensors.getTempC(KWaterTankTemperatureSensorAddr));

        m_hasTemperaturesRead = true;
        m_cacheReady = true;
    }
}

DallasTemperature* Board::dallasTemperature()
{
    return &m_sensors;
}

bool Board::isReady() const
{
    return m_cacheReady;
}

bool Board::hasFlame() const
{
    // LOW = incoming signal from Oilon
    return !m_mcp.digitalRead(KPIN_INPUT_HEATING_FLAME);
}
bool Board::hasHeatingError() const
{
    // LOW = incoming signal from Oilon
    return !m_mcp.digitalRead(KPIN_INPUT_HEATING_ERROR);
}
bool Board::hasMainPower() const
{
    return m_mainPower;
}
bool Board::hasHeaterPower() const
{
    return m_mcp.digitalRead(KMCP_PIN_RELAY_HEATER);
}
bool Board::hasWaterPumpPower() const
{
    return m_mcp.digitalRead(KMCP_PIN_RELAY_WATERPUMP);
}
bool Board::hasExtPower() const
{
    return m_mcp.digitalRead(KMCP_PIN_RELAY_EXT);
}

void Board::setMainPower(bool power)
{
    Serial.print("setMainPower:"); Serial.println(power);
    m_mainPower = power;
    if (m_mainPower) {
        m_mcp.digitalWrite(KMCP_PIN_RELAY_LED, HIGH);
    }
    else {
        m_mcp.digitalWrite(KMCP_PIN_RELAY_LED, LOW);
        setHeaterPower(false);
    }
}
void Board::setHeaterPower(bool power)
{
    Serial.print("setHeaterPower:"); Serial.println(power);
    if (power)
        m_mcp.digitalWrite(KMCP_PIN_RELAY_HEATER, HIGH);
    else
        m_mcp.digitalWrite(KMCP_PIN_RELAY_HEATER, LOW);
}
void Board::setWaterPumpPower(bool power)
{
    Serial.print("setWaterPumpPower:"); Serial.println(power);
    if (power)
        m_mcp.digitalWrite(KMCP_PIN_RELAY_WATERPUMP, HIGH);
    else
        m_mcp.digitalWrite(KMCP_PIN_RELAY_WATERPUMP, LOW);
}
void Board::setExtPower(bool power)
{
    Serial.print("setExtPower:"); Serial.println(power);
    if (power)
        m_mcp.digitalWrite(KMCP_PIN_RELAY_EXT, HIGH);
    else
        m_mcp.digitalWrite(KMCP_PIN_RELAY_EXT, LOW);
}
float Board::getTemperatureWaterTank() const
{
    return m_temperatureCache[KCacheIndex_WaterTankTemperature].value();
}
float Board::getTemperatureSensorWaterIn() const
{
    return m_temperatureCache[KCacheIndex_WaterPipeLowerTemperature].value();
}
float Board::getTemperatureSensorWaterOut() const
{
    return m_temperatureCache[KCacheIndex_WaterPipeUpperTemperature].value();
}
float Board::getTemperatureOutside() const
{
    return m_temperatureCache[KCacheIndex_OutsideTemperature].value();
}


float Board::getTT() const
{
    DeviceAddress adr;// = {0x28,0xde,0x08,0x43,0x98,0x0c,0x00,0xdf};
    uint64toUInt8Array(0x28de0843980c00dfULL, adr);
    
    return m_sensors.getTempC(adr);
}
void Board::setButtonPressedCallback(void (*func)(uint8_t) )
{
    m_buttonPressedCallback = func;
}
// TODO: KOutsideTemperatureSensorIndex