#pragma once
#include "wled.h"

class SensTable : public Usermod
{
  public:
    SensTable();
    virtual ~SensTable();

    void setup();
    void loop();
    uint16_t getId();
    
    /**
     * subscribe to MQTT topic for controlling usermod
     */
    void onMqttConnect(bool sessionPresent);
    /**
     * handling of MQTT message
     * topic only contains stripped topic (part after /wled/MAC)
     * topic should look like: /swipe with amessage of [up|down]
     */
    bool onMqttMessage(char* topic, char* payload);

    /*
    * Shows the delay between steps and power-off time in the "info"
    * tab of the web-UI.
    */
    void addToJsonInfo(JsonObject& root);
    void addToJsonState(JsonObject& root);
    /*
    * Reads configuration settings from the json API.
    * See void addToJsonState(JsonObject& root)
    */
    void readFromJsonState(JsonObject& root);

    /*
    * Writes the configuration to internal flash memory.
    */
    void addToConfig(JsonObject& root);
    /*
    * Reads the configuration to internal flash memory before setup() is called.
    * 
    * The function should return true if configuration was successfully loaded or false if there was no configuration.
    */
    bool readFromConfig(JsonObject& root);
    
  private:
    void publishMqtt(bool bottom, const char* state);

    // send sesnor values to JSON API
    void writeSensorsToJson(JsonObject& staircase);
    // allow overrides from JSON API
    void readSensorsFromJson(JsonObject& staircase);

    void updateSegments();
    /*
    * Detects if an object is within ultrasound range.
    * signalPin: The pin where the pulse is sent
    * echoPin:   The pin where the echo is received
    * maxTimeUs: Detection timeout in microseconds. If an echo is
    *            received within this time, an object is detected
    *            and the function will return true.
    *
    * The speed of sound is 343 meters per second at 20 degress Celcius.
    * Since the sound has to travel back and forth, the detection
    * distance for the sensor in cm is (0.0343 * maxTimeUs) / 2.
    *
    * For practical reasons, here are some useful distances:
    *
    * Distance =	maxtime
    *     5 cm =  292 uS
    *    10 cm =  583 uS
    *    20 cm = 1166 uS
    *    30 cm = 1749 uS
    *    50 cm = 2915 uS
    *   100 cm = 5831 uS
    */
    bool ultrasoundRead(int8_t signalPin, int8_t echoPin, unsigned int maxTimeUs);
    bool checkSensors();
    void autoPowerOff();
    void updateSwipe();
    
    void enable(bool enable);

  private:
    /* configuration (available in API and stored in flash) */
    bool enabled = false;                   // Enable this usermod
    unsigned long segment_delay_ms = 150;   // Time between switching each segment
    unsigned long on_time_ms       = 30000; // The time for the light to stay on
    int8_t topPIRorTriggerPin      = -1;    // disabled
    int8_t bottomPIRorTriggerPin   = -1;    // disabled
    int8_t topEchoPin              = -1;    // disabled
    int8_t bottomEchoPin           = -1;    // disabled
    bool useUSSensorTop            = false; // using PIR or UltraSound sensor?
    bool useUSSensorBottom         = false; // using PIR or UltraSound sensor?
    unsigned int topMaxDist        = 50;    // default maximum measured distance in cm, top
    unsigned int bottomMaxDist     = 50;    // default maximum measured distance in cm, bottom

    /* runtime variables */
    bool initDone = false;
    // Time between checking of the sensors
    const unsigned int scanDelay = 100;
    // Lights on or off.
    // Flipping this will start a transition.
    bool on = false;
    // Swipe direction for current transition
  #define SWIPE_UP true
  #define SWIPE_DOWN false
    bool swipe = SWIPE_UP;
    // Indicates which Sensor was seen last (to determine
    // the direction when swiping off)
  #define LOWER false
  #define UPPER true
    bool lastSensor = LOWER;
    // Time of the last transition action
    unsigned long lastTime = 0;
    // Time of the last sensor check
    unsigned long lastScanTime = 0;
    // Last time the lights were switched on or off
    unsigned long lastSwitchTime = 0;
    // segment id between onIndex and offIndex are on.
    // controll the swipe by setting/moving these indices around.
    // onIndex must be less than or equal to offIndex
    byte onIndex = 0;
    byte offIndex = 0;
    // The maximum number of configured segments.
    // Dynamically updated based on user configuration.
    byte maxSegmentId = 1;
    byte mainSegmentId = 0;
    // These values are used by the API to read the
    // last sensor state, or trigger a sensor
    // through the API
    bool topSensorRead     = false;
    bool topSensorWrite    = false;
    bool bottomSensorRead  = false;
    bool bottomSensorWrite = false;
    bool topSensorState    = false;
    bool bottomSensorState = false;
    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _enabled[];
    static const char _segmentDelay[];
    static const char _onTime[];
    static const char _useTopUltrasoundSensor[];
    static const char _topPIRorTrigger_pin[];
    static const char _topEcho_pin[];
    static const char _useBottomUltrasoundSensor[];
    static const char _bottomPIRorTrigger_pin[];
    static const char _bottomEcho_pin[];
    static const char _topEchoCm[];
    static const char _bottomEchoCm[];
};
