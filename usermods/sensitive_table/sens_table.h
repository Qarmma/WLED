#pragma once
#include "wled.h"
#include <Wire.h>

#define UM_SENSTABLE_ID USERMOD_ID_UNSPECIFIED

#define GPIO_nINT GPIO_NUM_36
#define GPIO_SCL GPIO_NUM_2
#define GPIO_SDA GPIO_NUM_15
#define GPIO_LBATT GPIO_NUM_13
#define GPIO_EXT_NB 16
static int8_t GPIO_EXT[GPIO_EXT_NB] =
{
  GPIO_NUM_35, GPIO_NUM_25, GPIO_NUM_32, GPIO_NUM_26,
  GPIO_NUM_33, GPIO_NUM_27, GPIO_NUM_23, GPIO_NUM_14,
  GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19, GPIO_NUM_18,
  GPIO_NUM_5,  GPIO_NUM_17, GPIO_NUM_16, GPIO_NUM_4
};
#define GPIO_BTN_NB 2
static int8_t GPIO_BTN[GPIO_BTN_NB] = {GPIO_NUM_39, GPIO_NUM_34};

#define EXT_WRITE_ADDR 0x40

#define DBG_ON 1



/**
 * Act following onboards buttons to light a specific led
 **/
class SensTable : public Usermod
{
  public:
    union TouchSens
    {
        uint64_t val;
        bool bit[64];
        struct
        {
          bool nc[40];
          uint8_t p2;
          uint8_t p1;
          uint8_t p0;
        } Ext;
        ~TouchSens() {}
    };

    enum SENST_MODE
    {
      WLED, TOUCH, OTHER
    };


    SensTable() : _hasExtender(1), _touch({0}), _mode(SENST_MODE::WLED), _updateIntervalMS(20) {}
    
    virtual ~SensTable(){}

    /**
     * Setup board IOs and init extender (nothing network related !)
     * Called once at boot
     **/
    void setup()
    {
      PinOwner po = PinOwner::None;
      uint8_t i;

      // Buttons (inverted polarity)
      // Maybe use the handleButton function, have to check if the init here is required or may inmterfere with allocation
      PinManagerPinType btn_pins[GPIO_BTN_NB];
      for(i = 0; i<GPIO_BTN_NB; i++){btn_pins[i] = {GPIO_BTN[i], true};}
      if(!pinManager.allocateMultiplePins(btn_pins, GPIO_BTN_NB, po)) assert(0);
      for(i = 0; i<GPIO_BTN_NB; i++)
      {
        esp_sleep_enable_ext0_wakeup(gpio_num_t(GPIO_BTN[i]), 0);
      }

      // Ext pins
      PinManagerPinType ext_pins[GPIO_EXT_NB];
      for(i = 0; i<GPIO_EXT_NB; i++){ext_pins[i] = {GPIO_EXT[i], true};}
      if(!pinManager.allocateMultiplePins(ext_pins, GPIO_EXT_NB, po)) assert(0);
      for(i = 0; i<GPIO_EXT_NB; i++)
      {
        pinMode(GPIO_EXT[i], INPUT_PULLDOWN);
        esp_sleep_enable_ext0_wakeup(gpio_num_t(GPIO_EXT[i]), 1);
      }

      // Extender
        // Ext interrupt (inverted polarity)
      if(!pinManager.allocatePin(GPIO_nINT, DBG_ON, po)) assert(0);
      pinMode(GPIO_nINT, INPUT_PULLUP);
      esp_sleep_enable_ext0_wakeup(GPIO_nINT, 0);
        // IOs
      this->_hasExtender = _setupExtender(GPIO_SCL, GPIO_SDA, po);

      // Lowbatt
      if(!pinManager.allocatePin(GPIO_LBATT, DBG_ON, po)) assert(0);
      pinMode(GPIO_LBATT, INPUT_PULLDOWN);
    }

    /**
     * Work loop
     **/ 
    void loop()
    {
      static unsigned long time = 0, now;

      if(strip.isUpdating() || this->_mode == SENST_MODE::TOUCH) return;

      now = millis();
      if(now < time) return;
      time = now + _updateIntervalMS;

      // Check if is in "detect mode"
      if(this->_mode == SENST_MODE::TOUCH)
      {
        // Check any press
        if(digitalRead(GPIO_nINT))
        {
          _readExt();
        }
        // Check direct wiring

        // Adapt color to it
      }
    }

    /**
     * Handle user buttons (i.e. one of the 4 set on the web UI)
     **/ 
    bool handleButton(uint8_t b)
    {
      if(buttonType[b] != BTN_TYPE_PUSH) return 0;
      switch(b)
      {
        // Btn 0 switches working mode
        case 0:
            if(this->_mode == SENST_MODE::TOUCH) this->_mode == SENST_MODE::WLED;
            else if(this->_mode == SENST_MODE::WLED) this->_mode == SENST_MODE::TOUCH;
          break;
        default:
          break;
      }
      return 1;
    }

    /*
     * Register in flash for persistent data
     *
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_sensUpdIntervalMS)] = _updateIntervalMS;
    }

    /*
     * Read from flash persistent data
     */
    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) return 0;
      _updateIntervalMS = top[FPSTR(_sensUpdIntervalMS)];
      return 1;
    }

    /**
     * This mod ID
     * W.o. official support, not setting one
     **/ 
    uint16_t getId() {return USERMOD_ID_UNSPECIFIED;}

  private:
    /**
     * Setup extender while trying to reach it
     */
    bool _setupExtender(int8_t scl_pin, int8_t sda_pin, PinOwner& po)
    {
      PinManagerPinType pins[2] = { { scl_pin, true }, { sda_pin, true } };
      if (!pinManager.allocateMultiplePins(pins, 2, po)) assert(0);
      Wire.begin(GPIO_SDA,GPIO_SCL,1000000);
      Wire.beginTransmission(EXT_WRITE_ADDR);
      if(Wire.endTransmission() != 0)
      {
        pinManager.deallocateMultiplePins(pins, 2, po);
        return 0;
      }
      // Found extender, read once
      _readExt();
      return 1;
    }

    bool _readExt()
    {
      static uint8_t vals[3] = {0};

      if(!_hasExtender) return 0;
      Wire.readBytes(vals, 3); // not checking how many bytes received
      this->_touch.Ext.p0 = vals[0];
      this->_touch.Ext.p1 = vals[1];
      this->_touch.Ext.p2 = vals[2];
      return 1;
    }

  private:
    // If extender could be reached and can be used
    bool _hasExtender;
    TouchSens _touch;
    SENST_MODE _mode;
    uint32_t _updateIntervalMS;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[];
    static const char _sensUpdIntervalMS[];
};

const char SensTable::_name[] PROGMEM = "Sensitive Table";
const char SensTable::_sensUpdIntervalMS[] PROGMEM = "Sensors update interval [ms]";
