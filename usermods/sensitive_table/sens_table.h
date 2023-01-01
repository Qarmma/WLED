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

#define EXT_I2C_ADDR (0x40 >> 1)

#define DBG_ON 1


#define PRINT(x) DEBUGOUT.print(x)
#define PRINTLN(x) DEBUGOUT.println(x)
#define PRINTF(x...) DEBUGOUT.printf(x)


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
        ~TouchSens() {}
    };

    enum SENST_MODE
    {
      WLED, TOUCH, OTHER
    };

    #define BINSZ (45)
    void inAsBin(TouchSens* t, char b[BINSZ])
    {
      uint8_t nt = 0;
      for(uint8_t i = 0; i < 40; i++)
      {
        if(i != 0 && i%8==0)
        {
          b[BINSZ - 2 - i - nt] = '\'';
          nt ++;
        }
        b[BINSZ - 2 - i - nt] = t->bit[i] ? '1':'0';
      }
      b[BINSZ-1] = '\0';
    }


    SensTable() : _hasExtender(1), _touch({0}), _mode(SENST_MODE::TOUCH), _updateIntervalMS(20), _ledsOffTimeoutMS(1500), _ledsFadeMS(500), _leds_ctrl{{0}} {}
    
    virtual ~SensTable(){}

    /**
     * Setup board IOs and init extender (nothing network related !)
     * Called once at boot
     **/
    void setup()
    {
      //if(esp_reset_reason() != esp_reset_reason_t::ESP_RST_SW) { delay(6000); }
      PRINTLN("SensTable mod setting up");
      PinOwner po = PinOwner::None;
      uint8_t i;

      // Buttons (inverted polarity)
      // Maybe use the handleButton function, have to check if the init here is required or may inmterfere with allocation
      PinManagerPinType btn_pins[GPIO_BTN_NB];
      for(i = 0; i<GPIO_BTN_NB; i++){btn_pins[i] = {GPIO_BTN[i], true};}
      if(!pinManager.allocateMultiplePins(btn_pins, GPIO_BTN_NB, po))
      {
        PRINTLN(" *** Could not allocate touch pins");
      }
      for(i = 0; i<GPIO_BTN_NB; i++)
      {
        esp_sleep_enable_ext0_wakeup(gpio_num_t(GPIO_BTN[i]), 0);
      }

      // Ext pins
      PinManagerPinType ext_pins[GPIO_EXT_NB];
      for(i = 0; i<GPIO_EXT_NB; i++){ext_pins[i] = {GPIO_EXT[i], true};}
      if(!pinManager.allocateMultiplePins(ext_pins, GPIO_EXT_NB, po))
      {
        PRINTLN(" *** Could not allocate ext. touch pins");
      }
      for(i = 0; i<GPIO_EXT_NB; i++)
      {
        pinMode(GPIO_EXT[i], INPUT_PULLDOWN);
        esp_sleep_enable_ext0_wakeup(gpio_num_t(GPIO_EXT[i]), 1);
      }

      // Extender
        // Ext interrupt (inverted polarity)
      if(!pinManager.allocatePin(GPIO_nINT, DBG_ON, po))
      {
        PRINTLN(" *** Could not allocate ext. intr. pin");
      }
      pinMode(GPIO_nINT, INPUT_PULLUP);
      esp_sleep_enable_ext0_wakeup(GPIO_nINT, 0);
        // IOs
      _setupExtender(GPIO_SCL, GPIO_SDA, po);

      // Lowbatt
      if(!pinManager.allocatePin(GPIO_LBATT, DBG_ON, po))
      {
        PRINTLN(" *** Could not allocate low batt. pin");
      }
      pinMode(GPIO_LBATT, INPUT_PULLDOWN);
    }

    /**
     * Work loop
     **/ 
    unsigned long now;
    void loop()
    {
      static unsigned long time = 0,  refreshtime = 0, triggertime = 0;
      static bool anychange;
      static uint8_t oldmode = 255;
      
      // Check if is in "detect mode"
      if(this->_mode == SENST_MODE::TOUCH)
      {
        if(strip.isUpdating()) return;

        // Force mode to static
        if(strip._segments[0].mode != 0)
        {
          if(oldmode == 255)
          {
            oldmode = strip._segments[0].mode;
          }
          strip.setMode(0,0);
        }
        
        anychange = 0;
        now = millis();

        // Do not refresh too often
        if(now >= time)
        {
          time = now + this->_updateIntervalMS;

          // Check any press
          PRINT("nINT = ");
          PRINTLN(digitalRead(GPIO_nINT));
          if(!digitalRead(GPIO_nINT))
          {
            PRINTLN(" ** Reading exts");
            anychange &= _readExt();
          }
          // Check direct wiring
          for(uint8_t i = 0; i < GPIO_EXT_NB; i++)
          {
            static bool r;
            r = digitalRead(GPIO_EXT[i]);
            anychange |= _manageSens(r, i);
          }
          
          if(anychange)
          {
            triggertime = now + 2*this->_ledsFadeMS + this->_ledsOffTimeoutMS; // worst case
          }

          // DBG
          static char b[BINSZ];
          inAsBin(&this->_touch, b);
          PRINTLN(b);
          PRINTLN();
        }
        
        // Force refresh @ max 60Hz and only when needed
        if(now < triggertime && now > refreshtime /*lock to max 60Hz*/ )
        {
          refreshtime = now + 17;
          strip.trigger();// force strip refresh while at least fading
          stateChanged = true;  // inform external devices/UI of change
          colorUpdated(CALL_MODE_DIRECT_CHANGE);
        }
      }
      // Not in our mod
      else
      {
        if(oldmode != 255)
        {
          strip.setMode(0,oldmode);
          oldmode = 255;
        }
      }
    }

    /*
     * handleOverlayDraw() is called just before every show() (LED strip update frame) after effects have set the colors.
     * Use this to blank out some LEDs or set them to a different color regardless of the set effect mode.
     * Commonly used for custom clocks (Cronixie, 7 segment)
     */
    void handleOverlayDraw()
    {

      if(this->_mode == SENST_MODE::TOUCH)
      {
        // Adapt color and on times
        for(uint8_t i = 0; i < 40; i++)
        {
          static float t;
          t = now - this->_leds_ctrl.time[i];
          // on
          if(this->_touch.bit[i])
          {
            // fading
            if(t < this->_ledsFadeMS)
            {
              strip.setPixelColor(i, CRGB(strip.getPixelColor(i)).nscale8_video( 255*(t / float(this->_ledsFadeMS))));
            }
            //else still -> let color as is
          }
          // off
          else
          {
            // still timeout (need this not to trigger fading too fast)
            if(now < this->_leds_ctrl.time[i])
            {
              // strip.setPixelColor(i, CRGB(0,255,255));
            }
            // fade
            else if(now - this->_ledsFadeMS < this->_leds_ctrl.time[i])
            {
              strip.setPixelColor(i, CRGB(strip.getPixelColor(i)).nscale8_video( 255 - 255*(t / float(this->_ledsFadeMS)) ));
            }
            else
            {
              strip.setPixelColor(i, 0);
            }
          }
        }
      }
    }

    /**
     * Handle user buttons (i.e. one of the 4 set on the web UI)
     **/ 
    bool handleButton(uint8_t b)
    {
      static bool pressed[2] = {0};
      if(b >= 2){return 0;}
      if(isButtonPressed(b) && pressed[b] == 0)
      {
        pressed[b] = 1;
        PRINT("Button pressed : ");
        PRINTLN(b);
        switch(b)
        {
          // Btn 0 switches working mode
          case 0:
              if(this->_mode == SENST_MODE::TOUCH)
              {
                PRINTLN(" * Setting mode to WLED");
                this->_mode = SENST_MODE::WLED;
              }
              else if(this->_mode == SENST_MODE::WLED)
              {
                PRINTLN(" * Setting mode to TOUCH");
                this->_mode = SENST_MODE::TOUCH;
              }
            break;
           case 1:
             esp_restart();
          default:
            return 0;
            break;
        }
      }
      else if(!isButtonPressed(b) && pressed[b] == 1)
      {
        pressed[b] = 0;
        return 1; // exit, except if we may do something with this
      }
      return 1;
    }

    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root)
    {
      JsonObject top = root[FPSTR(_name)];
      if(top.isNull()) top = root.createNestedObject(FPSTR(_name));
      
      JsonArray senstArr = top.createNestedArray(FPSTR(_sensUpdIntervalMS)); //name
      senstArr.add(_sensUpdIntervalMS); //value
      senstArr.add(" [ms]"); //unit
      JsonArray ledstArr = top.createNestedArray(FPSTR(_ledsTimeoutMS)); //name
      ledstArr.add(_ledsOffTimeoutMS); //value
      ledstArr.add(" [ms]"); //unit
      JsonArray lfarr = top.createNestedArray(FPSTR(_ledsFadeTimeMS)); //name
      lfarr.add(_ledsFadeMS); //value
      lfarr.add(" [ms]"); //
    }

    /*
     * Register in flash for persistent data
     *
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     */
    void addToConfig(JsonObject& root)
    {
      JsonObject top = root[FPSTR(_name)];
      if(top.isNull()) top = root.createNestedObject(FPSTR(_name));
      top[FPSTR(_sensUpdIntervalMS)] = _updateIntervalMS;
      top[FPSTR(_ledsTimeoutMS)] = _ledsOffTimeoutMS;
      top[FPSTR(_ledsFadeTimeMS)] = _ledsFadeMS;
    }

    /*
     * Read from flash persistent data
     */
    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root[FPSTR(_name)];
      if (top.isNull()) return 0;
      getJsonValue(top[FPSTR(_sensUpdIntervalMS)], _updateIntervalMS, _updateIntervalMS);
      getJsonValue(top[FPSTR(_ledsTimeoutMS)], _ledsOffTimeoutMS, _ledsOffTimeoutMS);
      getJsonValue(top[FPSTR(_ledsFadeTimeMS)], _ledsFadeMS, _ledsFadeMS);
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
      PRINTLN(" * Setting up extender");
      PinManagerPinType pins[2] = { { scl_pin, true }, { sda_pin, true } };
      if (!pinManager.allocateMultiplePins(pins, 2, po))
      {
        PRINTLN("  *** Could not allocate I2C pins");
      }
      Wire.begin(GPIO_SDA,GPIO_SCL,1000000);
      Wire.beginTransmission(EXT_I2C_ADDR);
      if(Wire.endTransmission() != I2C_ERROR_OK)
      {
        PRINTLN("  ** I2C extender not found");
        pinManager.deallocateMultiplePins(pins, 2, po);
        this->_hasExtender = 0;
        return 0;
      }
      PRINTLN("  ** I2C extender found");
      this->_hasExtender = 1;
      return 1;
    }

    bool _readExt()
    {
      static uint8_t vals[3] = {0};
      if(!this->_hasExtender) { return 0; }
      PRINTLN("EXTERNAL READ !");
      Wire.requestFrom(EXT_I2C_ADDR,3); 
      Wire.readBytes(vals, 3); // 3 data
      PRINTLN("Read bytes : "); PRINTLN(vals[0]);PRINTLN(vals[1]);PRINTLN(vals[2]);
      bool b = 0;
      for(uint8_t j = 0; j<3; j++)
      {
        for(uint8_t i = 0; i < 8; i++)
        {
          static bool r;
          static uint8_t ind;
          r = vals[j] & (1 << i);
          ind = _EXT_LUT[i+8*j];
          b |= _manageSens(r, ind);
        }
      }
      return b;
    }

    inline bool _manageSens(bool newval, uint8_t index)
    {
      bool b = 0;
      if(newval != this->_touch.bit[index])
      {
        if(!newval && now-this->_leds_ctrl.time[index] < _ledsFadeMS){} // does not register until fade-in completed
        else
        {
          this->_leds_ctrl.time[index] = newval ? now : now + _ledsOffTimeoutMS;
          b = 1;
          this->_touch.bit[index] = newval;
        }
      }
      return b;
    }

  private:
    // If extender could be reached and can be used
    bool _hasExtender;
    TouchSens _touch;
    SENST_MODE _mode;
    uint32_t _updateIntervalMS;
    uint32_t _ledsOffTimeoutMS, _ledsFadeMS;
    struct{
      time_t time[40];
    } _leds_ctrl;

    // strings to reduce flash memory usage (used more than twice)
    static const char _name[], _sensUpdIntervalMS[], _ledsTimeoutMS[], _ledsFadeTimeMS[];
    static const uint8_t _EXT_LUT[];
};

const char SensTable::_name[] PROGMEM = "Sensitive Table";
const char SensTable::_sensUpdIntervalMS[] PROGMEM = "Sensors update interval [ms]";
const char SensTable::_ledsTimeoutMS[] PROGMEM = "Leds STILL time [ms]";
const char SensTable::_ledsFadeTimeMS[] PROGMEM = "Leds FADE time [ms]";
const uint8_t SensTable::_EXT_LUT[] PROGMEM = {39,37,35,38,36,34,33,32,30,28,26,24,22,20,21,23,25,27,29,31,19,17,16,18};
