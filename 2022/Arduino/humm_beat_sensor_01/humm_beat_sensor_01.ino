/*
 *    this code targeted to Teensy 3.6
 *    use for baseline record capability and data retrieval
 *    set to highest ADC resolution
 *    
 */


// Example to demonstrate write latency for preallocated exFAT files.
// I suggest you write a PC program to convert very large bin files.
//
// The maximum data rate will depend on the quality of your SD
// the size of the FIFO, and using dedicated SPI.
// March, 2022: using ScanDisk EDGE 16GB micro with UHS 10

#include "SdFat.h"
#include "FreeStack.h"  // this is legacy from the example. not sure how useful.
#include "HummBird.h"
#include <TimeLib.h>  // used to time stamp files
#include <EEPROM.h>   // storing record duration.  or other things?


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
  delay(500);
  printRecord(&Serial, nullptr);
  uint32_t m = micros();
  while (!isRunning) {
    m += interval;
    do {
      diff = m - micros();
      getButtonState();
      fadeLED(millis());
    } while (diff > 0);
    logRecord(&data, 0);
    printRecord(&Serial, &data);
    if(Serial.available()){ break; } 
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
  pinMode(ACCEL_TEST_PIN,OUTPUT); digitalWrite(ACCEL_TEST_PIN,LOW);
  sampleClockPinState = false;
  pinMode(BLU_LED,OUTPUT); analogWrite(BLU_LED,255);
  pinMode(RED_LED,OUTPUT); digitalWrite(RED_LED,LOW);
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  buttonValue = lastButtonValue = digitalRead(BUTTON_PIN);
  analogReadResolution(12);  // crank up the bits
  
  Serial.begin(250000);
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

  recordDuration = setRecordDuration(STARTUP_DURATION);
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
    // logData jumps to a loop that waits for Serial or a time out to break
    isRunning = false;
  }
  if(freeRunningTest){
    testSensor();
    freeRunningTest = false;
  }
}
