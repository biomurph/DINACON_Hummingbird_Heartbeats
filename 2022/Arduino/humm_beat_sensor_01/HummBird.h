

#ifndef HummBird_h
#define HummBird_h

const size_t ADC_COUNT = 5;
struct data_t {
  uint16_t adc[ADC_COUNT];
};
//------------------------------------------------------------------------------
// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 2
//------------------------------------------------------------------------------
// Interval between data records in microseconds.
// Try 250 with Teensy 3.6, Due, or STM32.
// Try 2000 with AVR boards.
// Try 4000 with SAMD Zero boards.

// LOG INTERVAL DETERMINES SAMPLE RATE
const uint32_t LOG_INTERVAL_USEC = 500; // 500uS = 2kHz
#define FIFO_SIZE_SECTORS 16
// Preallocate 1GiB file.
const uint32_t PREALLOCATE_SIZE_MiB = 1024UL;
// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(50)

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif  ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
#endif  // HAS_SDIO_CLASS

// LED to light if overruns occur.
#define ERROR_LED_PIN 13
const uint8_t SD_CS_PIN = 4;  // SS;
#define BUTTON_PIN  23
#define BLU_LED 10
#define RED_LED 3
#define SAMPLE_CLK_PIN 24
#define ACCEL_TEST_PIN 21 

#define DEFAULT_RECORD_DURATION 5000
#define STARTUP_DURATION  -3
#define LED_CYCLE 256*3 // half cycle led fade power of 2 please
#define RECORD_OFF 1
#define RECORD_ON  2
#define PULSE_RED 0
#define PULSE_BLU 1
#define ACCEL_X 2
#define ACCEL_Y 3
#define ACCEL_Z 4
#define DURATION_ADDRESS 1
#define MAX_G 3.0
#define MIN_G -3.0

// You may modify the filename.  Digits before the dot are file versions.
char binName[] = "Humm_00.bin";

unsigned long lastFadeTime;
int LEDfadeValue = 255;
int LEDfadeTime = LED_CYCLE/256;
boolean fadeUp = false;
boolean sampleClockPinState = false;
boolean isRunning = false;
int recordDuration = 5000;
int lastButtonValue, buttonValue;
int buttonState = RECORD_OFF;
unsigned long timeIn;
int timeOut;
boolean freeRunningTest = false;


#endif
