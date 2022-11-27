#pragma once
#include "wled.h"

/**
 * Act following onboards buttons to light a specific led
 **/
class SensTable : public Usermod
{
  public:
    SensTable()
    {

    }
    
    virtual ~SensTable()
    {

    }

    /**
     * Setup board IOs and init extender (nothing network related !)
     * Called once at boot
     **/
    void setup()
    {

    }

    /**
     * Work loop
     **/ 
    void loop()
    {
      
    }

    /**
     * Handle user buttons (i.e. one of the 4 set on the web UI)
     **/ 
    bool handleButton(uint8_t b)
    {
      return false;
    }

    /**
     * ???
     **/
    void appendConfigData()
    {
      oappend(SET_F("dd=addDropdown('Sensitive table','type');"));
      oappend(SET_F("addOption(dd,'None',0);"));
      oappend(SET_F("addOption(dd,'SSD1306',1);"));
      oappend(SET_F("addOption(dd,'SH1106',2);"));
      oappend(SET_F("addOption(dd,'SSD1306 128x64',3);"));
      oappend(SET_F("addOption(dd,'SSD1305',4);"));
      oappend(SET_F("addOption(dd,'SSD1305 128x64',5);"));
      oappend(SET_F("addOption(dd,'SSD1306 SPI',6);"));
      oappend(SET_F("addOption(dd,'SSD1306 SPI 128x64',7);"));
      oappend(SET_F("addInfo('4LineDisplay:pin[]',0,'<i>-1 use global</i>','I2C/SPI CLK');"));
      oappend(SET_F("addInfo('4LineDisplay:pin[]',1,'<i>-1 use global</i>','I2C/SPI DTA');"));
      oappend(SET_F("addInfo('4LineDisplay:pin[]',2,'','SPI CS');"));
      oappend(SET_F("addInfo('4LineDisplay:pin[]',3,'','SPI DC');"));
      oappend(SET_F("addInfo('4LineDisplay:pin[]',4,'','SPI RST');"));
    }
    
    /*
     * addToJsonInfo() can be used to add custom entries to the /json/info part of the JSON API.
     * Creating an "u" object allows you to add custom key/value pairs to the Info section of the WLED web UI.
     * Below it is shown how this could be used for e.g. a light sensor
     */
    void addToJsonInfo(JsonObject& root)
    {
      //JsonObject user = root["u"];
      //if (user.isNull()) user = root.createNestedObject("u");
      //JsonArray data = user.createNestedArray(F("4LineDisplay"));
      //data.add(F("Loaded."));
    }

    /*
     * addToJsonState() can be used to add custom entries to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void addToJsonState(JsonObject& root)
    {
    }

    /*
     * readFromJsonState() can be used to receive data clients send to the /json/state part of the JSON API (state object).
     * Values in the state object may be modified by connected clients
     */
    void readFromJsonState(JsonObject& root)
    {
    //  if (!initDone) return;  // prevent crash on boot applyPreset()
    }

    /*
     * Register in flash for persistent data
     *
     * addToConfig() will also not yet add your setting to one of the settings pages automatically.
     * To make that work you still have to add the setting to the HTML, xml.cpp and set.cpp manually.
     */
    void addToConfig(JsonObject& obj)
    {
      // // determine if we are using global HW pins (data & clock)
      // int8_t hw_dta, hw_clk;
      // if ((type == SSD1306_SPI || type == SSD1306_SPI64)) {
      //   hw_clk = spi_sclk<0 ? HW_PIN_CLOCKSPI : spi_sclk;
      //   hw_dta = spi_mosi<0 ? HW_PIN_DATASPI : spi_mosi;
      // } else {
      //   hw_clk = i2c_scl<0 ? HW_PIN_SCL : i2c_scl;
      //   hw_dta = i2c_sda<0 ? HW_PIN_SDA : i2c_sda;
      // }

      // JsonObject top   = root.createNestedObject(FPSTR(_name));
      // top[FPSTR(_enabled)]       = enabled;

      // JsonArray io_pin = top.createNestedArray("pin");
      // for (int i=0; i<5; i++) {
      //   if      (i==0 && ioPin[i]==hw_clk) io_pin.add(-1); // do not store global HW pin
      //   else if (i==1 && ioPin[i]==hw_dta) io_pin.add(-1); // do not store global HW pin
      //   else                               io_pin.add(ioPin[i]);
      // }
      // top["type"]                = type;
      // top[FPSTR(_flip)]          = (bool) flip;
      // top[FPSTR(_contrast)]      = contrast;
      // top[FPSTR(_contrastFix)]   = (bool) contrastFix;
      // #ifndef ARDUINO_ARCH_ESP32
      // top[FPSTR(_refreshRate)]   = refreshRate;
      // #endif
      // top[FPSTR(_screenTimeOut)] = screenTimeout/1000;
      // top[FPSTR(_sleepMode)]     = (bool) sleepMode;
      // top[FPSTR(_clockMode)]     = (bool) clockMode;
      // top[FPSTR(_showSeconds)]   = (bool) showSeconds;
      // top[FPSTR(_busClkFrequency)] = ioFrequency/1000;
      // DEBUG_PRINTLN(F("4 Line Display config saved."));
    }

