/*
 *    this code targeted to Teensy 3.6
 *    use for baseline record capability and data retrieval
 *    set to highest ADC resolution
 *    
 */


// Example to demonstrate write latency for preallocated exFAT files.
// I suggest you write a PC program to convert very large bin files.
//
// The maximum data rate will depend on the quality of your SD,
// the size of the FIFO, and using dedicated SPI.
#include "SdFat.h"
#include "FreeStack.h"
#include "HummBird.h"
#include <TimeLib.h>


const uint64_t PREALLOCATE_SIZE  =  (uint64_t)PREALLOCATE_SIZE_MiB << 20;
// Max length of file name including zero byte.
#define FILE_NAME_DIM 40
// Max number of records to buffer while SD is busy.
const size_t FIFO_DIM = 512*FIFO_SIZE_SECTORS/sizeof(data_t);

#if SD_FAT_TYPE == 0
typedef SdFat sd_t;
typedef File file_t;
#elif SD_FAT_TYPE == 1
typedef SdFat32 sd_t;
typedef File32 file_t;
#elif SD_FAT_TYPE == 2
typedef SdExFat sd_t;
typedef ExFile file_t;
#elif SD_FAT_TYPE == 3
typedef SdFs sd_t;
typedef FsFile file_t;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

sd_t sd;

file_t binFile;
file_t csvFile;

#define error(s) sd.errorHalt(&Serial, F(s))
#define dbgAssert(e) ((e) ? (void)0 : error("assert " #e))

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
void testSensor() {
  const uint32_t interval = 50000;
  int32_t diff;
  data_t data;
  clearSerialInput();
  Serial.println(F("\nTesting - type any character to stop\n"));
  delay(1000);
  printRecord(&Serial, nullptr);
  uint32_t m = micros();
  while (!Serial.available()) {
    m += interval;
    do {
      diff = m - micros();
    } while (diff > 0);
    logRecord(&data, 0);
    printRecord(&Serial, &data);
  }
}
//------------------------------------------------------------------------------
void setup() {
  setSyncProvider(getTeensy3Time);
  if (ERROR_LED_PIN >= 0) {
    pinMode(ERROR_LED_PIN, OUTPUT);
    digitalWrite(ERROR_LED_PIN, HIGH);
  }
  pinMode(SAMPLE_CLK_PIN,OUTPUT); digitalWrite(SAMPLE_CLK_PIN,LOW);
  sampleClockPinState = false;
  pinMode(BLU_LED,OUTPUT); analogWrite(BLU_LED,255);
  pinMode(RED_LED,OUTPUT); digitalWrite(RED_LED,LOW);
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  buttonValue = lastButtonValue = digitalRead(BUTTON_PIN);
  analogReadResolution(12);  // crank up the bits
  
  Serial.begin(2000000);
  // Wait for USB Serial
  while (!Serial) {
    yield();
  }
  delay(1000);
  Serial.println(F("Hummingbird Beats v1"));
//  Verify the RTC
  if (timeStatus()!= timeSet) {
      Serial.println("Unable to sync with the RTC");
    } else {
    Serial.println("RTC has set the system time");
  }
  
  FillStack();
#if !ENABLE_DEDICATED_SPI
  Serial.println(F(
    "\nFor best performance edit SdFatConfig.h\n"
    "and set ENABLE_DEDICATED_SPI nonzero"));
#endif  // !ENABLE_DEDICATED_SPI

  Serial.print(FIFO_DIM);
  Serial.println(F(" FIFO entries will be used."));

  // Initialize SD.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }

  FsDateTime::setCallback(dateTime);

  lastFadeTime = millis();
  printUnusedStack();
  clearSerialInput();
  printControl();
}

  
//------------------------------------------------------------------------------
void loop() {
  // Read any Serial data.
  fadeLED(millis());
  getButtonState();
  serialCheck();
  if(isRunning){
    createBinFile(); logData();
    // logData jumps to a loop that waits for Serial to break. Also breakable by 
    isRunning = false;
  }
}
