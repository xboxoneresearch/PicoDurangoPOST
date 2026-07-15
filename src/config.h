#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include <CRC.h>

#define CFG_VERSION  2
#define CONFIG_ADDR  0x0
#define EEPROM_SIZE  256
#define CONFIG_MAGIC 0x30474643 // CFG0

typedef struct {
    uint32_t magic;                  /* 0x00 */
    uint8_t version;                 /* 0x04 */
    uint8_t data_length;             /* 0x05 */
    uint16_t checksum;               /* 0x06 */
} ConfigHeader, *PConfigHeader;      /* Total len: 0x08 */

typedef struct {
    uint8_t  disp_mirrored;          /* 0x00 */
    uint8_t  disp_rotation_portrait; /* 0x01 */
    uint8_t  serial_print_colors;    /* 0x02 */
    uint8_t  post_print_timestamps;  /* 0x03 */
    uint8_t  xbox_sda_pin;           /* 0x04 */
    uint8_t  xbox_scl_pin;           /* 0x05 */
} ConfigData, *PConfigData;          /* Total len: 0x06 */

const uint8_t CFG_HEADER_SIZE = sizeof(ConfigHeader);
const uint8_t CFG_DATA_SIZE = sizeof(ConfigData);

enum ConfigValues {
    CFG_DISPLAY_MIRRORED = 0,
    CFG_ROTATION_PORTRAIT = 1,
    CFG_PRINT_COLORS = 2,
    CFG_PRINT_TIMESTAMPS = 3,
    CFG_XBOX_SDA_PIN = 4,
    CFG_XBOX_SCL_PIN = 5,
};

// Default configuration values
const ConfigData DEFAULT_CONFIG = {
    .disp_mirrored = 0,
    .disp_rotation_portrait = 0,
    .serial_print_colors = 1,
    .post_print_timestamps = 1,
    .xbox_sda_pin = PIN_SDA_XBOX,
    .xbox_scl_pin = PIN_SCL_XBOX,
};

// RP2040/ESP32 emulate EEPROM in flash and need an explicit begin(size) /
// commit() / end(); Teensy's EEPROM is written through directly, no buffering.
#if defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_ARCH_ESP32)
#define EEPROM_NEEDS_COMMIT 1
#else
#define EEPROM_NEEDS_COMMIT 0
#endif

class Config {
    public:
        ~Config() {
#if EEPROM_NEEDS_COMMIT
            EEPROM.end();
#endif
        }

        bool begin() {
#if EEPROM_NEEDS_COMMIT
            EEPROM.begin(EEPROM_SIZE);
#endif
            ConfigHeader tmpHeader = {0};
            EEPROM.get(CONFIG_ADDR, tmpHeader);

            if (tmpHeader.magic != CONFIG_MAGIC) {
                // Invalid config, initialize with defaults
                initDefaultConfig();
                return true;
            }

            if (tmpHeader.version < CFG_VERSION) {
                // upgradeConfig() reads the old (smaller) data itself, since
                // its on-disk layout/size differs from the current one, then
                // saves it back out in the current layout.
                if (!upgradeConfig(tmpHeader)) {
                    initDefaultConfig();
                }
                return true;
            }

            if (tmpHeader.data_length != CFG_DATA_SIZE) {
                // Invalid config, initialize with defaults
                initDefaultConfig();
                return true;
            }

            // Read data
            EEPROM.get(CONFIG_ADDR + CFG_HEADER_SIZE, data);

            // Verify checksum
            uint16_t calcChecksum = calcCRC16((uint8_t*)&data, CFG_DATA_SIZE);
            if (calcChecksum != tmpHeader.checksum) {
                // Checksum mismatch, initialize with defaults
                initDefaultConfig();
                return true;
            }

            header = tmpHeader;
            initialized = true;
            return true;
        }

        bool save() {
            if (!initialized) return false;
            
            // Calculate new checksum
            header.checksum = calcCRC16((uint8_t*)&data, CFG_DATA_SIZE);
            
            // Write header and data
            EEPROM.put(CONFIG_ADDR, header);
            EEPROM.put(CONFIG_ADDR + CFG_HEADER_SIZE, data);
#if EEPROM_NEEDS_COMMIT
            return EEPROM.commit();
#else
            return true;
#endif
        }

        // Getters
        bool isDisplayMirrored() const { return data.disp_mirrored; }
        bool isRotationPortrait() const { return data.disp_rotation_portrait; }
        bool isSerialPrintColors() const { return data.serial_print_colors; }
        bool isPostPrintTimestamps() const { return data.post_print_timestamps; }
        uint8_t getXboxSdaPin() const { return data.xbox_sda_pin; }
        uint8_t getXboxSclPin() const { return data.xbox_scl_pin; }

        // Setters
        void setDisplayMirrored(bool value) { data.disp_mirrored = value; }
        void setRotationPortrait(bool value) { data.disp_rotation_portrait = value; }
        void setSerialPrintColors(bool value) { data.serial_print_colors = value; }
        void setPostPrintTimestamps(bool value) { data.post_print_timestamps = value; }
        void setXboxI2CPins(uint8_t sda, uint8_t scl) { data.xbox_sda_pin = sda; data.xbox_scl_pin = scl; }

        // Togglers
        void toggleDisplayMirrored() { data.disp_mirrored = !data.disp_mirrored; }
        void toggleRotationPortrait() { data.disp_rotation_portrait = !data.disp_rotation_portrait; }
        void toggleSerialPrintColors() { data.serial_print_colors = !data.serial_print_colors; }
        void togglePostPrintTimestamps() { data.post_print_timestamps = !data.post_print_timestamps; }

    private:
        bool initialized = false;
        ConfigHeader header = {0};
        ConfigData data = {0};

        void initDefaultConfig() {
            header.magic = CONFIG_MAGIC;
            header.version = CFG_VERSION;
            header.data_length = CFG_DATA_SIZE;
            data = DEFAULT_CONFIG;
            initialized = true;
        }

        bool upgradeConfig(ConfigHeader& oldHeader) {
            // Perform version-specific upgrades
            switch (oldHeader.version) {
                case 1: {
                    // v1's ConfigData was 4 bytes (no xbox_sda_pin/xbox_scl_pin
                    // yet). Read exactly that many bytes so we don't pull in
                    // unrelated flash contents past the old struct's length.
                    struct {
                        uint8_t disp_mirrored;
                        uint8_t disp_rotation_portrait;
                        uint8_t serial_print_colors;
                        uint8_t post_print_timestamps;
                    } oldData;
                    EEPROM.get(CONFIG_ADDR + CFG_HEADER_SIZE, oldData);
                    data.disp_mirrored = oldData.disp_mirrored;
                    data.disp_rotation_portrait = oldData.disp_rotation_portrait;
                    data.serial_print_colors = oldData.serial_print_colors;
                    data.post_print_timestamps = oldData.post_print_timestamps;
                    data.xbox_sda_pin = DEFAULT_CONFIG.xbox_sda_pin;
                    data.xbox_scl_pin = DEFAULT_CONFIG.xbox_scl_pin;
                    break;
                }

                default:
                    return false; // Unknown version
            }

            // Update version number and length
            oldHeader.version = CFG_VERSION;
            oldHeader.data_length = CFG_DATA_SIZE;
            header = oldHeader;
            initialized = true;
            return save();
        }
};