    /*
     * Read from flash persistent data
     */
    bool readFromConfig(JsonObject& obj)
    {
      // bool needsRedraw    = false;
      // DisplayType newType = type;
      // int8_t oldPin[5]; for (byte i=0; i<5; i++) oldPin[i] = ioPin[i];

      // JsonObject top = root[FPSTR(_name)];
      // if (top.isNull()) {
      //   DEBUG_PRINT(FPSTR(_name));
      //   DEBUG_PRINTLN(F(": No config found. (Using defaults.)"));
      //   return false;
      // }

      // enabled       = top[FPSTR(_enabled)] | enabled;
      // newType       = top["type"] | newType;
      // for (byte i=0; i<5; i++) ioPin[i] = top["pin"][i] | ioPin[i];
      // flip          = top[FPSTR(_flip)] | flip;
      // contrast      = top[FPSTR(_contrast)] | contrast;
      // #ifndef ARDUINO_ARCH_ESP32
      // refreshRate   = top[FPSTR(_refreshRate)] | refreshRate;
      // refreshRate   = min(5000, max(250, (int)refreshRate));
      // #endif
      // screenTimeout = (top[FPSTR(_screenTimeOut)] | screenTimeout/1000) * 1000;
      // sleepMode     = top[FPSTR(_sleepMode)] | sleepMode;
      // clockMode     = top[FPSTR(_clockMode)] | clockMode;
      // showSeconds   = top[FPSTR(_showSeconds)] | showSeconds;
      // contrastFix   = top[FPSTR(_contrastFix)] | contrastFix;
      // if (newType == SSD1306_SPI || newType == SSD1306_SPI64)
      //   ioFrequency = min(20000, max(500, (int)(top[FPSTR(_busClkFrequency)] | ioFrequency/1000))) * 1000;  // limit frequency
      // else
      //   ioFrequency = min(3400, max(100, (int)(top[FPSTR(_busClkFrequency)] | ioFrequency/1000))) * 1000;  // limit frequency

      // DEBUG_PRINT(FPSTR(_name));
      // if (!initDone) {
      //   // first run: reading from cfg.json
      //   type = newType;
      //   DEBUG_PRINTLN(F(" config loaded."));
      // } else {
      //   DEBUG_PRINTLN(F(" config (re)loaded."));
      //   // changing parameters from settings page
      //   bool pinsChanged = false;
      //   for (byte i=0; i<5; i++) if (ioPin[i] != oldPin[i]) { pinsChanged = true; break; }
      //   if (pinsChanged || type!=newType) {
      //     if (type != NONE) delete u8x8;
      //     PinOwner po = PinOwner::UM_FourLineDisplay;
      //     bool isSPI = (type == SSD1306_SPI || type == SSD1306_SPI64);
      //     if (isSPI) {
      //       pinManager.deallocateMultiplePins((const uint8_t *)(&oldPin[2]), 3, po);
      //       uint8_t hw_sclk = spi_sclk<0 ? HW_PIN_CLOCKSPI : spi_sclk;
      //       uint8_t hw_mosi = spi_mosi<0 ? HW_PIN_DATASPI : spi_mosi;
      //       bool isHW = (oldPin[0]==hw_sclk && oldPin[1]==hw_mosi);
      //       if (isHW) po = PinOwner::HW_SPI;
      //     } else {
      //       uint8_t hw_scl = i2c_scl<0 ? HW_PIN_SCL : i2c_scl;
      //       uint8_t hw_sda = i2c_sda<0 ? HW_PIN_SDA : i2c_sda;
      //       bool isHW = (oldPin[0]==hw_scl && oldPin[1]==hw_sda);
      //       if (isHW) po = PinOwner::HW_I2C;
      //     }
      //     pinManager.deallocateMultiplePins((const uint8_t *)oldPin, 2, po);
      //     type = newType;
      //     setup();
      //     needsRedraw |= true;
      //   } else {
      //     u8x8->setBusClock(ioFrequency); // can be used for SPI too
      //     setVcomh(contrastFix);
      //     setContrast(contrast);
      //     setFlipMode(flip);
      //   }
      //   knownHour = 99;
      //   if (needsRedraw && !wakeDisplay()) redraw(true);
      //   else overlayLogo(3500);
      // }
      // // use "return !top["newestParameter"].isNull();" when updating Usermod with new features
      // return !top[FPSTR(_contrastFix)].isNull();
      return false;
    }

    /**
     * This mod ID
     * W.o. official support, not setting one
     **/ 
    uint16_t getId() {return USERMOD_ID_UNSPECIFIED;}

  private:
    void updateSegments();
    
    void enable(bool enable);

  private:
    /* configuration (available in API and stored in flash) */
    bool enabled = false;                   // Enable this usermod

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
